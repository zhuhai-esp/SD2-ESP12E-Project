#include <Adafruit_ST7789.h>
#include <Arduino.h>

#define TFT_BL D1
int LCD_BL_PWM = 70;

Adafruit_ST7789 tft = Adafruit_ST7789(D8, D3, D4);

void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));

  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.drawCircle(120, 120, 30, ST77XX_YELLOW);

  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextWrap(true);
  tft.print("Hello World!!");
}

void loop() {}
