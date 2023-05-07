#include <iostream>
#include <stdio.h>
#include <string>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <config.h>

#include "shared.hpp"

using namespace std;

void showText(std::string text);
void showNumberWithPair(std::string top, std::string bottom, std::string text);

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void splitText(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, String top, String bottom);

void splitText(uint dispNr, String top, String bottom);

uint getDisplayInitTime();

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showDigit(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, char chr);

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showDigit(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, char chr, bool partial, const GFXfont *font);

void showDigit(uint dispNr, char chr, bool partial, const GFXfont *font);
void showDigit(uint dispNr, char chr, bool partial);
void showDigit(uint dispNr, char chr);

template <typename GxEPD2_Type, template <typename, const uint16_t> class GxGFX_Type>
void showChar(GxGFX_Type<GxEPD2_Type, GxEPD2_Type::HEIGHT> &disp, String text, const GFXfont *font);

void showChar(uint dispNum, String text, const GFXfont *font);

void fullRefresh();
void fullRefreshWatcher(void *parameter);
void refreshDisplay(void *parameter);

void clearDisplayVtask(void *parameter);
void clearDisplay(uint dispNum);
void showCharsInLine(std::string text, const GFXfont *font);
void waitForDisplayGuard(uint dispNum);

void showDigitVtask(void *parameter);
void splitTextVtask(void *parameter);
void renderDisplay(void *parameter);

void initDisplays();
void resetAllDisplays();
void hibernateAll();
String getBlockHeight();
String getBitcoinPrice();