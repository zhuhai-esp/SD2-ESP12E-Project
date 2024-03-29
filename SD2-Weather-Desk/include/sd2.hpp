#ifndef __SD2_BOARD_HPP__
#define __SD2_BOARD_HPP__

#include "webapi.hpp"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <StaticThreadController.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();

Thread threadOTA = Thread();
Thread threadWeather = Thread();
StaticThreadController<2> controller(&threadOTA, &threadWeather);
int cityId = 0;

void loadApiWeather() {
  if (cityId > 0) {
    json info = get_now_huge_info(cityId);
    std::string date = info['date'];
    tft.printf("Weather : [%s]\n", date.c_str());
  }
}

inline void setupTasks() {
  threadOTA.setInterval(1000);
  threadOTA.onRun([]() { ArduinoOTA.handle(); });
  threadWeather.setInterval(2 * 60 * 1000);
  threadWeather.onRun(loadApiWeather);
}

inline void setupInitApi() {
  json info = get_cur_city_info();
  cityId = info["cityId"];
  tft.printf("City ID: [%d]\n", cityId);
}

inline void setupTFT() {
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(2);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));
}

void inline startWifiConfig() {
  tft.println("Start WiFi Connect!");
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  for (int i = 0; i < 10; i++) {
    tft.print("#");
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
  }
  tft.println();
  if (WiFi.status() != WL_CONNECTED) {
    tft.println("No WiFi, Smart Config Begin!!");
    WiFi.beginSmartConfig();
    while (!WiFi.smartConfigDone()) {
      delay(1000);
      tft.print("-");
    }
    tft.println();
  }
  tft.printf("[%s] Connected!!\n", WiFi.macAddress().c_str());
  while (!WiFi.localIP().isSet()) {
    delay(200);
  }
  tft.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  tft.println("Wait for NTP Time!!");
  while (time(nullptr) < 8 * 3600 * 2) {
    tft.print(".");
    delay(500);
  }
  tft.println();
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  tft.printf("Time: %s", asctime(&timeinfo));
}

void inline setupOTAConfig() {
  ArduinoOTA.onStart([] { tft.println("ArduinoOTA Start!!"); });
  ArduinoOTA.onProgress([](u32_t pro, u32_t total) {
    tft.printf("OTA Updating: %d / %d\n", pro, total);
  });
  ArduinoOTA.onEnd([] { tft.println("OTA End, Restarting..."); });
  ArduinoOTA.onError(
      [](ota_error_t err) { tft.printf("OTA Error [%d]!!", err); });
  ArduinoOTA.begin();
}

#endif