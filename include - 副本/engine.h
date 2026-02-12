#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "conf.h"

void drawLoadingBar() {
    const int screenW = 128;
    const int screenH = 32;
    const int animTime = 1200;  
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

void updateView() {
    if (WiFi.status() != WL_CONNECTED) {
        ViewCount = "未连网";
        return;
    }
    
    HTTPClient http;
    http.setTimeout(5000);
    http.begin("https://api.bilibili.com/x/web-interface/view?bvid=BV1LxF4znEwU");
    
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
            
            // 获取播放量
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

void drawViewCountPage() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(0, 2);
    u8g2.print(Title);
    u8g2.setCursor(0, 18);
    u8g2.print(ViewCount);
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
        delay(1000);
    }
}

void drawStatusDetail() {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 1);
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.printf("IP: %s", WiFi.localIP().toString().c_str());
    } else {
        u8g2.print("网络未连接");
    }
    u8g2.setCursor(0, 17);
    float currentTemp = temperatureRead();
    if (currentTemp > 50.0) {
        u8g2.print("发烧了喵！");
    } else {
        u8g2.printf("核心温度: %.1f°C", currentTemp);
    }
    u8g2.sendBuffer();
}

void drawClockAfterInit() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    if (connectedDuringInit == true) {
        struct tm ti;
        if (getLocalTime(&ti, 0)) { 
            u8g2.setCursor(0, 2);
            if (ti.tm_hour >= 12) {
                u8g2.printf("%d月%d日下午", ti.tm_mon + 1, ti.tm_mday);
            } else {
                u8g2.printf("%d月%d日上午", ti.tm_mon + 1, ti.tm_mday);
            } 
            u8g2.setCursor(0, 18);
            u8g2.printf("%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
            u8g2.setCursor(95, 2); u8g2.print(weatherText);
            u8g2.setCursor(95, 18); u8g2.print(weatherTemp + "°C"); 
        } else {
            u8g2.setCursor(5, 8); u8g2.print("获取中");
        }
    } else {
        u8g2.setCursor(5, 8); u8g2.print("未连网");
    }
    u8g2.sendBuffer();
}

void drawClockAtInit() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    if (WiFi.status() == WL_CONNECTED) {
        struct tm ti;
        if (getLocalTime(&ti, 0)) { 
            u8g2.setCursor(0, 2);
            if (ti.tm_hour >= 12) {
                u8g2.printf("%d月%d日下午", ti.tm_mon + 1, ti.tm_mday);
            } else {
                u8g2.printf("%d月%d日上午", ti.tm_mon + 1, ti.tm_mday);
            } 
            u8g2.setCursor(0, 18);
            u8g2.printf("%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
            u8g2.setCursor(95, 2); u8g2.print(weatherText);
            u8g2.setCursor(95, 18); u8g2.print(weatherTemp + "°C"); 
        } else {
            u8g2.setCursor(5, 8); u8g2.print("获取中");
        }
    } else {
        u8g2.setCursor(5, 8); u8g2.print("未连网");
    }
    u8g2.sendBuffer();
}

void drawCommonMenu(uint16_t ic1, uint16_t ic2, uint16_t ic3 = 0) {
    u8g2.clearBuffer();
    const char* dynamicTitle = "夜鹭";
    if (currentPage == PAGE_MENU_MAIN) dynamicTitle = (menuIndex == 0 ? "设置" : "小白李");
    else if (currentPage == PAGE_MENU_SET)  dynamicTitle = (menuIndex == 0 ? "网络" : "屏幕");
    else if (currentPage == PAGE_SUB_NET) {
        if (menuIndex == 0) dynamicTitle = "状态";
        else if (menuIndex == 1) dynamicTitle = "连接";
    }
    else if (currentPage == PAGE_SUB_SCR) dynamicTitle = (menuIndex == 0 ? "亮度" : "息屏");
    u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
    if (menuX[0] < 155 && menuX[0] > -35) u8g2.drawGlyph((int)menuX[0], 7, ic1);
    if (menuX[1] < 155 && menuX[1] > -35) u8g2.drawGlyph((int)menuX[1], 7, ic2);
    if (ic3 && menuX[2] < 155 && menuX[2] > -35) u8g2.drawGlyph((int)menuX[2], 7, ic3);
    if (frameX < 155 && frameX > -35) u8g2.drawFrame((int)frameX - 4, 4, 24, 24);
    u8g2.setDrawColor(0); u8g2.drawBox(0, 0, 50, 32); u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.setCursor(5, 8); u8g2.print(dynamicTitle);
    if (currentPage == PAGE_SUB_SCR) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(5, 22);
        if (menuIndex == 0) u8g2.printf("LV %d", contrastIdx + 1);
        else u8g2.printf("%ds", sleepTimeOptions[sleepIdx]);
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
}