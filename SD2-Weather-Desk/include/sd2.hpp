#ifndef __SD2_BOARD_HPP__
#define __SD2_BOARD_HPP__

#include "api_bean.hpp"
#include "tft_draw.hpp"
#include "webapi.hpp"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

int cityId = 0;
struct tm timeNow;
char buf[256] = {0};
String scrollText[6];
LocInfo locInfo;
WeatherInfo weaInfo;

using namespace std;

void inline parseCityInfo(json &info) {
  cityId = info["cityId"];
  locInfo.cityId = cityId;
  auto ipInfo = info["ip"];
  locInfo.cityName = ipInfo["district"];
}

void inline parseWeatherInfo(json &info) {
  weaInfo.minTemp = info["MINTEM"];
  weaInfo.maxTemp = info["MAXTEM"];
  weaInfo.curTemp = info["TEMP"];
  weaInfo.humi = info["HUMI"];
  weaInfo.aqi = info["aqi"];
  weaInfo.condition = info["CONDITIONSTEXT"];
  weaInfo.wind = info["WIND"];
  weaInfo.weatherCode = info["weatherCode"];
  weaInfo.uvTxt = info["ULTRAVIOLETRAYS"];
  scrollText[4] = "UV " + String(weaInfo.uvTxt.c_str());
  scrollText[5] = WiFi.localIP().toString();
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

void inline startWifiConfig() {
  textLoading("WiFi Start...", 10);
  delay(1000);
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
      delay(1000);
    }
  }
  while (!WiFi.localIP().isSet()) {
    delay(200);
  }
  textLoading("WiFi OK", 50);
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

void inline run10minTask() { loadApiWeather(); }

void inline run2sTask() {
  ArduinoOTA.handle();
  scrollBanner();
}

void inline run300msTask() {
  time_t now = time(nullptr);
  localtime_r(&now, &timeNow);
  digitalClockDisplay();
}

void inline run100msTask() { animationOneFrame(); }

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
}

#endif