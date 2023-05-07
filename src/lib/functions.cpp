#include "functions.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>
#include "../fonts/oswald-90.h"
#include "../fonts/oswald-20.h"
#include "../fonts/ubuntu-italic70.h"

#include "../web.h"

#ifdef IS_3C
GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> displays[7] = {
    GxEPD2_213_Z98c(/*CS=25*/ 21, /*DC=*/22, /*RST=*/-1, /*BUSY=34*/ 36), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=33*/ 13, /*DC=*/22, /*RST=*/-1, /*BUSY=36*/ 39), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=12*/ 5, /*DC=*/22, /*RST=*/-1, /*BUSY=35*/ 34),  // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=27*/ 17, /*DC=*/22, /*RST=*/-1, /*BUSY=*/35),    // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=14*/ 16, /*DC=*/22, /*RST=*/-1, /*BUSY=39*/ 32), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=5*/ 4, /*DC=*/22, /*RST=*/-1, /*BUSY=*/33),      // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_Z98c(/*CS=17*/ 15, /*DC=*/22, /*RST=*/-1, /*BUSY=*/25),    // GDEY0213Z98 122x250, SSD1680
};
#endif

#ifdef IS_BW
#ifdef IS_S3
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> displays[7] = {
    GxEPD2_213_B74(/*CS=25*/ 40, /*DC=*/1, /*RST=*/-1, /*BUSY=34*/ 4), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=33*/ 39, /*DC=*/1, /*RST=*/-1, /*BUSY=36*/ 5), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=12*/ 9, /*DC=*/1, /*RST=*/-1, /*BUSY=35*/ 6),  // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=27*/ 10, /*DC=*/1, /*RST=*/-1, /*BUSY=*/7),    // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=14*/ 3, /*DC=*/1, /*RST=*/-1, /*BUSY=39*/ 15), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=5*/ 8, /*DC=*/1, /*RST=*/-1, /*BUSY=*/16),     // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=17*/ 18, /*DC=*/1, /*RST=*/-1, /*BUSY=*/17),   // GDEY0213Z98 122x250, SSD1680
};
#else
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> displays[7] = {
    GxEPD2_213_B74(/*CS=25*/ 21, /*DC=*/22, /*RST=*/-1, /*BUSY=34*/ 36), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=33*/ 13, /*DC=*/22, /*RST=*/-1, /*BUSY=36*/ 39), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=12*/ 5, /*DC=*/22, /*RST=*/-1, /*BUSY=35*/ 34),  // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=27*/ 17, /*DC=*/22, /*RST=*/-1, /*BUSY=*/35),    // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=14*/ 16, /*DC=*/22, /*RST=*/-1, /*BUSY=39*/ 32), // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=5*/ 4, /*DC=*/22, /*RST=*/-1, /*BUSY=*/33),      // GDEY0213Z98 122x250, SSD1680
    GxEPD2_213_B74(/*CS=17*/ 15, /*DC=*/22, /*RST=*/-1, /*BUSY=*/25),    // GDEY0213Z98 122x250, SSD1680
};
#endif
#endif

String epdContent[7];
uint displaySize = sizeof(displays) / sizeof(uint);
uint32_t lastFullRefresh;
bool displayGuard[7] = {false, false, false, false, false, false, false};

void initDisplays()
{
    for (uint i = 0; i < 7; i++)
    {
        displays[i].init();
    }

    std::string text = "BTClock";

    showCharsInLine(text, &Ubuntu_Italic70pt7b);
    lastFullRefresh = millis();
}

void fullRefreshWatcher(void *parameter)
{
    for (;;)
    {
        if (millis() - lastFullRefresh > 1800 * 1000)
        {
            Serial.println("Doing full refresh");
            fullRefresh();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void fullRefresh()
{
    NormalText_T data[7];
    for (uint i = 0; i < 7; i++)
    {
        data[i] = {i, ' '};
        xTaskCreate(refreshDisplay, "RefreshDisplay" + char(i), 2048, &data[i], 500 + (i + 2), NULL);
    }
    lastFullRefresh = millis();
}

uint getDisplayInitTime()
{
    return displays[0].epd2.full_refresh_time * 1.5;
}

void resetAllDisplays()
{
    digitalWrite(RST_PIN, HIGH);
    pinMode(RST_PIN, OUTPUT);
    delay(20);
    digitalWrite(RST_PIN, LOW);
    delay(20);
    digitalWrite(RST_PIN, HIGH);
    delay(200);
}

void clearDisplayVtask(void *parameter)
{
    NormalText_T *data = (NormalText_T *)parameter;

    if (data->displayNum > displaySize)
    {
        vTaskDelete(NULL);
        return;
    }
    //    displays[data->displayNum].fillScreen(BG_COLOR);
    showDigit(data->displayNum, data->chr, false, &Ubuntu_Italic70pt7b);
    /// learDisplay(data->displayNum);
    // displays[data->displayNum].display(false);

    vTaskDelete(NULL);
}

void clearDisplay(uint dispNum)
{
    displays[dispNum].fillScreen(BG_COLOR);
    displays[dispNum].display(false);
}

void showNumberWithPair(std::string top, std::string bottom, std::string text)
{
    digitalWrite(RST_PIN, HIGH);
    pinMode(RST_PIN, OUTPUT);
    delay(20);
    digitalWrite(RST_PIN, LOW);
    delay(20);
    digitalWrite(RST_PIN, HIGH);
    delay(200);

    text.insert(text.begin(), 7 - text.length(), ' ');
    strcpy(blocks, text.c_str());
    SplitText_T spd = {0, top.c_str(), bottom.c_str()};
    xTaskCreate(splitTextVtask, "ShowText", 5000, &spd, 10, NULL);
    NormalText_T data[7];
    Serial.println(blocks);
    for (uint i = 1; i < 7; i++)
    {
        if (blocks[i] != currentBlocks[i] || true)
        {

            Serial.println("Writing " + String(blocks[i]) + " to display " + String(i));
            data[i] = {(i), blocks[i]};

            xTaskCreate(showDigitVtask, "Disp" + char(i), 2048, &data[i], (10 + i * 10), NULL);
        }
    }
}

void showText(std::string text)
{
    digitalWrite(RST_PIN, HIGH);
    pinMode(RST_PIN, OUTPUT);
    delay(20);
    digitalWrite(RST_PIN, LOW);
    delay(20);
    digitalWrite(RST_PIN, HIGH);
    delay(200);

    text.insert(text.begin(), 7 - text.length(), ' ');
    strcpy(blocks, text.c_str());
    // SplitText_T spd = {0, "MSCW", "TIME"};
    // xTaskCreate(splitTextVtask, "ShowText", 5000, &spd, 10, NULL);
    NormalText_T data[7];
    Serial.println(blocks);
    for (uint i = 0; i < 7; i++)
    {
        if (blocks[i] != currentBlocks[i] || true)
        {

            Serial.println("Writing " + String(blocks[i]) + " to display " + String(i));
            data[i] = {(i), blocks[i]};

            xTaskCreate(showDigitVtask, "Disp" + char(i), 2048, &data[i], (10 + i * 10), NULL);
        }
    }

    std::copy(blocks, blocks + 7, currentBlocks);
}

void waitForDisplayGuard(uint dispNum)
{
    if (!displayGuard[dispNum])
        return;
    Serial.print("Displayguard for screen ");
    Serial.print(dispNum);
    uint count = 0;
    while (displayGuard[dispNum])
    {
        if (count > 100)
        {
            throw std::invalid_argument("Waiting too long.");
        }
        Serial.print(".");
        delay(100);
        count++;
    }

    Serial.println();
}

void splitTextVtask(void *parameter)
{
    try
    {
        SplitText_T *data = (SplitText_T *)parameter;

        if (data->displayNum > displaySize)
        {
            vTaskDelete(NULL);
            return;
        }

        uint val = data->displayNum;
        String top = data->top;
        String bottom = data->bottom;

        waitForDisplayGuard(val);
        displayGuard[val] = true;
        epdContent[val] = top + "/" + bottom;
        // Serial.println("[VTASK SPLITTEXT] Writing " + String(data->top) + "/" + String(data->bottom) + " to display " + String(val));
        splitText(val, top, bottom);
        displayGuard[val] = false;
    }
    catch (...)
    {
        Serial.println("Error in splitText Vtask");
    }
    // displays[data->displayNum].hibernate();
    vTaskDelete(NULL);
}

void showDigitVtask(void *parameter)
{
    NormalText_T *data = (NormalText_T *)parameter;

    if (data->displayNum > displaySize)
    {
        vTaskDelete(NULL);
        return;
    }

    uint val = data->displayNum;
    char chr = data->chr;

    waitForDisplayGuard(val);

    displayGuard[val] = true;
#ifdef DEBUG
    Serial.println("[VTASK DIGIT] Writing " + String(chr) + " to display " + String(val));
#endif
    epdContent[val] = String(chr);

    showDigit(val, chr);
    displayGuard[val] = false;
    // displays[data->displayNum].hibernate();
    vTaskDelete(NULL);
}

void splitText(uint dispNr, String top, String bottom)
{
    splitText(displays[dispNr], top, bottom);
}

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void splitText(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, String top, String bottom)
{
    disp.setRotation(2);
    disp.setFont(&FONT_SMALL);
    disp.setTextColor(FG_COLOR);

    // Top text
    int16_t ttbx, ttby;
    uint16_t ttbw, ttbh;
    disp.getTextBounds(top, 0, 0, &ttbx, &ttby, &ttbw, &ttbh);
    uint16_t tx = ((disp.width() - ttbw) / 2) - ttbx;
    uint16_t ty = ((disp.height() - ttbh) / 2) - ttby - ttbh / 2 - 12;

    // Bottom text
    int16_t tbbx, tbby;
    uint16_t tbbw, tbbh;
    disp.getTextBounds(bottom, 0, 0, &tbbx, &tbby, &tbbw, &tbbh);
    uint16_t bx = ((disp.width() - tbbw) / 2) - tbbx;
    uint16_t by = ((disp.height() - tbbh) / 2) - tbby + tbbh / 2 + 12;

    // Make separator as wide as the shortest text.
    uint16_t lineWidth, lineX;
    if (tbbw < ttbh)
        lineWidth = tbbw;
    else
        lineWidth = ttbw;
    lineX = round((disp.width() - lineWidth) / 2);

    // disp.firstPage();
    // do
    //{
    disp.fillScreen(BG_COLOR);
    disp.setCursor(tx, ty);
    disp.print(top);
    disp.fillRoundRect(lineX, disp.height() / 2 - 3, lineWidth, 6, 3, FG_COLOR);
    disp.setCursor(bx, by);
    disp.print(bottom);

    disp.display(true);
    disp.hibernate();
    //  } while (disp.nextPage());
}

void showDigit(uint dispNr, char chr)
{
    showDigit(displays[dispNr], chr);
}

void showDigit(uint dispNr, char chr, bool partial)
{
    showDigit(displays[dispNr], chr, partial, &FONT_BIG);
}

void showDigit(uint dispNr, char chr, bool partial, const GFXfont *font)
{
    showDigit(displays[dispNr], chr, partial, font);
}

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showDigit(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, char chr)
{
    showDigit(disp, chr, true, &FONT_BIG);
}

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showDigit(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, char chr, bool partial, const GFXfont *font)
{
    String str(chr);
    disp.setRotation(2);
    disp.setFont(font);
    disp.setTextColor(FG_COLOR);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    disp.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t x = ((disp.width() - tbw) / 2) - tbx;
    uint16_t y = ((disp.height() - tbh) / 2) - tby;
    //    disp.setPartialWindow(x,y,tbw,tbh);
    // disp.firstPage();
    // do
    // {
    disp.fillScreen(BG_COLOR);
    disp.setCursor(x, y);
    disp.print(str);
    disp.display(partial);
    disp.hibernate();
    //  } while (disp.nextPage());
}

void refreshDisplay(void *parameter)
{
    NormalText_T *data = (NormalText_T *)parameter;
    if (data->displayNum > displaySize)
    {
        vTaskDelete(NULL);
        return;
    }
    uint val = data->displayNum;

    waitForDisplayGuard(val);
    displayGuard[val] = true;
    displays[val].epd2.refresh(false);
    displays[val].hibernate();
    displayGuard[val] = false;
    delay(displays[val].epd2.full_refresh_time);
    vTaskDelete(NULL);
}

void renderDisplay(void *parameter)
{
    NormalText_T *data = (NormalText_T *)parameter;

    uint val = data->displayNum;
    waitForDisplayGuard(val);

    displayGuard[val] = true;
    displays[val].display(false);
    displayGuard[val] = false;

    vTaskDelete(NULL);
}

void renderDisplayPartial(void *parameter)
{
    NormalText_T *data = (NormalText_T *)parameter;

    waitForDisplayGuard(data->displayNum);

    displayGuard[data->displayNum] = true;
    displays[data->displayNum].display(true);
    displayGuard[data->displayNum] = false;
    vTaskDelete(NULL);
}

void showCharsInLine(std::string text, const GFXfont *font)
{

    int16_t tbx[7], tby[7];
    uint16_t tbw[7], tbh[7];

    int i = 0;
    uint16_t largestChar = 0;
    uint16_t largestCharY = 0;
    for (int i = 0; i < text.size(); i++)
    {
        displays[i].setRotation(2);
        displays[i].setFont(font);
        displays[i].setTextColor(FG_COLOR);
        displays[i].fillScreen(BG_COLOR);
        displays[i].getTextBounds(String(text[i]), 0, 0, &tbx[i], &tby[i], &tbw[i], &tbh[i]);
        if (tbh[i] > largestChar)
        {
            largestChar = tbh[i];
            largestCharY = tby[i];
        }
    }

    NormalText_T data[7];
    i = 0;
    for (uint i = 0; i < text.size(); i++)
    {
        uint16_t x = ((displays[i].width() - tbw[i]) / 2) - tbx[i];
        uint16_t y = ((displays[i].height() - largestChar) / 2) - largestCharY;

        displays[i].setCursor(x, y);
        displays[i].print(String(text[i]));
        data[i] = {i, ' '};
        xTaskCreate(renderDisplay, "ClearDisp" + char(i), 2048, &data[i], (i + 2), NULL);
    }
}

void showChar(uint dispNum, String text, const GFXfont *font)
{
    showChar(displays[dispNum], text, font);
    NormalText_T data = {dispNum, ' '};
    xTaskCreate(renderDisplayPartial, "RP", 2048, &data, 10, NULL);
    delay(50); // This is slowing down the update, but causes errors otherwise.
}

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showChar(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, String text, const GFXfont *font)
{
    //    GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> disp = displays[dispNum];

    disp.setRotation(2);
    disp.setFont(font);
    disp.setTextColor(FG_COLOR);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    disp.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t x = ((disp.width() - tbw) / 2) - tbx;
    uint16_t y = ((disp.height() - tbh) / 2) - tby;
    //    disp.setPartialWindow(x,y,tbw,tbh);
    // disp.firstPage();
    // do
    // {
    disp.fillScreen(BG_COLOR);
    disp.setCursor(x, y);
    disp.print(text);
}

void hibernateAll()
{
    for (int i = 0; i < 7; i++)
    {
        Serial.print("Hibernate Display ");
        Serial.println(i);
        displays[i].hibernate();
    }
}

String getBitcoinPrice()
{
    HTTPClient http;
    StaticJsonDocument<64> doc;

    http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        DeserializationError error = deserializeJson(doc, http.getString());

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return "0";
        }
    }

    return doc["bitcoin"]["usd"].as<String>();
}

String getBlockHeight()
{
    HTTPClient http;
    http.begin("https://mempool.bitcoin.nl/api/blocks/tip/height");
    int httpCode = http.GET();

    return http.getString();
}
