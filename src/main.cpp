#include "engine.h"

// 使用ESP32-S3的默认I2C引脚初始化OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE); 
PageState currentPage = PAGE_CLOCK;
bool isSleep = false;
String weatherText = "--", weatherTemp = "--";
int menuIndex = 0;
uint8_t contrastValues[] = {10, 80, 160, 255}, contrastIdx = 2, sleepIdx = 2;
int sleepTimeOptions[] = {30, 60, 0}; 
unsigned long lastOperateTime = 0, lastAnimTime = 0, lastClockUpdate = 0;
float menuX[3] = {160, 160, 160}, targetX[3] = {65, 107, 149}, frameX = 160;
bool needsViewCountRefresh = false;

bool isButtonPressed(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(20);
        if (digitalRead(pin) == LOW) {
            lastOperateTime = millis();
            while(digitalRead(pin) == LOW) delay(1);
            return true;
        }
    }
    return false;
}

void setup() {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFontPosTop(); 
    u8g2.setContrast(contrastValues[contrastIdx]);
    int pins[] = {BTN_CONFIRM, BTN_RIGHT, BTN_LEFT, BTN_BACK, BTN_SLEEP};
    for(int p : pins) pinMode(p, INPUT_PULLUP);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long startT = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startT < 15000) {
        u8g2.clearBuffer();
        drawLoadingBar();
        u8g2.sendBuffer();
    }
    if (WiFi.status() == WL_CONNECTED) {
        configTime(28800, 0, "time.apple.com");
        updateWeather();
    }
    lastOperateTime = millis();
}

void loop() {
    if (sleepTimeOptions[sleepIdx] != 0 && !isSleep) {
        if (millis() - lastOperateTime > (unsigned long)sleepTimeOptions[sleepIdx] * 1000) {
            isSleep = true; u8g2.setPowerSave(1);
        }
    }
    if (isButtonPressed(BTN_SLEEP)) { isSleep = !isSleep; u8g2.setPowerSave(isSleep); }
    if (isSleep) return;
    if (currentPage == PAGE_STATUS_DETAIL) {
        if (isButtonPressed(BTN_CONFIRM) || isButtonPressed(BTN_BACK) || isButtonPressed(BTN_LEFT) || isButtonPressed(BTN_RIGHT)) {
            currentPage = PAGE_SUB_NET;
            menuX[0] = -40; menuX[1] = -82; menuX[2] = -124; 
        }
        drawStatusDetail();
        return; 
    }
    if (isButtonPressed(BTN_CONFIRM)) {
        bool needsResetAnim = true;
        if (currentPage == PAGE_CLOCK) { currentPage = PAGE_MENU_MAIN; menuIndex = 0; }
        else if (currentPage == PAGE_MENU_MAIN) { 
            if (menuIndex == 0) currentPage = PAGE_MENU_SET;
            else if (menuIndex == 1) { 
                currentPage = PAGE_VIEWCOUNT;
                needsViewCountRefresh = true;
            }
        }
        else if (currentPage == PAGE_MENU_SET) { currentPage = (menuIndex == 0 ? PAGE_SUB_NET : PAGE_SUB_SCR); menuIndex = 0; }
        else if (currentPage == PAGE_SUB_NET) {
            if (menuIndex == 0) { currentPage = PAGE_STATUS_DETAIL; needsResetAnim = false; }
            else if (menuIndex == 1) { reconnectWiFi(); needsResetAnim = false; }
        }
        else if (currentPage == PAGE_SUB_SCR) {
            if (menuIndex == 0) { contrastIdx = (contrastIdx + 1) % 4; u8g2.setContrast(contrastValues[contrastIdx]); }
            else { sleepIdx = (sleepIdx + 1) % 3; }
            needsResetAnim = false;
        }
        else if (currentPage == PAGE_VIEWCOUNT) {
            // 在小白李页面按确定键刷新数据
            needsViewCountRefresh = true;
            needsResetAnim = false;
        }
        if (needsResetAnim) { menuX[0]=160; menuX[1]=202; menuX[2]=244; }
    }
    if (isButtonPressed(BTN_BACK)) {
        if (currentPage == PAGE_MENU_MAIN) currentPage = PAGE_CLOCK;
        else if (currentPage == PAGE_VIEWCOUNT) currentPage = PAGE_MENU_MAIN;
        else {
            if (currentPage >= PAGE_SUB_NET) currentPage = PAGE_MENU_SET;
            else if (currentPage == PAGE_MENU_SET) currentPage = PAGE_MENU_MAIN;
            menuX[0] = -40; menuX[1] = -82; menuX[2] = -124;
        }
        menuIndex = 0;
    }
    int maxIdx = (currentPage == PAGE_SUB_NET) ? 1 : (currentPage == PAGE_MENU_MAIN) ? 1 : 1; 
    if (isButtonPressed(BTN_RIGHT)) menuIndex = (menuIndex + 1) % (maxIdx + 1);
    if (isButtonPressed(BTN_LEFT))  menuIndex = (menuIndex - 1 + (maxIdx + 1)) % (maxIdx + 1);
    unsigned long now = millis();
    lastAnimTime = now;
    updateAnimation();
    if (currentPage == PAGE_CLOCK) {
        if (menuX[0] < 140 && menuX[0] > -30) drawCommonMenu(0, 0); 
        else if (now - lastClockUpdate >= 500) { lastClockUpdate = now; drawClock(); }
    } else {
        if (currentPage == PAGE_MENU_MAIN) drawCommonMenu(129, 171);
        else if (currentPage == PAGE_MENU_SET) drawCommonMenu(248, 222); 
        else if (currentPage == PAGE_SUB_NET) drawCommonMenu(238, 84);
        else if (currentPage == PAGE_SUB_SCR) drawCommonMenu(137, 123);
        else if (currentPage == PAGE_VIEWCOUNT) {
            if (needsViewCountRefresh) {
                updateView();
                needsViewCountRefresh = false;
            }
            drawViewCountPage(); 
        }
    }
    delay(3);
}