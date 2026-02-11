# 🛰️ Heron Clock

An elegant, smooth-animation ESP32-C3 desk clock featuring real-time weather, NTP time synchronization, and a custom UI engine.

![Platform](https://img.shields.io/badge/Platform-ESP32--C3-orange?logo=espressif)
![License](https://img.shields.io/badge/License-MIT-blue)
![Framework](https://img.shields.io/badge/Framework-Arduino-pro?logo=Arduino)

## ✨ Features
- 🕒 **NTP Sync**: Auto-syncing time with millisecond* precision via Apple Time servers.
- 🌤️ **Live Weather**: Real-time weather data fetched from Seniverse API.
- 🌊 **Fluid UI**: Smooth easing animations for menu transitions and page switching.
- 🛠️ **System Monitor**: Built-in status page to monitor IP address and ESP32 core temperature.
- 🌙 **Power Saving**: Customizable auto-sleep mode and manual brightness control.

## 🛠️ Hardware Requirements
- **Core**: ESP32-C3 DevKit
- **Display**: SSD1306 128x32 OLED (I2C)
- **Buttons**: 5-button navigation (Confirm, Left, Right, Back, Sleep)

## 🚀 Quick Start
1. Clone this repo to your PlatformIO environment.
2. Edit `include/conf.h` to add your WiFi credentials:
   ```cpp
   #define WIFI_SSID "Your_SSID"
   #define WIFI_PASS "Your_Password"

## 📷 Pictures

### 🕒 Main Interface
The clock interface features a clean layout with time, date, and real-time weather.
<p align="center">
  <img src="1.jpg" width="400">
</p>

### 📱 Smooth Menu System
Fluid easing animations for seamless navigation between settings and apps.
<p align="center">
  <img src="2.jpg" width="400">
</p>

---
> this program Gemini wrote it.

> ![哔哩哔哩账号](https://space.bilibili.com/2121656213) 附带演示和使用说明。

> 在干燥适宜温度环境下使用，仅进行正常操作（静置、携带、充电、烧录官方程序）。对于非正常使用造成的任何损害，本人(即项目作者)不承担责任。
