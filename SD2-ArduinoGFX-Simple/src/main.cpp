#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define TFT_BL D1
#define LCD_BL_PWM 5

Arduino_DataBus *bus = new Arduino_HWSPI(D3, D8);
Arduino_GFX *tft = new Arduino_ST7789(bus, D4, 0, true, 240, 240);

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));
  tft->begin();
  tft->fillScreen(BLACK);
  tft->setCursor(10, 10);
  tft->setTextColor(GREEN);
  tft->setTextSize(2);
  tft->println("Hello World!");
  tft->drawCircle(120, 120, 40, YELLOW);
  tft->drawRect(30, 30, 40, 50, BLUE);
  tft->drawRoundRect(20, 100, 180, 80, 8, WHITE);
}

void loop() {}
