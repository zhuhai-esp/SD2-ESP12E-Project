/* *****************************************************************
 *
 * SmallDesktopDisplay2.0
 *    小型桌面显示器
 *
 * 作      者：Misaka
 * 修  改  者：NodYoung
 * 创 建 日 期：2022.01.16
 * 最后更改日期：2022.02.13
 * 更 改 说 明：Web联网部分基于网友“微车游”V1.3.4版本程序更改
 *                V2.0.0  初次创建
 *
 *                按键切换 更改为自动切换 默认15秒。适配没按键的小电视。
 *
 * 引 脚 分 配： SCK  GPIO14
 *             MOSI  GPIO13
 *             RES   GPIO2
 *             DC    GPIO0
 *             LCDBL GPIO5
 *
 *             KEY   GPIO4  低电平按下
 *
 * 转载请著名出处
 *
 * *****************************************************************/
#include "display.h"
#include "number.h"
#include "weathernum.h"
#include <SPI.h>
#include <WiFiManager.h>

/*** Component objects ***/
Display screen;

/***  ***/
int loadNUM = 0;             // 默认进度条长度
unsigned char gethour = 0;   // 时
unsigned char getminute = 0; // 分
unsigned char getsecond = 0; // 秒
unsigned char getweek = 0;   // 星期
unsigned char getmonth = 0;  // 月
unsigned char getday = 0;    // 日
unsigned char APPMODE = 0;   // 当前运行的App
unsigned char APPNUM = 2;    // APP个数
boolean APPflagChange = false;

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
}

void gettime();

void setup() {
  Serial.begin(115200);
  EEPROM.begin(20);
  screen.TFT_init();
  WiFiManager wm;
  wm.autoConnect();
  Serial.print("正在连接WIFI ");
  startConfigTime();
  while (loadNUM < 200) {
    screen.loading(loadNUM); // 绘制进度条
    loadNUM += 1;
    delay(1);
  }
  screen.TFT_CLS(TFT_WHITE);
}

/*** App运行***/
void AppRun() {
  switch (APPMODE) {
  case 0:
    // APP1--时间
    screen.app1(gethour, getminute, getsecond, getmonth, getday, getweek);
    break;
  case 1:
    // APP2--天气
    screen.app2(3000);
    break;
  default:
    Serial.println("显示主界面错误");
    break;
  }
}

/*** 循环 ***/
void loop() {
  gettime(); // 获取时间
  AppRun();  // App运行
  if ((millis() / 1000) % 15 == 0) {
    APPflagChange = true;
    if (APPflagChange) { // KeyNum ==3
      delay(1000);
      APPMODE++;
      screen.APP_flag();
      if (APPMODE == APPNUM) {
        APPMODE = 0;
      }
      APPflagChange = false;
    }
  }
}

/*** 获取时间 ***/
void gettime() {
  struct tm info;
  getLocalTime(&info);
  gethour = info.tm_hour;
  getminute = info.tm_min;
  getsecond = info.tm_sec;
  getweek = info.tm_wday + 1;
  getmonth = info.tm_mon + 1;
  getday = info.tm_mday;
}