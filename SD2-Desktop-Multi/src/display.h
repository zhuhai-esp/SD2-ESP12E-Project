#ifndef DISPLAY_H
#define DISPLAY_H

#include "ArduinoJson.h"
#include "number.h"
#include "weathernum.h"
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "img/logo1.h"
// #include "font/ZdyLwFont_20.h"
#include "font/ZoloFont_20.h"
#include "img/humidity.h"
#include "img/temperature.h"

#define LCD_BL_PIN 5         // 背光引脚
#define Version "SDD V2.0.0" // 软件版本

class Display {
private:
  int LCD_BL_PWM = 10; // 默认亮度，0-100
  int LCD_Rotation = 4; // LCD屏幕方向 1USB朝右  2USB朝上  3USB朝左  4USB朝下
  int UpdateWeater_Time = 10; // 天气更新时间min

  unsigned char Hour_sign = 60;
  unsigned char Minute_sign = 60;
  unsigned char Second_sign = 60;

  boolean APPflag = true; // true第一次启动

public:
  void TFT_init();              // 显示初始化
  void loading(byte delayTime); // 绘制进度条
  void TFT_CLS(int bgColor);    // 绘制颜色
  void APP_flag();              // app重进标志
  void app1(unsigned char Get_Hour, unsigned char Get_Minute,
            unsigned char Get_Second, unsigned char Get_Month,
            unsigned char Get_Day, unsigned char Get_Week);
  void app2(int xms);
};

#endif
