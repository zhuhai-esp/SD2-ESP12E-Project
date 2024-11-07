#include <Adafruit_ST7789.h>
#include <Arduino.h>
#include <Fonts/FreeSans12pt7b.h>

int LCD_BL_PWM = 2;

#define TFT_BL D1
#define TFT_CS D8
#define TFT_DC D3
#define TFT_RST D4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void inline initTFT() {
  tft.init(240, 240, SPI_MODE2);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));
}

void setup() {
  Serial.begin(115200);
  initTFT();
}

void loop() {
}
