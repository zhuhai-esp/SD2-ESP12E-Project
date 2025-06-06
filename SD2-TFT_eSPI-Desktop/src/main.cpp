#include <FS.h>
#include "SD2.hpp"

uint64_t ms100ms = 0, ms300ms = 0, ms2s = 0, ms10min = 0;

void run10minTask() { getCityWeater(); }

void run2sTask() { scrollBanner(); }

void run300msTask() {
  showTimeDate();
  uint32_t c = random();
  pixels.setPixelColor(0, c);
  pixels.show();
}

void run100msTask() { animationOneFrame(); }

void setup() {
  Serial.begin(115200);
  EEPROM.begin(1024);
  loadSavedConfig();
  initDisplay();
  initPixels();
  autoConfigWifi();
  startConfigTime();
  initTJpeg();
  animationOneFrame();
  loadInitWeather();
  showTimeDate(1);
  webServerInit();
}

void loop() {
  auto cur = millis();
  webServeUpdate();
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