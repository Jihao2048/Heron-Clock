#include "engine.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
PageState currentPage = PAGE_CLOCK;
bool isSleep = false;
String weatherText = "--", weatherTemp = "--";
uint8_t contrastValues[] = {10, 80, 160, 255}, contrastIdx = 2, sleepIdx = 0;
int sleepTimeOptions[] = {30, 60, 0};
volatile int64_t hardwareTimerCount = 0;
int64_t lastOperateTime = 0, lastAnimTime = 0, lastClockUpdate = 0;
bool timerInitialized = false;
esp_timer_handle_t periodicTimer = NULL;

float menuX[3] = {160, 160, 160}, targetX[3] = {65, 107, 149}, frameX = 160;
bool isFirstClockDisplay = true;
bool connectedDuringInit = false;
int menuIndex = 0;
int currentAppIndex = 0;
int targetAppIndex = 0;
float appScrollX = 0;
float targetAppScrollX = 0;
bool isAppScrolling = false;
const int btnPins[] = {BTN_LEFT, BTN_CONFIRM, BTN_RIGHT, BTN_BACK, BTN_SLEEP};
const int btnCount = 5;
volatile bool btnPressedFlags[5] = {false, false, false, false, false};
int64_t lastInterruptTime = 0;

static int64_t getHardwareTime() {
    return hardwareTimerCount;
}

static void timerCallback(void* arg) {
    hardwareTimerCount += 1000;
}

void initHardwareTimer() {
    esp_timer_init();
    esp_timer_create_args_t timerArgs = {
        .callback = &timerCallback,
        .name = "PeriodicTimer"
    };
    esp_timer_create(&timerArgs, &periodicTimer);
    esp_timer_start_periodic(periodicTimer, 1000);
    timerInitialized = true;
}

const char* appsList[] = {"中国农历", "播放量", "设置"};
const int maxApps = sizeof(appsList) / sizeof(appsList[0]);

void IRAM_ATTR handleButtonInterrupt() {
    int64_t interruptTime = getHardwareTime();
    if (interruptTime - lastInterruptTime > 200000) {
        for (int i = 0; i < btnCount; i++) {
            if (digitalRead(btnPins[i]) == LOW) {
                btnPressedFlags[i] = true;
                lastOperateTime = interruptTime;
            }
        }
        lastInterruptTime = interruptTime;
    }
}

bool checkBtn(int btnIndex) {
    if (btnPressedFlags[btnIndex]) {
        btnPressedFlags[btnIndex] = false;
        return true;
    }
    return false;
}

void setup() {
    initHardwareTimer();
    
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFontPosTop();
    u8g2.setContrast(contrastValues[contrastIdx]);

    for (int i = 0; i < btnCount; i++) {
        pinMode(btnPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(btnPins[i]), handleButtonInterrupt, FALLING);
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int64_t startT = getHardwareTime();
    while (WiFi.status() != WL_CONNECTED && getHardwareTime() - startT < 15000000) {
        u8g2.clearBuffer();
        drawLoadingBar();
        u8g2.sendBuffer();
    }

    if (WiFi.status() == WL_CONNECTED) {
        configTime(28800, 0, "time.apple.com");
        struct tm timeinfo;
        int64_t timeSyncStart = getHardwareTime();
        while (!getLocalTime(&timeinfo, 100) && (getHardwareTime() - timeSyncStart < 2000000)) {
            delay(100);
        }
        updateWeather();
        updateLunar();
        updateView();
        connectedDuringInit = true;
        
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    lastOperateTime = getHardwareTime();
    gpio_wakeup_enable((gpio_num_t)BTN_SLEEP, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
}

void loop() {
    // 自动休眠检测
    if (sleepTimeOptions[sleepIdx] != 0 && !isSleep) {
        if (getHardwareTime() - lastOperateTime > (int64_t)sleepTimeOptions[sleepIdx] * 1000000) {
            isSleep = true;
            u8g2.setPowerSave(1);
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
        }
    }

    // 睡眠键逻辑
    if (checkBtn(4)) {
        isSleep = !isSleep;
        u8g2.setPowerSave(isSleep);
        if (isSleep) {
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            setCpuFrequencyMhz(40);
        } else {
            lastOperateTime = getHardwareTime();
            setCpuFrequencyMhz(160);
        }
    }

    if (isSleep) return;

    // 详情页返回逻辑
    if (currentPage == PAGE_STATUS_DETAIL) {
        if (checkBtn(1) || checkBtn(3) || checkBtn(0) || checkBtn(2)) {
            currentPage = PAGE_SUB_NET;
            menuIndex = 0;
            menuX[0] = -40; menuX[1] = -82; menuX[2] = -124;
        }
        drawStatusDetail();
        return;
    }

    // 1. 确认键 (Confirm)
    if (checkBtn(1)) {
        bool needsResetAnim = true;
        switch (currentPage) {
            case PAGE_CLOCK:
                currentPage = PAGE_APPS;
                menuIndex = 0;
                break;
            case PAGE_MENU_SET:
                currentPage = (menuIndex == 0) ? PAGE_SUB_NET : PAGE_SUB_SCR;
                menuIndex = 0;
                break;
            case PAGE_SUB_NET:
                if (menuIndex == 0) {
                    currentPage = PAGE_STATUS_DETAIL;
                } else {
                    reconnectWiFi();
                }
                needsResetAnim = false;
                break;
            case PAGE_SUB_SCR:
                if (menuIndex == 0) {
                    contrastIdx = (contrastIdx + 1) % 4;
                    u8g2.setContrast(contrastValues[contrastIdx]);
                } else {
                    sleepIdx = (sleepIdx + 1) % 3;
                }
                needsResetAnim = false;
                break;
            case PAGE_APPS:
                if (currentAppIndex == 0) currentPage = PAGE_APP_LUNAR;
                else if (currentAppIndex == 1) currentPage = PAGE_APP_VIEWCOUNT;
                else if (currentAppIndex == 2) currentPage = PAGE_MENU_SET;
                needsResetAnim = false;
                break;
            case PAGE_APP_LUNAR:
            case PAGE_APP_VIEWCOUNT:
                currentPage = PAGE_APPS;
                menuIndex = 0;
                break;
            default:
                break;
        }
        if (needsResetAnim) { menuX[0] = 160; menuX[1] = 202; menuX[2] = 244; }
    }

    // 2. 返回键 (Back)
    if (checkBtn(3)) {
        switch (currentPage) {
            case PAGE_MENU_SET:
                currentPage = PAGE_APPS;
                break;
            case PAGE_SUB_NET:
            case PAGE_SUB_SCR:
                currentPage = PAGE_MENU_SET;
                menuIndex = 0;
                break;
            case PAGE_STATUS_DETAIL:
                currentPage = PAGE_SUB_NET;
                menuIndex = 0;
                break;
            case PAGE_APPS:
                currentPage = PAGE_CLOCK;
                break;
            case PAGE_APP_LUNAR:
            case PAGE_APP_VIEWCOUNT:
                currentPage = PAGE_APPS;
                menuIndex = 0;
                break;
            default:
                break;
        }
        menuX[0] = -40; menuX[1] = -82; menuX[2] = -124;
    }

    // 3. 左右切换逻辑
    if (checkBtn(2)) { // Next
        if (currentPage == PAGE_APPS) {
            targetAppIndex = (currentAppIndex + 1) % maxApps;
            targetAppScrollX = -128;
            scrollDirection = SCROLL_LEFT;
            isAppScrolling = true;
        } else if (currentPage >= PAGE_MENU_SET && currentPage <= PAGE_SUB_SCR) {
            menuIndex = (menuIndex + 1) % 2;
        }
    }
    if (checkBtn(0)) { // Prev
        if (currentPage == PAGE_APPS) {
            targetAppIndex = (currentAppIndex - 1 + maxApps) % maxApps;
            targetAppScrollX = 128;
            scrollDirection = SCROLL_RIGHT;
            isAppScrolling = true;
        } else if (currentPage >= PAGE_MENU_SET && currentPage <= PAGE_SUB_SCR) {
            menuIndex = (menuIndex - 1 + 2) % 2;
        }
    }

    // 动画与渲染
    updateAnimation();
    int64_t now = getHardwareTime();

    if (currentPage == PAGE_CLOCK) {
        if (menuX[0] < 140 && menuX[0] > -30) drawCommonMenu(0, 0);
        else if (now - lastClockUpdate >= 500000) {
            lastClockUpdate = now;
            drawClock(isFirstClockDisplay);
            isFirstClockDisplay = false;
        }
    } else {
        switch (currentPage) {
            case PAGE_SUB_NET:
                drawCommonMenu(238, 84);
                break;
            case PAGE_SUB_SCR:
                drawCommonMenu(137, 123);
                break;
            case PAGE_APPS:
                drawAppsPage();
                break;
            case PAGE_MENU_SET:
                drawCommonMenu(248, 222);
                break;
            case PAGE_APP_LUNAR:
                drawLunarCalendar();
                break;
            case PAGE_APP_VIEWCOUNT:
                drawView();
                break;
            default:
                break;
        }
    }
}