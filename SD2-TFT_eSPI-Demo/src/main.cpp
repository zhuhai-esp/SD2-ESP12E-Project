#include <Arduino.h>
#include <TFT_eSPI.h>

int LCD_BL_PWM = 10;
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("Hello World", 120, 120, 4);
}

void loop() {}
