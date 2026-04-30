#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <esp_timer.h>

#define BTN_LEFT    0
#define BTN_CONFIRM 1
#define BTN_RIGHT   2
#define BTN_BACK    3
#define BTN_SLEEP   4
//在这里填写你家的WiFi和密码
#define WIFI_SSID "1054"
#define WIFI_PASS "manifest"

enum PageState { 
    PAGE_CLOCK, 
    PAGE_MENU_SET, 
    PAGE_SUB_NET, 
    PAGE_SUB_SCR, 
    PAGE_STATUS_DETAIL, 
    PAGE_APPS, 
    PAGE_APP_LUNAR, 
    PAGE_APP_VIEWCOUNT };
enum ScrollDirection { 
    SCROLL_NONE, 
    SCROLL_LEFT,
    SCROLL_RIGHT 
};
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;
extern PageState currentPage;
extern bool isSleep;
extern String weatherText, weatherTemp;
extern int menuIndex;
extern uint8_t contrastValues[], contrastIdx, sleepIdx;
extern int sleepTimeOptions[];
extern int64_t lastOperateTime, lastAnimTime, lastClockUpdate;
extern float menuX[3], targetX[3], frameX, scrollX;
extern bool connectedDuringInit;
extern int currentAppIndex;
extern int targetAppIndex;
extern float appScrollX, targetAppScrollX;
extern bool isAppScrolling;
extern const char* appsList[];
extern const int maxApps;
extern ScrollDirection scrollDirection;
extern volatile bool btnPressedFlags[];
String ViewCount = "--";
String Title = "--";
String lunarData = "--";
String lunarOthers = "--";
String bvid = "BV1KEojBeEDi";
int pixeloffset = 2;
ScrollDirection scrollDirection = SCROLL_NONE;
