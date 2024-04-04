#ifndef __TFT_DRAW_HPP__
#define __TFT_DRAW_HPP__

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "Animate/Animate.h" //动画模块
#include "api_bean.hpp"
#include "font/ZdyLwFont_20.h"  //字体库
#include "font/timeClockFont.h" //字体库
#include "img/humidity.h"       //湿度图标
#include "img/temperature.h"    //温度图标
#include "sd2.hpp"
#include "weatherNum/weatherNum.h" //天气图库

#define timeY 82 // 定义高度

#define FONT_COLOR_ONE 0x6D9D
#define FONT_COLOR_TWO 0x7FC0
#define FONT_COLOR_THREE 0xEC1D
#define FONT_COLOR_HOUR 0x6D9D
#define FONT_COLOR_MIN 0x7FC0
#define FONT_COLOR_SEC 0xEC1D

const String WEEK_DAYS[7] = {"日", "一", "二", "三", "四", "五", "六"};
extern char buf[];
extern String scrollText[];
extern LocInfo locInfo;
extern WeatherInfo weaInfo;

int brightness = 50;
auto tft = TFT_eSPI();
auto clk = TFT_eSprite(&tft);

int currentIndex = 0;
int prevTime = 0; // 滚动显示更新标志位

int tempnum = 0;      // 温度百分比
int huminum = 0;      // 湿度百分比
int tempcol = 0xffff; // 温度显示颜色
int humicol = 0xffff; // 湿度显示颜色
uint16_t bgColor = 0x0000;
WeatherNum wrat;

const uint8_t *Animate_value; // 指向关键帧的指针
u32 Animate_size;             // 指向关键帧大小的指针

inline void setupTFT() {
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  // 设置背光
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (brightness * 10));
}

// TFT屏幕输出函数
bool tft_output(s16 x, s16 y, u16 w, u16 h, u16 *bitmap) {
  if (y >= tft.height()) {
    return 0;
  }
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

inline void animationOneFrame() {
  imgAnim(&Animate_value, &Animate_size);
  TJpgDec.drawJpg(160, 160, Animate_value, Animate_size);
}

void inline setupJPEG() {
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
}

// 绘制进度条
void textLoading(const char *txt, u8 progress) {
  clk.setColorDepth(8);
  clk.createSprite(200, 100);
  clk.fillScreen(TFT_BLACK);
  clk.drawRoundRect(0, 40, 200, 20, 3, TFT_WHITE);
  clk.fillRoundRect(2, 42, progress * 2, 16, 2, FONT_COLOR_THREE);
  clk.loadFont(ZdyLwFont_20);
  clk.setTextColor(FONT_COLOR_ONE);
  clk.drawCentreString("Welcome to SD2", 100, 0, 4);
  clk.setTextColor(FONT_COLOR_TWO);
  clk.drawCentreString(txt, 100, 80, 4);
  clk.unloadFont();
  clk.pushSprite(20, 80);
}

// 湿度图标显示函数
void humidityWin() {
  clk.setColorDepth(8);
  huminum = huminum / 2;
  clk.createSprite(52, 6); // 创建窗口
  clk.fillSprite(0x0000);  // 填充率
  // 空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  clk.fillRoundRect(1, 1, huminum, 4, 2, humicol); // 实心圆角矩形
  clk.pushSprite(45, 222);                         // 窗口位置
  clk.deleteSprite();
}

// 温度图标显示函数
void tempWin() {
  clk.setColorDepth(8);
  clk.createSprite(52, 6); // 创建窗口
  clk.fillSprite(0x0000);  // 填充率
  // 空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  clk.fillRoundRect(1, 1, tempnum, 4, 2, tempcol); // 实心圆角矩形
  clk.pushSprite(45, 192);                         // 窗口位置
  clk.deleteSprite();
}

void inline drawWeatherIcon() {
  TJpgDec.drawJpg(15, 183, temperature, sizeof(temperature)); // 温度图标
  TJpgDec.drawJpg(15, 213, humidity, sizeof(humidity));       // 湿度图标
}

// 天气信息写到屏幕上
void refreshWeatherData() {
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  // 温度
  tempnum = weaInfo.curTemp;
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  sprintf(buf, "%d℃", tempnum);
  clk.drawString(buf, 28, 13);
  clk.pushSprite(100, 184);
  clk.deleteSprite();

  if (tempnum < 10)
    tempcol = 0x00FF;
  else if (tempnum < 28)
    tempcol = 0x0AFF;
  else if (tempnum < 34)
    tempcol = 0x0F0F;
  else if (tempnum < 41)
    tempcol = 0xFF0F;
  else if (tempnum < 49)
    tempcol = 0xF00F;
  else {
    tempcol = 0xF00F;
  }
  tempWin();

  // 湿度
  huminum = weaInfo.humi;
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  sprintf(buf, "%d%%", huminum);
  clk.drawString(buf, 28, 13);
  clk.pushSprite(100, 214);
  clk.deleteSprite();

  if (huminum > 90)
    humicol = 0x00FF;
  else if (huminum > 70)
    humicol = 0x0AFF;
  else if (huminum > 40)
    humicol = 0x0F0F;
  else if (huminum > 20)
    humicol = 0xFF0F;
  else
    humicol = 0xF00F;
  humidityWin();

  // 城市名称
  clk.createSprite(94, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(locInfo.cityName.c_str(), 44, 16);
  clk.pushSprite(15, 15);
  clk.deleteSprite();

  // PM2.5空气指数
  uint16_t pm25BgColor = tft.color565(156, 202, 127); // 优
  String aqiTxt = "优";
  int pm25V = weaInfo.aqi; // aqi
  if (pm25V > 200) {
    pm25BgColor = tft.color565(136, 11, 32); // 重度
    aqiTxt = "重度";
  } else if (pm25V > 150) {
    pm25BgColor = tft.color565(186, 55, 121); // 中度
    aqiTxt = "中度";
  } else if (pm25V > 100) {
    pm25BgColor = tft.color565(242, 159, 57); // 轻
    aqiTxt = "轻度";
  } else if (pm25V > 50) {
    pm25BgColor = tft.color565(247, 219, 100); // 良
    aqiTxt = "良";
  }
  clk.createSprite(56, 24);
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0, 0, 50, 24, 4, pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0x0000);
  clk.drawString(aqiTxt, 25, 13);
  clk.pushSprite(104, 18);
  clk.deleteSprite();
  scrollText[0] = "天气 " + String(weaInfo.condition.c_str());
  scrollText[1] = "空气 " + aqiTxt;
  scrollText[2] = "风向 " + String(weaInfo.wind.c_str());
  sprintf(buf, "%d℃ - %d℃", weaInfo.minTemp, weaInfo.maxTemp);
  scrollText[3] = buf;
  wrat.printfweather(170, 15, weaInfo.weatherCode);
  clk.unloadFont();
}

void scrollBanner() {
  if (scrollText[currentIndex]) {
    clk.setColorDepth(8);
    clk.loadFont(ZdyLwFont_20);
    clk.createSprite(150, 30);
    clk.fillSprite(bgColor);
    clk.setTextWrap(false);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, bgColor);
    clk.drawString(scrollText[currentIndex], 74, 16);
    clk.pushSprite(10, 45);
    clk.deleteSprite();
    clk.unloadFont();
    if (currentIndex >= 5)
      currentIndex = 0; // 回第一个
    else
      currentIndex += 1; // 准备切换到下一个
  }
  prevTime = 1;
}

// 用快速线方法绘制数字
void drawLineFont(u32 _x, u32 _y, u32 _num, u32 _size, u32 _color) {
  u32 fontSize;
  const LineAtom *f1;
  // 小号(9*14)
  if (_size == 1) {
    f1 = smallLineFont[_num];
    fontSize = smallLineFont_size[_num];
    // 绘制前清理字体绘制区域
    tft.fillRect(_x, _y, 9, 14, TFT_BLACK);
  }
  // 中号(18*30)
  else if (_size == 2) {
    f1 = middleLineFont[_num];
    fontSize = middleLineFont_size[_num];
    // 绘制前清理字体绘制区域
    tft.fillRect(_x, _y, 18, 30, TFT_BLACK);
  }
  // 大号(36*90)
  else if (_size == 3) {
    f1 = largeLineFont[_num];
    fontSize = largeLineFont_size[_num];
    // 绘制前清理字体绘制区域
    tft.fillRect(_x, _y, 36, 60, TFT_BLACK);
  } else {
    return;
  }
  for (u32 i = 0; i < fontSize; i++) {
    auto x = f1[i].xValue + _x;
    auto y = f1[i].yValue + _y;
    tft.drawFastHLine(x, y, f1[i].lValue, _color);
  }
}

int Hour_sign = -1;
int Minute_sign = -1;
int Second_sign = -1;
int YDay_sign = -1;
extern struct tm timeNow;

// 日期刷新
void digitalClockDisplay(s8 force = 0) {
  // 时钟刷新,输入1强制刷新
  auto now_hour = timeNow.tm_hour;  // 获取小时
  auto now_minute = timeNow.tm_min; // 获取分钟
  auto now_second = timeNow.tm_sec; // 获取秒针
  auto yDay = timeNow.tm_yday;
  // 小时刷新
  if ((now_hour != Hour_sign) || (force == 1)) {
    drawLineFont(20, timeY, now_hour / 10, 3, FONT_COLOR_HOUR);
    drawLineFont(60, timeY, now_hour % 10, 3, FONT_COLOR_HOUR);
    Hour_sign = now_hour;
  }
  // 分钟刷新
  if ((now_minute != Minute_sign) || (force == 1)) {
    drawLineFont(101, timeY, now_minute / 10, 3, FONT_COLOR_MIN);
    drawLineFont(141, timeY, now_minute % 10, 3, FONT_COLOR_MIN);
    Minute_sign = now_minute;
  }
  // 秒针刷新
  if ((now_second != Second_sign) || (force == 1)) {
    drawLineFont(182, timeY + 30, now_second / 10, 2, FONT_COLOR_SEC);
    drawLineFont(202, timeY + 30, now_second % 10, 2, FONT_COLOR_SEC);
    Second_sign = now_second;
  }
  if (yDay != YDay_sign || (force == 1)) {
    YDay_sign = yDay;
    clk.setColorDepth(8);
    clk.loadFont(ZdyLwFont_20);
    // 星期
    clk.createSprite(58, 30);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, bgColor);
    clk.drawString("周" + WEEK_DAYS[timeNow.tm_wday], 29, 16);
    clk.pushSprite(102, 150);
    clk.deleteSprite();
    // 月日
    clk.createSprite(95, 30);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, bgColor);
    sprintf(buf, "%02d月%02d日", timeNow.tm_mon + 1, timeNow.tm_mday);
    clk.drawString(buf, 49, 16);
    clk.pushSprite(5, 150);
    clk.deleteSprite();
    clk.unloadFont();
  }
}

#endif