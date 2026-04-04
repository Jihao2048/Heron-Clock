#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_sleep.h>
#include "conf.h"

void drawLoadingBar() {
    const int screenW = 128;
    const int screenH = 32;
    const int animTime = 800;  
    const int pauseTime = 500; 
    long ms = millis() % (animTime + pauseTime);
    if (ms < animTime) {
        float t = (float)ms / (animTime / 2.0);    
        if (t < 1.0) {
            float easeOut = 1.0 - pow(1.0 - t, 2);
            u8g2.drawBox(0, 0, (int)(screenW * easeOut), screenH);
        } else {
            float t2 = t - 1.0;
            float easeOut = 1.0 - pow(1.0 - t2, 2);
            int startX = (int)(screenW * easeOut);
            u8g2.drawBox(startX, 0, screenW - startX, screenH);
        }
    } 
}

void updateWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    HTTPClient http;
    http.setTimeout(1500);
    http.begin("http://api.seniverse.com/v3/weather/now.json?key=SZ8f-QeTjfWad262W&location=weifang&language=zh-Hans&unit=c");
    if (http.GET() == 200) {
        JsonDocument doc; 
        deserializeJson(doc, http.getString());
        weatherText = doc["results"][0]["now"]["text"].as<String>();
        weatherTemp = doc["results"][0]["now"]["temperature"].as<String>();
    }
    http.end();
}

void updateLunar() {
    if (WiFi.status() != WL_CONNECTED) {
        lunarData = "网络未连接";
        return;
    }
    HTTPClient http;
    http.setTimeout(3000);
    http.begin("http://v2.alapi.cn/api/lunar?token=LwExDtUWhF3rH5ib");
    int httpCode = http.GET();
    if (httpCode == 200) {
        JsonDocument doc; 
        deserializeJson(doc, http.getString());
        if(doc["code"].as<int>() == 200) {
            String month = doc["data"]["lunar_month_chinese"].as<String>();
            String day = doc["data"]["lunar_day_chinese"].as<String>();
            String festival = doc["data"]["festival"].as<String>();
            String luck = doc["data"]["xiu_luck"].as<String>();
            lunarData = month + day + "  " + luck;
            festival = festival;
        } else {
            lunarData = "API错误";
        }
    } else {
        lunarData = "请求失败";
    }
    http.end();
}

void updateView() {
    if (WiFi.status() != WL_CONNECTED) {
        ViewCount = "未连网";
        return;
    }
    HTTPClient http;
    http.setTimeout(5000);
    http.begin("https://api.bilibili.com/x/web-interface/view?bvid=" + bvid);
    if (http.GET() == 200) {
        JsonDocument doc;
        deserializeJson(doc, http.getString());
        if (doc["code"].as<int>() == 0) {
            // 获取标题
            const char* title = doc["data"]["title"];
            if (title) {
                Title = String(title);
            } else {
                Title = "标题";
            }
            int view = doc["data"]["stat"]["view"].as<int>();
            ViewCount = String(view);
        } else {
            ViewCount = "获取失败";
            Title = "标题";
        }
    } else {
        ViewCount = "网络错误";
        Title = "标题";
    }
    http.end();
}

void drawView() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(0, 0 + pixeloffset);
    u8g2.print(Title);
    u8g2.setCursor(0, 16 + pixeloffset);
    u8g2.print(ViewCount);
    u8g2.sendBuffer();
    }


void drawAppsPage() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    
    // 平滑滚动动画
    float easing = 0.3;  // 加快动画速度
    float prevScrollX = appScrollX;
    appScrollX += (targetAppScrollX - appScrollX) * easing;
    
    // 检测滚动是否完成
    if (isAppScrolling) {
        // 检查是否接近目标位置
        if (abs(appScrollX - targetAppScrollX) < 0.5) {
            appScrollX = targetAppScrollX;
            
            // 检查是否完成整个屏幕的滚动
            if (abs(appScrollX) >= 128) {
                // 滚动完成，重置到中心位置并应用目标应用索引
                appScrollX = 0;
                targetAppScrollX = 0;
                currentAppIndex = targetAppIndex;  // 动画完成后才更新当前应用索引
                scrollDirection = SCROLL_NONE;
            }
            // 只有在没有新的滚动目标时才停止动画
            if (abs(targetAppScrollX) < 0.1) {
                isAppScrolling = false;
            }
        }
    }
    
    // 绘制应用（实现滚动效果）
    int baseX = (int)appScrollX;
    int y = 8;
    
    // 根据滚动方向绘制不同的应用组合
    if (scrollDirection == SCROLL_LEFT) {
        // 向左滚动：显示当前应用和下一个应用
        u8g2.setCursor(baseX + 5, y);
        u8g2.print(appsList[currentAppIndex]);
        
        int nextIndex = (currentAppIndex + 1) % maxApps;
        u8g2.setCursor(baseX + 128 + 5, y);
        u8g2.print(appsList[nextIndex]);
    } else if (scrollDirection == SCROLL_RIGHT) {
        // 向右滚动：显示当前应用和前一个应用
        u8g2.setCursor(baseX + 5, y);
        u8g2.print(appsList[currentAppIndex]);
        
        int prevIndex = (currentAppIndex - 1 + maxApps) % maxApps;
        u8g2.setCursor(baseX - 128 + 5, y);
        u8g2.print(appsList[prevIndex]);
    } else {
        // 无滚动：只显示当前应用
        u8g2.setCursor(5, y);
        u8g2.print(appsList[currentAppIndex]);
    }
    
    // 显示左右箭头指示器
    u8g2.setFont(u8g2_font_5x7_tf);
    if (maxApps > 1) {
        u8g2.setCursor(0, 10);
        u8g2.print("<");
        u8g2.setCursor(122, 10);
        u8g2.print(">");
    }
    
    u8g2.sendBuffer();
    }
    
void reconnectWiFi() {
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
        updateLunar();
        updateView();
        delay(1000);
    }
}

void drawStatusDetail() {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0 + pixeloffset);
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.printf("IP: %s", WiFi.localIP().toString().c_str());
    } else {
        u8g2.print("网络未连接");
    }
    u8g2.setCursor(0, 17);
    float currentTemp = temperatureRead();
    if (currentTemp > 40.0) {
        esp_restart();
    } else {
        u8g2.printf("核心温度: %.1f°C", currentTemp);
    }
    u8g2.sendBuffer();
}

void drawClock(bool isInitialDisplay) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    bool isConnected = isInitialDisplay ? (WiFi.status() == WL_CONNECTED) : connectedDuringInit;
    if (isConnected) {
        struct tm ti;
        if (getLocalTime(&ti, 0)) { 
            // 显示日期
            u8g2.setCursor(0, 0 + pixeloffset);
            if (ti.tm_hour >= 12) {
                u8g2.printf("%d月%d日下午", ti.tm_mon + 1, ti.tm_mday);
            } else {
                u8g2.printf("%d月%d日上午", ti.tm_mon + 1, ti.tm_mday);
            } 
            u8g2.setCursor(0, 16 + pixeloffset);
            u8g2.printf("%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
            u8g2.setCursor(95, 0 + pixeloffset); 
            u8g2.print(weatherText);
            u8g2.setCursor(95, 16 + pixeloffset); 
            u8g2.print(weatherTemp + "°C"); 
        } else {
            u8g2.setCursor(5, 8 + pixeloffset); 
            u8g2.print("获取中");
        }
    } else {
        u8g2.setCursor(5, 8 + pixeloffset); 
        u8g2.print("未连网");
    }
    u8g2.sendBuffer();
}

void drawCommonMenu(uint16_t ic1, uint16_t ic2, uint16_t ic3 = 0) {
    u8g2.clearBuffer();
    
    const char* dynamicTitle;
    switch (currentPage) {
        case PAGE_MENU_MAIN:  dynamicTitle = (menuIndex == 0 ? "设置" : "应用"); break;
        case PAGE_MENU_SET:   dynamicTitle = (menuIndex == 0 ? "网络" : "屏幕"); break;
        case PAGE_SUB_NET:    dynamicTitle = (menuIndex == 0 ? "状态" : "连接"); break;
        case PAGE_SUB_SCR:    dynamicTitle = (menuIndex == 0 ? "亮度" : "息屏"); break;
        default:              dynamicTitle = "夜鹭";
    }

    auto inBounds = [](float x) { return x < 155 && x > -35; };

    u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
    uint16_t icons[] = {ic1, ic2, ic3};
    for (int i = 0; i < 3; i++) {
        if (icons[i] && inBounds(menuX[i])) u8g2.drawGlyph((int)menuX[i], 7, icons[i]);
    }
    if (inBounds(frameX)) u8g2.drawFrame((int)frameX - 4, 4, 24, 24);

    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, 50, 32);
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(5, 8);
    u8g2.print(dynamicTitle);

    if (currentPage == PAGE_SUB_SCR) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(5, 20 + pixeloffset);
        u8g2.print(menuIndex == 0 ? (String("LV ") + (contrastIdx + 1)).c_str() : (String("") + sleepTimeOptions[sleepIdx] + "s").c_str());
    }
    u8g2.sendBuffer();
}

void updateAnimation() {
    float easing = 0.28; 
    if (currentPage != PAGE_CLOCK && currentPage != PAGE_STATUS_DETAIL) {
        targetX[0] = 60; targetX[1] = 100; targetX[2] = 140;
        for (int i = 0; i < 3; i++) menuX[i] += (targetX[i] - menuX[i]) * easing;
        float targetFrameX = menuIndex * 40 + 60;
        frameX += (targetFrameX - frameX) * easing;
    } else {
        for (int i = 0; i < 3; i++) menuX[i] += (160 - menuX[i]) * easing;
        frameX += (160 - frameX) * easing;
    }
    
    // 重置应用滚动位置（当不在APPS页面时完全重置）
    if (currentPage != PAGE_APPS) {
        targetAppScrollX = 0;
        appScrollX = 0;
        isAppScrolling = false;
    } else if (!isAppScrolling && abs(appScrollX) > 1.0) {
        // 逐渐回到中心位置
        targetAppScrollX = 0;
    }
}

// 计算器应用
void drawCalculator() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(5, 8);
    u8g2.print("计算器");
    u8g2.setCursor(5, 22);
    u8g2.print("功能开发中");
    u8g2.sendBuffer();
}

// 中国农历应用
void drawLunarCalendar() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(0, 0 + pixeloffset);
    u8g2.print(lunarData);
    u8g2.setCursor(0, 16 + pixeloffset);
    u8g2.print(festival);
    u8g2.sendBuffer();
}