#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

#define BTN_CONFIRM 1
#define BTN_RIGHT   3
#define BTN_LEFT    4
#define BTN_BACK    5
#define BTN_SLEEP   6

#define WIFI_SSID "CMCC_xx"
#define WIFI_PASS "manifest"

enum PageState { PAGE_CLOCK, PAGE_MENU_MAIN, PAGE_MENU_SET, PAGE_SUB_NET, PAGE_SUB_SCR, PAGE_STATUS_DETAIL };

extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;
extern PageState currentPage;
extern bool isSleep;
extern String weatherText, weatherTemp;
extern int menuIndex;
extern uint8_t contrastValues[], contrastIdx, sleepIdx;
extern int sleepTimeOptions[];
extern unsigned long lastOperateTime, lastAnimTime, lastClockUpdate;
extern float menuX[3], targetX[3], frameX, scrollX;