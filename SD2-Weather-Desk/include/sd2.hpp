#ifndef __SD2_BOARD_HPP__
#define __SD2_BOARD_HPP__

#include "WiFiManager.h"
#include "api_bean.hpp"
#include "tft_draw.hpp"
#include "webapi.hpp"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

int cityId = 0;
struct tm timeNow;
char buf[256] = {0};
String scrollText[5];
LocInfo locInfo;
WeatherInfo weaInfo;
u8 wifiSleeping = 0;

using namespace std;

void inline parseCityInfo(JsonDocument &info) {
  cityId = info["cityId"];
  locInfo.cityId = cityId;
  auto ipInfo = info["ip"];
  locInfo.cityName = (String)ipInfo["district"];
}

void inline parseWeatherInfo(JsonDocument &info) {
  weaInfo.minTemp = info["MINTEM"];
  weaInfo.maxTemp = info["MAXTEM"];
  weaInfo.curTemp = info["TEMP"];
  weaInfo.humi = info["HUMI"];
  weaInfo.aqi = info["aqi"];
  weaInfo.condition = (String)info["CONDITIONSTEXT"];
  weaInfo.wind = (String)info["WIND"];
  weaInfo.weatherCode = info["weatherCode"];
  weaInfo.uvTxt = (String)info["ULTRAVIOLETRAYS"];
  scrollText[4] = WiFi.localIP().toString();
}

void loadApiWeather() {
  if (cityId <= 0) {
    auto info = get_cur_city_info();
    parseCityInfo(info);
  }
  auto info = get_now_huge_info(cityId);
  parseWeatherInfo(info);
  refreshWeatherData();
}

void inline webConfigWiFi() {
  textLoading("WiFi Start...", 10);
  WiFiManager wm;
  textLoading("Config WiFi In Web", 40);
  wm.autoConnect();
  textLoading("WiFi Connect OK", 50);
}

void inline startWifiConfig() {
  textLoading("WiFi Start...", 10);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  for (int i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.beginSmartConfig();
    textLoading("Use ESPTouch App", 40);
    while (!WiFi.smartConfigDone()) {
      delay(500);
    }
  }
  while (!WiFi.localIP().isSet()) {
    delay(200);
  }
  textLoading("WiFi Connect OK", 50);
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  textLoading("Start NTP...", 70);
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
  textLoading("NTP Config OK", 99);
  delay(500);
  tft.fillScreen(bgColor);
}

void inline setupOTAConfig() {
  ArduinoOTA.onStart([] {});
  ArduinoOTA.onProgress([](u32_t pro, u32_t total) {});
  ArduinoOTA.onEnd([] {});
  ArduinoOTA.onError([](ota_error_t err) {});
  ArduinoOTA.begin();
}

void run10minTask() {
  if (wifiSleeping == 1) {
    WiFi.forceSleepWake();
    wifiSleeping = 0;
  }
}

void run2sTask() {
  ArduinoOTA.handle();
  scrollBanner();
}

void run300msTask() {
  time_t now = time(nullptr);
  localtime_r(&now, &timeNow);
  digitalClockDisplay();
}

void run100msTask() { animationOneFrame(); }

void inline runNoWaitTask() {
  if (wifiSleeping == 0 && WiFi.status() == WL_CONNECTED) {
    loadApiWeather();
    WiFi.forceSleepBegin();
    wifiSleeping = 1;
  }
}

void inline setupTasks() {
  drawWeatherIcon();
  run100msTask();
  run300msTask();
  run2sTask();
  run10minTask();
}

u64 ms100ms = 0;
u64 ms300ms = 0;
u64 ms2s = 0;
u64 ms10min = 0;

void inline handleLoop() {
  auto cur = millis();
  if (cur - ms100ms > 100) {
    ms100ms = cur;
    run100msTask();
  }
  if (cur - ms300ms > 300) {
    ms300ms = cur;
    run300msTask();
  }
  if (cur - ms2s > 2000) {
    ms2s = cur;
    run2sTask();
  }
  if (cur - ms10min > 600000) {
    ms10min = cur;
    run10minTask();
  }
  runNoWaitTask();
}

#endif