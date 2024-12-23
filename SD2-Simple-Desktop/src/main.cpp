#include "SD2.hpp"

void setup() {
  Serial.begin(115200);
  initDisplay();
  initPixels();
  autoConfigWifi();
  startConfigTime();
}

void loop() {
  pixels.setPixelColor(0, pixels.Color(200, 0, 0));
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(0, 200, 0));
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(0, 0, 200));
  pixels.show();
  delay(1000);
}