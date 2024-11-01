#include "display.h"

/*** Component objects ***/
Number      dig;
WeatherNum  wrat;

TFT_eSPI tft = TFT_eSPI();  // 引脚请自行配置tft_espi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);

WiFiClient wificlient;


//EEPROM参数存储地址位
int BL_Addr = 1;    //被写入数据的EEPROM地址编号  1亮度
int Ro_Addr = 2;    //被写入数据的EEPROM地址编号  2旋转方向
int Uw_Addr = 3;    //被写入数据的EEPROM地址编号  3天气更新时间
int CC_Addr = 10;   //被写入数据的EEPROM地址编号  10城市


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

/* app重进标志 */
void Display::APP_flag()
{
  APPflag = true; //true第一次启动
}
/* 显示初始化 */
void Display::TFT_init()
{
  
  LCD_BL_PWM = EEPROM.read(BL_Addr);
  LCD_Rotation = EEPROM.read(Ro_Addr);
  UpdateWeater_Time = EEPROM.read(Uw_Addr);
  if(LCD_BL_PWM <= 0 || LCD_BL_PWM >= 100)
    LCD_BL_PWM = 10;
  if(LCD_Rotation != 1 || LCD_Rotation != 2 || LCD_Rotation != 3 || LCD_Rotation != 4)
    LCD_Rotation = 4;
  if(UpdateWeater_Time == 0)
    UpdateWeater_Time = 10;
  
  tft.begin();                    // TFT init
  tft.invertDisplay(1);           //反转所有显示颜色：1反转，0正常
  tft.setRotation(LCD_Rotation);  //屏幕方向  1USB朝右  2USB朝上  3USB朝左  4USB朝下
  tft.fillScreen(TFT_WHITE);      //清屏
  clk.setColorDepth(8);           //色深
  clk.loadFont(ZoloFont_20);     //安装字体

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  pinMode(LCD_BL_PIN, OUTPUT);
  analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM*10));

  //开始显示
  TJpgDec.drawJpg(60,40,logo1, sizeof(logo1));//显示logo
  
  clk.createSprite(200,60);       //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);      //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);     //设置文本数据
  clk.setTextColor(TFT_BLACK);    //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(Version,98,20);  //绘制字符串--字符串,x,y  
  clk.pushSprite(20,160);         //窗口位置--x,y
  clk.deleteSprite();             //删除窗口

  delay(500);
}


/* 绘制进度条 */
void Display::loading(byte loadNum)
{
  clk.createSprite(200,60);       //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);      //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);     //设置文本数据
  clk.setTextColor(TFT_BLACK);    //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString("Connecting to WiFi...",98,20);    //绘制字符串--字符串,x,y

  clk.fillRoundRect(0,35,200,6,3,0x7BEF);           //实心圆角矩形--x,y,长度,宽度,圆角半径,颜色
  clk.fillRoundRect(0,35,loadNum,6,3,TFT_ORANGE);    //实心圆角矩形--x,y,长度,宽度,圆角半径,颜色
  
  clk.pushSprite(20,160);         //窗口位置--x,y
  clk.deleteSprite();             //删除窗口
}

/* 清屏 */
void Display::TFT_CLS(int bgColor)    //绘制颜色
{
  tft.fillScreen(bgColor);        //清屏
}

/* Web配网界面 */
void Display::Web_win()
{  
  
  tft.fillScreen(TFT_WHITE);      //清屏
  
  TJpgDec.drawJpg(60,40,logo1, sizeof(logo1));//显示logo
  clk.createSprite(200,60);       //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);      //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);     //设置文本数据
  clk.setTextColor(TFT_BLACK);    //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString("WiFi Connect Fail!",95,10);   //绘制字符串--字符串,x,y  
  clk.drawString("SSID:",25,40);  //绘制字符串--字符串,x,y  
  clk.setTextColor(TFT_BLUE);    //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString("AutoConnectAP",125,40);       //绘制字符串--字符串,x,y  
  
  clk.pushSprite(20,160);         //窗口位置--x,y
  clk.deleteSprite();             //删除窗口
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

/* app1 */
void Display::app1(unsigned char Get_Hour,unsigned char Get_Minute,unsigned char Get_Second,unsigned char Get_Month,unsigned char Get_Day,unsigned char Get_Week)
{
  
  if(APPflag == true)
  {
    tft.fillScreen(TFT_WHITE);        //清屏
  }
  
  if(Get_Hour!=Hour_sign || APPflag == true)//时钟刷新
  {
    dig.printfO80120(10,2,Get_Hour/10);
    dig.printfO80120(90,2,Get_Hour%10);
    Hour_sign = Get_Hour;
  }
  if(Get_Second%2==0)     //秒钟刷新
    TJpgDec.drawJpg(180,2,O_40120_colon, sizeof(O_40120_colon));  //图片--x,y,图片
  else
  {
    clk.createSprite(40,120);       //创建窗口--大小x,y
    clk.fillSprite(TFT_WHITE);      //窗口填充--颜色
    clk.pushSprite(180,2);          //窗口位置--x,y
    clk.deleteSprite();             //删除窗口
  }
  if(Get_Minute!=Minute_sign || APPflag == true)//分钟刷新
  {
    dig.printfB80120(70,120,Get_Minute/10);
    dig.printfB80120(150,120,Get_Minute%10);
    Minute_sign = Get_Minute;

    String wk[7] = {"日","一","二","三","四","五","六"};
    String WK = "周" + wk[(Get_Week)-1];
  
    String MONTH = String(Get_Month);
    MONTH = MONTH + "月";
    String DAY = String(Get_Day);
    DAY = DAY + "日";
    
    clk.createSprite(70,120);       //创建窗口--大小x,y
    clk.fillSprite(TFT_WHITE);      //窗口填充--颜色
    clk.setTextDatum(CC_DATUM);     //设置文本数据
    clk.setTextColor(TFT_BLACK,0x263D);    //设置文本颜色--文本颜色,背景颜色(可删除)
    clk.fillRoundRect(15,17,54,24,5,0x263D);   //实心圆角矩形--x,y,长度,宽度,圆角半径,颜色
    clk.drawString(MONTH,42,30);              //绘制字符串--字符串,x,y
    clk.fillRoundRect(15,47,54,24,5,0x263D);   //实心圆角矩形--x,y,长度,宽度,圆角半径,颜色
    clk.drawString(DAY,42,60);                //绘制字符串--字符串,x,y
    clk.setTextColor(TFT_BLACK,0xEC44);    //设置文本颜色--文本颜色,背景颜色(可删除)
    clk.fillRoundRect(15,77,54,24,5,0xEC44);   //实心圆角矩形--x,y,长度,宽度,圆角半径,颜色
    clk.drawString(WK,42,90);                 //绘制字符串--字符串,x,y
    clk.pushSprite(0,118);          //窗口位置--x,y
    clk.deleteSprite();             //删除窗口
  }
  APPflag = false;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void weaterData(String *cityDZ,String *dataSK,String *dataFC);
void scrollBanner();
void getCityCode();

  String scrollText[7];               //文字数组
  String cityCode   = "101110101";    //城市代码
  int currentIndex  = 0;              //滚动显示某条
  int WeaterTime1   = 0;              //天气时间
  int WeaterTime2   = 0;              //天气时间

// 获取城市天气
void getCityWeater()
{
  //String URL = "http://d1.weather.com.cn/dingzhi/" + cityCode + ".html?_="+String(now());//新
  String URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_="+String(now());//原来
  
  HTTPClient httpClient;    //创建 HTTPClient 对象
  //httpClient.begin(URL); 
  httpClient.begin(wificlient, URL); 
  
  //设置请求头中的User-Agent
  httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();    //启动连接并发送HTTP请求
  Serial.println("正在获取天气数据");
  Serial.println(URL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  if (httpCode == HTTP_CODE_OK) 
  {
    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":");
    int indexEnd = str.indexOf("};var alarmDZ");

    String jsonCityDZ = str.substring(indexStart+13,indexEnd);

    indexStart = str.indexOf("dataSK =");
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart+8,indexEnd);

    
    indexStart = str.indexOf("\"f\":[");
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart+5,indexEnd);
    
    weaterData(&jsonCityDZ,&jsonDataSK,&jsonFC);
    Serial.println("获取成功");
    
  } 
  else 
  {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }
 
  httpClient.end();       //关闭ESP8266与服务器连接
}
void getCityCode()
{
 String URL = "http://wgeo.weather.com.cn/ip/?_="+String(now());
  HTTPClient httpClient;                //创建 HTTPClient 对象
  httpClient.begin(wificlient,URL);     //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  
  //设置请求头中的User-Agent
  httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();      //启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  if (httpCode == HTTP_CODE_OK) 
  {
    String str = httpClient.getString();
    int aa = str.indexOf("id=");
    if(aa>-1)
       getCityWeater();
    else
      Serial.println("获取城市代码失败");    
  } 
  else 
  {
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }
  httpClient.end();       //关闭ESP8266与服务器连接
}

//天气信息写到屏幕上
void weaterData(String *cityDZ,String *dataSK,String *dataFC)
{
  //解析第一段JSON
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();
  
  //----温度----//
  int tempcol = 0xffff;
  int tempnum = 0;      //温度百分比
  
  tempnum = sk["temp"].as<int>();
  tempnum = tempnum+10;
  if(tempnum<10)
    tempcol = tft.color565(132,146,255);//蓝色
  else if(tempnum<28)
    tempcol = tft.color565(132,239,255);//青色
  else if(tempnum<34)
    tempcol = tft.color565(141,255,132);//绿色
  else if(tempnum<41)
    tempcol = tft.color565(255,222,132);//橙色
  else if(tempnum<49)
    tempcol = tft.color565(255,117,117);//红色
  else
  {
    tempcol = tft.color565(222,222,222);//灰色
    tempnum = 50;
  }
  clk.createSprite(60, 26);        //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);        //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);       //设置文本数据
  clk.fillRoundRect(0,0,60,26,4,tempcol);               //实心圆角矩形
  clk.setTextColor(TFT_BLACK);                  //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(sk["temp"].as<String>()+"℃",30,14);   //绘制字符串--字符串,x,y
  clk.pushSprite(30,200);           //窗口位置--x,y
  clk.deleteSprite();               //删除窗口
  TJpgDec.drawJpg(91,201,temperature, sizeof(temperature));  //温度图标
  
  //----湿度----//
  int humicol = 0xffff;
  int huminum = 0;   //湿度百分比
  
  huminum = atoi((sk["SD"].as<String>()).substring(0,2).c_str());
  if(huminum>90)
    humicol = tft.color565(132,146,255);//蓝色
  else if(huminum>70)
    humicol = tft.color565(132,239,255);//青色
  else if(huminum>40)
    humicol = tft.color565(141,255,132);//绿色
  else if(huminum>20)
    humicol = tft.color565(255,222,132);//橙色
  else
    humicol = tft.color565(255,117,117);//红色
  clk.createSprite(60,26);          //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);        //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);       //设置文本数据
  clk.fillRoundRect(0,0,60,26,4,humicol);           //实心圆角矩形
  clk.setTextColor(TFT_BLACK);              //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(sk["SD"].as<String>(),30,14);      //绘制字符串--字符串,x,y
  clk.pushSprite(150,200);          //窗口位置--x,y
  clk.deleteSprite();               //删除窗口
  TJpgDec.drawJpg(125,201,humidity, sizeof(humidity));  //湿度图标

  //----PM2.5空气指数----//
  uint16_t pm25BgColor = tft.color565(131,255,149);//优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if(pm25V>200)
  {
    pm25BgColor = tft.color565(215,85,85);//重度
    aqiTxt = "重度";
  }
  else if(pm25V>150)
  {
    pm25BgColor = tft.color565(219,137,178);//中度
    aqiTxt = "中度";
  }
  else if(pm25V>100)
  {
    pm25BgColor = tft.color565(242,159,57);//轻
    aqiTxt = "轻度";
  }
  else if(pm25V>50)
  {
    pm25BgColor = tft.color565(247,219,100);//良
    aqiTxt = "良";
  }

  clk.createSprite(60,26);          //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);        //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);       //设置文本数据
  clk.fillRoundRect(0,0,60,26,4,pm25BgColor);           //实心圆角矩形
  clk.setTextColor(TFT_BLACK);      //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(aqiTxt,30,14);     //绘制字符串--字符串,x,y
  clk.pushSprite(150,140);          //窗口位置--x,y
  clk.deleteSprite();               //删除窗口
  
  //----城市名称----//
  clk.createSprite(115,26);         //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);        //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);       //设置文本数据
  clk.fillRoundRect(0,0,115,26,4,pm25BgColor);           //实心圆角矩形
  clk.setTextColor(TFT_BLACK);      //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(sk["cityname"].as<String>(),57,14);     //绘制字符串--字符串,x,y
  clk.pushSprite(30,140);          //窗口位置--x,y
  clk.deleteSprite();               //删除窗口

  //----天气图标----//
  wrat.printfweather(60,10,atoi((sk["weathercode"].as<String>()).substring(1,3).c_str()));

  //----左上角滚动字幕----//
  
  scrollText[0] = "实时天气 "+sk["weather"].as<String>();
  scrollText[1] = "空气质量 "+aqiTxt;
  scrollText[2] = "风向 "+sk["WD"].as<String>()+sk["WS"].as<String>();
  
  //解析第二段JSON
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  //Serial.println(sk["ws"].as<String>());
  scrollText[3] = "今日"+dz["weather"].as<String>();
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  
  scrollText[4] = "最低温度"+fc["fd"].as<String>()+"℃";
  scrollText[5] = "最高温度"+fc["fc"].as<String>()+"℃";
  
  scrollBanner();  
}


void scrollBanner()
{
  clk.createSprite(180,24);         //创建窗口--大小x,y
  clk.fillSprite(TFT_WHITE);        //窗口填充--颜色
  clk.setTextDatum(CC_DATUM);       //设置文本数据
  clk.setTextColor(TFT_BLACK);      //设置文本颜色--文本颜色,背景颜色(可删除)
  clk.drawString(scrollText[currentIndex],90,13);     //绘制字符串--字符串,x,y
  clk.pushSprite(30,171);          //窗口位置--x,y
  clk.deleteSprite();               //删除窗口

  if(currentIndex>=5)
    currentIndex = 0;  //回第一个
  else
    currentIndex += 1;  //准备切换到下一个        
}

/* app2 */
void Display::app2(int xms)
{
  if(APPflag == true)
  {
    tft.fillScreen(TFT_WHITE);        //清屏
  }
  
  if(millis() - WeaterTime1 > UpdateWeater_Time*60000 || APPflag == true)  //*分钟更新一次天气
  { 
    WeaterTime1 = millis();

    int CityCODE = 0;
    for(int cnum=5;cnum>0;cnum--)
    {          
      CityCODE = CityCODE*100;
      CityCODE += EEPROM.read(CC_Addr+cnum-1); 
      delay(5);
    }
    
    if(CityCODE>=101000000 && CityCODE<=102000000)
      cityCode = CityCODE;
    else
      getCityCode();  //获取城市代码
  
    Serial.printf("cityCode");
    Serial.println(CityCODE);
    Serial.println(cityCode);
    
    getCityWeater();
  }
  if(millis() - WeaterTime2 > xms)
  {
    WeaterTime2 = millis();
    scrollBanner();  
  }
  
  APPflag = false;
}
