#ifndef __SD2_HPP__
#define __SD2_HPP__

#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <WiFiManager.h>

#define PIN_WS2812 12
#define PIN_BUTTON 4

#define NUM_LEDS 1

Adafruit_NeoPixel pixels(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
uint16_t brightness = 10;
uint8_t rotation = 0;
WiFiManager wm;
char buf[256];

void inline autoConfigWifi() {
  tft.println("Start Config WiFi!");
  auto res = wm.autoConnect();
  if (!res) {
    ESP.restart();
  }
  tft.println("WiFi Connected!");
  sprintf(buf, "IP: %s", WiFi.localIP().toString().c_str());
  tft.println(buf);
}

void inline startConfigTime() {
  tft.println("Start Config Time!");
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(300);
  }
}

void inline initPixels() {
  pixels.begin();
  pixels.setBrightness(200);
  pixels.clear();
  pixels.show();
}

void inline initDisplay() {
  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(rotation);
  tft.fillScreen(0x0000);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (brightness * 10));
  tft.setCursor(0, 16);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSerif9pt7b);
}

#endif