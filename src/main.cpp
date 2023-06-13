#define ENABLE_GxEPD2_GFX 0

#include <WiFiClient.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "web.h"
#include "lib/shared.hpp"
#include "lib/functions.hpp"
#include "fonts/ubuntu-italic12.h"
#include "fonts/ubuntu-italic60.h"

Preferences preferences;

const int RST_PIN = 2;
char blocks[8];
char currentBlocks[8];
unsigned int currentScreen;

// GMT+1 is 3600 seconds time difference
ESP32Time rtc(TIME_OFFSET_SECONDS);
bool timerRunning = true;
uint timerSeconds;
String currentShownTime = "";
uint32_t moment;
uint otaProgressValue;

void showMscwTime();  
void showBlockHeight();
void showBtcUsd();
void showSimpleTime();

void screenHandler(void *parameter);
void otaHandler(void *parameter);
void timebasedChange(void *parameter);

void setup()
{
  Serial.begin(115200);
  preferences.begin("btclock", false);
  resetAllDisplays();
  initDisplays();

  uint initStart = millis();

#if CONFIG_LWIP_IPV6
  WiFi.enableIpV6();
#endif
  WiFi.hostname(OTA_NAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIPv6());

  ArduinoOTA.onStart([]()
                     {
    std::string text = "Update.";

    showCharsInLine(text, &Ubuntu_Italic60pt7b);
    delay(2000); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
    if (otaProgressValue == round(int(progress / (total / 100))))
      return;
    otaProgressValue = round(int(progress / (total / 100)));
    showChar(6, String(otaProgressValue) + "%", &Ubuntu_Italic12pt7b); });

  ArduinoOTA.setHostname(OTA_NAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  // one common reset for all displays

  configTime(3600, 0, NTP_SERVER);
  struct tm timeinfo;

  while (!getLocalTime(&timeinfo))
  {
    configTime(3600, 0, NTP_SERVER);
    delay(500);
    Serial.println("Retry set time");
  }

  rtc.setTimeStruct(timeinfo);

  setupWebserver();

  FG_COLOR = preferences.getUInt("fgColor", FG_COLOR);
  BG_COLOR = preferences.getUInt("bgColor", BG_COLOR);
  timerSeconds = preferences.getUInt("timerSeconds", 1800);

  currentScreen = preferences.getUInt("currentScreen", 0);
  Serial.print("Current screen: ");
  Serial.println(currentScreen);

  // Get the size of the flash memory
  uint32_t flash_size = ESP.getFlashChipSize();

  Serial.print("Flash size: ");
  Serial.print(flash_size);
  Serial.println(" bytes");

  Serial.print("Waiting for display init");
  while (millis() - initStart < getDisplayInitTime())
  {
    delay(500);
    Serial.print(".");
  }

  xTaskCreate(otaHandler, "ota", 10000, NULL, 10000, NULL);
  xTaskCreate(screenHandler, "screen", 10000, NULL, 5, NULL);
  xTaskCreate(timebasedChange, "tbc", 5000, NULL, 15, NULL);
  xTaskCreate(fullRefreshWatcher, "refreshWatch", 5000, NULL, 15, NULL);
}

void loop()
{
  // ArduinoOTA.handle();
}

void otaHandler(void *parameter)
{
  for (;;)
  {
    ArduinoOTA.handle();
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void screenHandler(void *parameter)
{
  int displayedScreen = -1;
  for (;;)
  {
    if (displayedScreen != currentScreen)
    {
      resetAllDisplays();

      switch (currentScreen)
      {
      case SCREEN_BLOCK_HEIGHT:
        showBlockHeight();
        break;
      case SCREEN_MSCW_TIME:
        showMscwTime();
        break;
      case SCREEN_BTC_TICKER:
        showBtcUsd();
        break;
      case SCREEN_TIME:
        showSimpleTime();
        break;
      }

      displayedScreen = currentScreen;
#ifdef IS_BW
    }
    else if (displayedScreen == SCREEN_TIME)
    {
      showSimpleTime();
#endif
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void timebasedChange(void *parameter)
{
  moment = millis();

  for (;;)
  {
    if (millis() - moment > timerSeconds * 1000 && timerRunning)
    {
      currentScreen = (currentScreen + 1) % screenAmount;
      preferences.putUInt("currentScreen", currentScreen);
      moment = millis();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void showSimpleTime()
{
  if (currentShownTime.compareTo(rtc.getTime("%H:%M").c_str()) != 0)
  {
    resetAllDisplays();
    showNumberWithPair(String(rtc.getDay()).c_str(), String(rtc.getMonth() + 1).c_str(), rtc.getTime("%H:%M").c_str());
    currentShownTime = rtc.getTime("%H:%M").c_str();
  }
  delay(1000);
}

void showMscwTime()
{
#ifdef DEBUG
  Serial.println("Show Moscow Time");
#endif

  String btcPrice = getBitcoinPrice();

  std::string moscowTime = std::to_string(int(round(1 / btcPrice.toFloat() * 10e7)));

  moscowTime.insert(moscowTime.begin(), 7 - moscowTime.length(), ' ');
  strcpy(blocks, moscowTime.c_str());

#ifdef DEBUG
  Serial.println(blocks);
#endif
  static SplitText_T spd = {0, "MSCW", "TIME"};
  xTaskCreate(splitTextVtask, "STv", 5000, &spd, 20, NULL);
  NormalText_T data[7];
  for (uint i = 1; i < 7; i++)
  {
    if (blocks[i] != currentBlocks[i])
    {
      data[i] = {i, blocks[i]};

      xTaskCreate(showDigitVtask, "Disp" + char(i), 2048, &data[i], 2 + i, NULL);
    }
  }

  std::copy(blocks, blocks + 7, currentBlocks);
  delay(2000);
}

void showBtcUsd()
{
#ifdef DEBUG
  Serial.println("Show BTC price");
#endif

  std::string btcUsd = ("$" + getBitcoinPrice()).c_str();

  btcUsd.insert(btcUsd.begin(), 7 - btcUsd.length(), ' ');
  strcpy(blocks, btcUsd.c_str());

  Serial.println(blocks);

  static SplitText_T spd = {0, "BTC", "USD"};
  xTaskCreate(splitTextVtask, "STv", 5000, &spd, 1, NULL);

  NormalText_T data[7];
  for (uint i = 1; i < 7; i++)
  {
    if (blocks[i] != currentBlocks[i])
    {
      data[i] = {i, blocks[i]};

      xTaskCreate(showDigitVtask, "Disp" + char(i), 2048, &data[i], 2, NULL);
    }
  }

  std::copy(blocks, blocks + 7, currentBlocks);
  delay(2000);
}

void showBlockHeight()
{
#ifdef DEBUG
  Serial.println("Show BlockHeight");
#endif
  std::string blockNrString = getBlockHeight().c_str();

  blockNrString.insert(blockNrString.begin(), 7 - blockNrString.length(), ' ');
  strcpy(blocks, blockNrString.c_str());

  Serial.println(blocks);
  SplitText_T spd = {0, "BLOCK", "HEIGHT"};
  xTaskCreate(splitTextVtask, "STv", 5000, &spd, 1, NULL);

  NormalText_T data[7];
  for (uint i = 1; i < 7; i++)
  {
    if (blocks[i] != currentBlocks[i])
    {
      data[i] = {i, blocks[i]};

      xTaskCreate(showDigitVtask, "Disp" + char(i), 2048, &data[i], 2, NULL);
    }
  }

  std::copy(blocks, blocks + 7, currentBlocks);
  delay(2000);
}