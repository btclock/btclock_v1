#pragma once

//#include <string>
#ifdef IS_BW
#include <GxEPD2_BW.h>
#endif
#ifdef IS_3C
#include <GxEPD2_3C.h>
#endif
#include <cstdint>
#include "structs.h"
#pragma once

#include <Preferences.h>

//template <typename GxEPD2_Type, template <a, const uint16_t> class GxGFX_Type>
//extern GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> displays[7];

extern char blocks[8];
extern char currentBlocks[8];
extern String blockNr;

#ifndef OTA_NAME
#define OTA_NAME "btclock"
#endif

extern int FG_COLOR; 
extern int BG_COLOR;
extern const int RST_PIN;
extern unsigned int currentScreen;
extern Preferences preferences;
extern String epdContent[7];
extern bool timerRunning;
extern uint timerSeconds;
extern uint32_t moment;

const int SCREEN_BLOCK_HEIGHT = 0;
const int SCREEN_MSCW_TIME = 1;
const int SCREEN_BTC_TICKER = 2;
const int SCREEN_TIME = 3;
const int screens[4] = { SCREEN_BLOCK_HEIGHT, SCREEN_MSCW_TIME, SCREEN_BTC_TICKER, SCREEN_TIME };
const uint screenAmount = sizeof(screens) / sizeof(uint);