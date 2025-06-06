#ifndef __SD2_HPP__
#define __SD2_HPP__

#include "Animate/Animate.h"
#include "font/ZoloFont_20.h"
#include "font/timeClockFont.h" //字体库
#include "img/humidity.h"       //湿度图标
#include "img/temperature.h"    //温度图标
#include "weatherNum/weatherNum.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <WiFiManager.h>

#define PIN_WS2812 12
#define PIN_BUTTON 4

#define NUM_LEDS 1

#define HTTP_UA                                                                \
  "Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) "                    \
  "AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 "       \
  "Safari/604.1"

#define FONT_COLOR_HOUR 0x6D9D
#define FONT_COLOR_MIN 0x7FC0
#define FONT_COLOR_SEC 0xEC1D

const String WEEK_DAYS[7] = {"日", "一", "二", "三", "四", "五", "六"};
Adafruit_NeoPixel pixels(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);
WiFiManager wm;
char buf[256];
WiFiClient wificlient;
uint16_t bgColor = 0x0000;
int tempnum = 0;      // 温度百分比
int huminum = 0;      // 湿度百分比
int tempcol = 0xffff; // 温度显示颜色
int humicol = 0xffff; // 湿度显示颜色
String scrollText[6];
WeatherNum wrat;
int currentIndex = 0;
int Hour_sign = -1;
int Minute_sign = -1;
int Second_sign = -1;
int YDay_sign = -1;
struct tm timeNow;
const uint8_t *Animate_value;
uint32_t Animate_size;
ESP8266WebServer server(80);

int BL_addr = 1;  // 被写入数据的EEPROM地址编号  1亮度
int Ro_addr = 2;  // 被写入数据的EEPROM地址编号  2 旋转方向
int Pix_addr = 3; // LED亮度 0~255
int CC_addr = 10; // 被写入数据的EEPROM地址编号  10城市

int LCD_Rotation = 0;   // LCD屏幕方向
int lcdBL = 50;         // 屏幕亮度0-100，默认50
int ledBrightness = 50; // led亮度
String cityCode = "";

void animationOneFrame();
void loadInitWeather();
void showTimeDate(uint8_t force);
void getCityWeater();

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void saveCityCodetoEEP(int *citycode) {
  for (int cnum = 0; cnum < 5; cnum++) {
    EEPROM.write(CC_addr + cnum, *citycode % 100);
    EEPROM.commit();
    *citycode = *citycode / 100;
    delay(5);
  }
}

void readCityCodefromEEP(int *citycode) {
  for (int cnum = 5; cnum > 0; cnum--) {
    *citycode = *citycode * 100;
    *citycode += EEPROM.read(CC_addr + cnum - 1);
    delay(5);
  }
}

inline String generateHTML(bool reload = false) {
  String html = F("<!DOCTYPE html><html lang='en'>");
  html += F("<head>");
  html += F("<meta charset='UTF-8'>");
  html += F("<meta name='viewport' content='width=device-width, "
            "initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, "
            "user-scalable=no, viewport-fit=cover'>");
  html += F("<title>SD2 Config</title>");
  if (reload) {
    html += F("<script>setTimeout(function(){ window.location.href='/'; }, "
              "2000);</script>");
  }
  html += F("</head>");
  html += F("<body style='text-align:center'>");
  html += F("<h2>SD2 Config</h2>");
  html += F("<form action='/' method='POST'>");
  html += F("<p>城市代码：<input type='text' name='web_cityCode' value='");
  html += cityCode;
  html += F("'></p>");
  html += F("<p>屏幕亮度：<input type='text' name='web_bl' placeholder='0~255' "
            "value='");
  html += lcdBL;
  html += F("'></p>");
  html += F("<p>屏幕方向：<input type='text' name='web_rotation' "
            "placeholder='0~3' value='");
  html += LCD_Rotation;
  html += F("'></p>");
  html +=
      F("<p>LED亮度：<input type='text' name='web_ledb' placeholder='0~255' "
        "value='");
  html += ledBrightness;
  html += F("'></p>");
  html += F("<p>");
  html += F("<button type='submit' name='save'>保存</button>&nbsp");
  html += F("<button type='submit' name='reset'>重启</button>");
  html += F("</p>");
  html += F("</form>");
  html += F("</body>");
  html += F("</html>");
  return html;
}

inline void handleFromPost() {
  if (server.hasArg("reset")) {
    server.send(200, "text/html", generateHTML(true));
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 4);
    tft.loadFont(ZoloFont_20);
    tft.print("Restarting...");
    tft.unloadFont();
    delay(2000);
    ESP.reset();
  } else if (server.hasArg("save")) {
    int web_cc, web_setro, web_lcdbl, web_ledb;
    web_cc = server.arg("web_cityCode").toInt();
    web_setro = server.arg("web_rotation").toInt();
    web_lcdbl = server.arg("web_bl").toInt();
    web_ledb = server.arg("web_ledb").toInt();
    if (web_cc >= 101000000 && web_cc <= 102000000) {
      saveCityCodetoEEP(&web_cc);
      readCityCodefromEEP(&web_cc);
      cityCode = web_cc;
      loadInitWeather();
      showTimeDate(1);
    }
    if (web_lcdbl >= 0 && web_lcdbl <= 255) {
      EEPROM.write(BL_addr, web_lcdbl);
      EEPROM.commit();
      lcdBL = EEPROM.read(BL_addr);
      analogWrite(TFT_BL, 255 - lcdBL);
    }
    if (web_ledb >= 0 && web_ledb <= 255) {
      EEPROM.write(Pix_addr, web_ledb);
      EEPROM.commit();
      ledBrightness = EEPROM.read(Pix_addr);
      pixels.setBrightness(ledBrightness);
    }
    EEPROM.write(Ro_addr, web_setro);
    EEPROM.commit();
    if (web_setro != LCD_Rotation) {
      LCD_Rotation = web_setro;
      tft.setRotation(LCD_Rotation);
      tft.fillScreen(0x0000);
      TJpgDec.drawJpg(15, 183, temperature, sizeof(temperature));
      TJpgDec.drawJpg(15, 213, humidity, sizeof(humidity));
      showTimeDate(1);
      getCityWeater();
    }
    server.send(200, "text/html", generateHTML(true));
  }
}

void handleConfig() {
  if (server.method() == HTTP_POST) {
    handleFromPost();
  } else {
    server.send(200, "text/html", generateHTML());
  }
}

void inline webServerInit() {
  server.on("/", handleConfig);
  server.onNotFound(handleNotFound);
  server.begin();
}

void webServeUpdate() { server.handleClient(); }

void inline tempWin() {
  clk.setColorDepth(8);
  clk.createSprite(52, 6); // 创建窗口
  clk.fillSprite(0x0000);  // 填充率
  clk.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  clk.fillRoundRect(1, 1, tempnum, 4, 2, tempcol); // 实心圆角矩形
  clk.pushSprite(45, 192);                         // 窗口位置
  clk.deleteSprite();
}

void inline humidityWin() {
  clk.setColorDepth(8);
  huminum = huminum / 2;
  clk.createSprite(52, 6); // 创建窗口
  clk.fillSprite(0x0000);  // 填充率
  clk.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);
  clk.fillRoundRect(1, 1, huminum, 4, 2, humicol); // 实心圆角矩形
  clk.pushSprite(45, 222);                         // 窗口位置
  clk.deleteSprite();
}

void inline parseWeatherData(String *cityDZ, String *dataSK, String *dataFC) {
  JsonDocument doc;
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();
  clk.setColorDepth(8);
  clk.loadFont(ZoloFont_20);
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(sk["temp"].as<String>() + "℃", 28, 13);
  clk.pushSprite(100, 184);
  clk.deleteSprite();
  tempnum = sk["temp"].as<int>();
  tempnum = tempnum + 10;
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
    tempnum = 50;
  }
  tempWin();
  clk.createSprite(58, 24);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(sk["SD"].as<String>(), 28, 13);
  clk.pushSprite(100, 214);
  clk.deleteSprite();
  huminum = atoi((sk["SD"].as<String>()).substring(0, 2).c_str());
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
  clk.drawString(sk["cityname"].as<String>(), 44, 16);
  clk.pushSprite(15, 15);
  clk.deleteSprite();

  // PM2.5空气指数
  uint16_t pm25BgColor = tft.color565(156, 202, 127); // 优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
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
  clk.unloadFont();

  scrollText[0] = "天气 " + sk["weather"].as<String>();
  scrollText[1] = "空气 " + aqiTxt;
  scrollText[2] = "风向 " + sk["WD"].as<String>() + sk["WS"].as<String>();

  auto cond = atoi((sk["weathercode"].as<String>()).substring(1, 3).c_str());
  wrat.printfweather(170, 15, cond);

  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  scrollText[3] = dz["weather"].as<String>();

  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();

  scrollText[4] = fc["fd"].as<String>() + "℃ - " + fc["fc"].as<String>() + "℃";
  scrollText[5] = WiFi.localIP().toString();
}

void inline getCityWeater() {
  String URL = "http://d1.weather.com.cn/weather_index/" + cityCode +
               ".html?_=" + String(millis());
  HTTPClient httpClient;
  httpClient.begin(wificlient, URL);
  httpClient.setUserAgent(HTTP_UA);
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
  int httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":");
    int indexEnd = str.indexOf("};var alarmDZ");
    String jsonCityDZ = str.substring(indexStart + 13, indexEnd);
    indexStart = str.indexOf("dataSK =");
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart + 8, indexEnd);
    indexStart = str.indexOf("\"f\":[");
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart + 5, indexEnd);
    parseWeatherData(&jsonCityDZ, &jsonDataSK, &jsonFC);
  } else {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }
  httpClient.end();
}

void inline getCityCode() {
  String URL = "http://wgeo.weather.com.cn/ip/?_=" + String(millis());
  HTTPClient httpClient;
  httpClient.begin(wificlient, URL);
  httpClient.setUserAgent(HTTP_UA);
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
  int httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
    String str = httpClient.getString();
    int aa = str.indexOf("id=");
    if (aa > -1) {
      cityCode = str.substring(aa + 4, aa + 4 + 9);
      Serial.println(cityCode);
      getCityWeater();
    } else {
      Serial.println("获取城市代码失败");
    }
  } else {
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }
  httpClient.end();
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bpm) {
  if (y >= tft.height())
    return 0;
  tft.pushImage(x, y, w, h, bpm);
  return 1;
}

void inline initTJpeg() {
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
}

void inline autoConfigWifi() {
  wm.setAPCallback([](WiFiManager *_wm) {
    tft.println("WiFi Failed!");
    tft.println("Please Connect AP:");
    tft.setTextColor(TFT_GREEN);
    tft.println(_wm->getConfigPortalSSID().c_str());
    tft.setTextColor(TFT_WHITE);
    tft.println("\nThen Open Link:");
    tft.setTextColor(TFT_GREEN);
    tft.printf("http://%s\n", WiFi.softAPIP().toString().c_str());
    tft.setTextColor(TFT_WHITE);
    tft.unloadFont();
  });
  wm.setConfigPortalTimeout(180);
  wm.setConfigPortalTimeoutCallback([]() { ESP.restart(); });
  auto res = wm.autoConnect();
  if (!res) {
    ESP.restart();
  }
  tft.loadFont(ZoloFont_20);
  tft.println("WiFi Connected!");
  sprintf(buf, "IP: %s", WiFi.localIP().toString().c_str());
  tft.println(buf);
  tft.unloadFont();
}

void inline startConfigTime() {
  tft.loadFont(ZoloFont_20);
  tft.println("Start Config Time!");
  tft.unloadFont();
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(300);
  }
  delay(1000);
}

void inline initPixels() {
  pixels.begin();
  pixels.setBrightness(ledBrightness);
  pixels.clear();
  pixels.show();
}

inline void loadSavedConfig() {
  if (EEPROM.read(BL_addr) >= 0 && EEPROM.read(BL_addr) <= 255) {
    lcdBL = EEPROM.read(BL_addr);
  }
  if (EEPROM.read(Pix_addr) >= 0 && EEPROM.read(Pix_addr) <= 255) {
    ledBrightness = EEPROM.read(Pix_addr);
  }
  if (EEPROM.read(Ro_addr) >= 0 && EEPROM.read(Ro_addr) <= 3) {
    LCD_Rotation = EEPROM.read(Ro_addr);
  }
  int CityCODE = 0;
  readCityCodefromEEP(&CityCODE);
  if (CityCODE >= 101000000 && CityCODE <= 102000000) {
    cityCode = CityCODE;
  }
}

void inline initDisplay() {
  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(LCD_Rotation);
  tft.fillScreen(0x0000);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(8);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 255 - lcdBL);
  tft.setCursor(0, 4);
  tft.setTextColor(TFT_WHITE);
  tft.loadFont(ZoloFont_20);
  tft.println("Hello World...");
}

void inline loadInitWeather() {
  tft.fillScreen(TFT_BLACK);
  TJpgDec.drawJpg(15, 183, temperature, sizeof(temperature));
  TJpgDec.drawJpg(15, 213, humidity, sizeof(humidity));
  if (cityCode.isEmpty()) {
    getCityCode();
  } else {
    getCityWeater();
  }
}

void scrollBanner() {
  if (scrollText[currentIndex]) {
    clk.setColorDepth(8);
    clk.loadFont(ZoloFont_20);
    clk.createSprite(150, 30);
    clk.fillSprite(bgColor);
    clk.setTextWrap(false);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, bgColor);
    clk.drawString(scrollText[currentIndex], 74, 16);
    clk.pushSprite(10, 45);
    clk.deleteSprite();
    clk.unloadFont();
    currentIndex = (currentIndex + 1) % 6;
  }
}

void drawLineFont(uint32_t _x, uint32_t _y, uint32_t _num, uint32_t _size,
                  uint32_t c) {
  uint32_t fontSize;
  const LineAtom *fontOne;
  if (_size == 1) {
    fontOne = smallLineFont[_num];
    fontSize = smallLineFont_size[_num];
    tft.fillRect(_x, _y, 9, 14, TFT_BLACK);
  } else if (_size == 2) {
    fontOne = middleLineFont[_num];
    fontSize = middleLineFont_size[_num];
    tft.fillRect(_x, _y, 18, 30, TFT_BLACK);
  } else if (_size == 3) {
    fontOne = largeLineFont[_num];
    fontSize = largeLineFont_size[_num];
    tft.fillRect(_x, _y, 36, 60, TFT_BLACK);
  } else
    return;
  for (uint32_t i = 0; i < fontSize; i++) {
    tft.drawFastHLine(fontOne[i].xValue + _x, fontOne[i].yValue + _y,
                      fontOne[i].lValue, c);
  }
}

void showTimeDate(uint8_t force = 0) {
  time_t now = time(nullptr);
  localtime_r(&now, &timeNow);
  auto now_hour = timeNow.tm_hour;  // 获取小时
  auto now_minute = timeNow.tm_min; // 获取分钟
  auto now_second = timeNow.tm_sec; // 获取秒针
  auto yDay = timeNow.tm_yday;
  uint32_t timeY = 82;
  if (now_hour / 10 != Hour_sign / 10 || (force == 1)) {
    drawLineFont(20, timeY, now_hour / 10, 3, FONT_COLOR_HOUR);
  }
  if (now_hour % 10 != Hour_sign % 10 || (force == 1)) {
    drawLineFont(60, timeY, now_hour % 10, 3, FONT_COLOR_HOUR);
  }
  Hour_sign = now_hour;
  if ((now_minute / 10 != Minute_sign / 10) || (force == 1)) {
    drawLineFont(101, timeY, now_minute / 10, 3, FONT_COLOR_MIN);
  }
  if ((now_minute % 10 != Minute_sign % 10) || (force == 1)) {
    drawLineFont(141, timeY, now_minute % 10, 3, FONT_COLOR_MIN);
  }
  Minute_sign = now_minute;
  if ((now_second / 10 != Second_sign / 10) || (force == 1)) {
    drawLineFont(182, timeY + 30, now_second / 10, 2, FONT_COLOR_SEC);
  }
  if ((now_second % 10 != Second_sign % 10) || (force == 1)) {
    drawLineFont(202, timeY + 30, now_second % 10, 2, FONT_COLOR_SEC);
  }
  Second_sign = now_second;
  if (yDay != YDay_sign || (force == 1)) {
    YDay_sign = yDay;
    clk.setColorDepth(8);
    clk.loadFont(ZoloFont_20);
    clk.createSprite(160, 30);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, bgColor);
    auto *we = WEEK_DAYS[timeNow.tm_wday].c_str();
    sprintf(buf, "%02d月%02d日 周%s", timeNow.tm_mon + 1, timeNow.tm_mday, we);
    clk.drawString(buf, 80, 15);
    clk.pushSprite(0, 150);
    clk.deleteSprite();
    clk.unloadFont();
  }
}

inline void animationOneFrame() {
  imgAnim(&Animate_value, &Animate_size);
  TJpgDec.drawJpg(160, 160, Animate_value, Animate_size);
}

#endif