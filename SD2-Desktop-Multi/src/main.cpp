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
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include "WebConfig.h"
#include "display.h"
#include "number.h"
#include "weathernum.h"

#define KEY 4       //按键

/*** Component objects ***/
Display screen;
WebSet  WEB;

//---------------修改此处""内的信息--------------------
const char ssid[] = "";      //WIFI名称 修改这2个就可以了
const char pass[] = "";    //WIFI密码
//----------------------------------------------------

/***  NTP服务器  ***/
static const char ntpServerName[] = "ntp.ntsc.ac.cn";
const int timeZone = 8;     //东八区

WiFiUDP Udp;
unsigned int localPort = 8000;
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);



/***  ***/
int loadNUM = 0;  //默认进度条长度
unsigned char gethour   = 0;  //时
unsigned char getminute = 0;  //分
unsigned char getsecond = 0;  //秒
unsigned char getweek   = 0;  //星期
unsigned char getmonth  = 0;  //月
unsigned char getday    = 0;  //日
unsigned char APPMODE   = 0;  //当前运行的App
unsigned char APPNUM    = 2;  //APP个数
//unsigned int KeyNum     = 0;  //按键计数

//int getsecondChangeDis = 0; //切换屏幕时间初始化
boolean APPflagChange = false;

void gettime();

/***  ***/
void setup()
{
  /*** 串口初始化 ***/
  Serial.begin(115200);
  
  /*** EEPROM初始化 ***/
  EEPROM.begin(20);
  
  /*** 显示初始化 ***/
  screen.TFT_init();            //显示初始化
  
  /*** Wifi初始化 ***/
  WiFi.begin(ssid, pass);  
  Serial.print("正在连接WIFI ");
  Serial.println(ssid);

  /*** 连接wifi进度条 ***/
  while (WiFi.status() != WL_CONNECTED)
  {
    screen.loading(loadNUM);    //绘制进度条
    loadNUM += 1;
    delay(70);
    if(loadNUM>=200)
    {
      screen.Web_win();         //Web配网界面
      WEB.Webconfig();          //Web配网函数
      break;
    }
  }
  delay(2); 
  while(loadNUM < 200)          //让动画走完
  {
    screen.loading(loadNUM);    //绘制进度条
    loadNUM += 1;
    delay(1);
  }

  /*** UPD初始化 ***/
  Serial.print("本地IP： ");
  Serial.println(WiFi.localIP());
  Serial.println("启动UDP");
  Udp.begin(localPort);
  Serial.println("等待同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  /*** 按键初始化 ***/
  pinMode(KEY, INPUT); 
  //attachInterrupt(KEY,KEY_int,LOW); //设置中断--IO,中断函数,中断类型(CHANGE：改变沿，RISING：上升沿，FALLING：下降沿)

  screen.TFT_CLS(TFT_WHITE);//清屏
}

/*** App运行***/
void AppRun()
{
  switch (APPMODE)
  {
    case 0:
      screen.app1(gethour,getminute,getsecond,getmonth,getday,getweek); //APP1--时间
      break;
    case 1:
      screen.app2(3000);    //APP2--天气
      break;
    default:
      Serial.println("显示主界面错误");
      break;
  }

  

  
}
/*** 循环 ***/
void loop()
{
  gettime();  //获取时间
  AppRun();   //App运行 


if((millis()/1000)%15 == 0){
//   KeyNum++;

   APPflagChange = true;
   
//   Serial.println(APPflagChange);
   
   if( APPflagChange){              //KeyNum ==3
      delay(1000);
      APPMODE ++;
      screen.APP_flag();
      if(APPMODE == APPNUM ){
        APPMODE = 0;
        }
//      KeyNum = 0;
      APPflagChange = false;
    }
  }


//------------------------------------

//  if(digitalRead(KEY)==0)
//  {
//    KeyNum++;  
//    Serial.printf("按键：");
//    Serial.println(KeyNum);
//  }
//  else
//  {
//    if(KeyNum > 500)
//    {
//      APPMODE = 0;
//      screen.Web_win();         //Web配网界面
//      WEB.Webconfig();          //Web配网函数
//      screen.APP_flag();        //app重进标志
//    }
//    else if(KeyNum > 5)
//    {
//      APPMODE++;
//      screen.APP_flag();        //app重进标志
//      if(APPMODE==APPNUM)
//        APPMODE = 0;
//    }
//    KeyNum = 0;
  }


/*** 按键中断程序 **
void KEY_int()
{
  delay(5);
//  if(digitalRead(KEY)==0)
//  {
    APPMODE = APPMODE + 1;
    Serial.println("-KEY-");
//  }
  if(APPMODE==APPNUM)
    APPMODE = 0;
    
  screen.APP_flag();  //app重进标志
}
*/

/*** 获取时间 ***/
void gettime()
{
  gethour   = hour();     //时
  getminute = minute();   //分
  getsecond = second();   //秒
  getweek   = weekday();  //星期
  getmonth  = month();    //月
  getday    = day();      //日
}

/*** NTP code ***/

const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
