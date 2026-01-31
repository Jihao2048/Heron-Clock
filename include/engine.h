#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "conf.h"

void drawLoadingCircle(int x, int y, int r, int angle) {
    for (int i = 0; i < 280; i += 15) {
        float rad = (angle + i) * 0.01745329;
        u8g2.drawPixel(x + cos(rad) * r, y + sin(rad) * r);
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

void forceRefresh() {
    if (WiFi.status() == WL_CONNECTED) {
        configTime(28800, 0, "time.apple.com");
        updateWeather();
    }
}

void reconnectWiFi() {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void drawStatusDetail() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.setCursor(0, 1); u8g2.print("--- 设备状态 ---");
    u8g2.setCursor(0, 13);
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.printf("IP: %s", WiFi.localIP().toString().c_str());
    } else {
        u8g2.print("网络: 未连接");
    }
    u8g2.setCursor(0, 25);
    u8g2.printf("核心温度: %.1f°C", temperatureRead());
    u8g2.sendBuffer();
}

void drawClock() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    if (WiFi.status() == WL_CONNECTED) {
        struct tm ti;
        if (getLocalTime(&ti, 0)) { 
            u8g2.setCursor(0, 2); 
            u8g2.printf("%d月%d日", ti.tm_mon + 1, ti.tm_mday);
            u8g2.setCursor(0, 18); 
            u8g2.printf("%02d %02d %02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
            u8g2.setCursor(95, 2); u8g2.print(weatherText);
            u8g2.setCursor(95, 18); u8g2.print(weatherTemp + "°C"); 
        } else {
            u8g2.setCursor(5, 8); u8g2.print("同步中");
        }
    } else {
        u8g2.setCursor(5, 8); u8g2.print("未连网");
    }
    u8g2.sendBuffer();
}

void drawCommonMenu(uint16_t ic1, uint16_t ic2, uint16_t ic3 = 0) {
    u8g2.clearBuffer();
    const char* dynamicTitle = "夜鹭";
    if (currentPage == PAGE_MENU_MAIN) dynamicTitle = (menuIndex == 0 ? "设置" : "文件");
    else if (currentPage == PAGE_MENU_SET)  dynamicTitle = (menuIndex == 0 ? "网络" : "屏幕");
    else if (currentPage == PAGE_SUB_NET) {
        if (menuIndex == 0) dynamicTitle = "状态";
        else if (menuIndex == 1) dynamicTitle = "连接";
        else dynamicTitle = "刷新";
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
        if (currentPage == PAGE_SUB_NET) {
            float targetScrollX = menuIndex * 42;
            scrollX += (targetScrollX - scrollX) * easing;
            for (int i = 0; i < 3; i++) {
                targetX[i] = 65 + (i * 42) - scrollX;
                menuX[i] += (targetX[i] - menuX[i]) * easing;
            }
            frameX += (65 - frameX) * easing;
        } else {
            scrollX = 0;
            targetX[0] = 65; targetX[1] = 107; targetX[2] = 149;
            for (int i = 0; i < 3; i++) menuX[i] += (targetX[i] - menuX[i]) * easing;
            float targetFrameX = menuIndex * 42 + 65;
            frameX += (targetFrameX - frameX) * easing;
        }
    } else {
        for (int i = 0; i < 3; i++) menuX[i] += (160 - menuX[i]) * easing;
        frameX += (160 - frameX) * easing;
    }
}