#include "WebConfig.h"

/*** Component objects ***/
WiFiManager wm;
Display Screen;

void saveParamCallback();
/* Web配网函数 */
void WebSet::Webconfig()
{
  WiFi.begin("SDD","88888888"); 
  delay(1500);
  
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  delay(1500);
  
  wm.resetSettings(); // wipe settings
  
  int customFieldLength = 40;   // 添加一个自定义输入字段
  

  const char* set_rotation = "<br/><label for='set_rotation'>Set Rotation</label>\
                              <input type='radio' name='set_rotation' value='1'> One<br>\
                              <input type='radio' name='set_rotation' value='2'> Two<br>\
                              <input type='radio' name='set_rotation' value='3'> Three<br>\
                              <input type='radio' name='set_rotation' value='4' checked> Four<br>";
  WiFiManagerParameter  custom_rot(set_rotation); // custom html input
  WiFiManagerParameter  custom_bl("LCDBL","LCD BackLight(1-100)","10",3);
  WiFiManagerParameter  custom_weatertime("WeaterUpdateTime","Weather Update Time(Min)","10",3);
  WiFiManagerParameter  custom_City_code("CityCode","CityCode","101110101",9);
  WiFiManagerParameter  p_lineBreak_notext("<p></p>");

  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_City_code);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_bl);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_weatertime);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_rot);
  
  wm.setSaveParamsCallback(saveParamCallback);
  
  // 自定义菜单通过数组或向量
  std::vector<const char *> menu = {"wifi","restart"};
  wm.setMenu(menu);
  
  // 黑色主题
  wm.setClass("invert");
  
  wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%

  bool res;
  res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  
  while(!res);
}

String getParam(String name)
{
  //从服务器读取参数，为customhmtl输入  
  String value;
  if(wm.server->hasArg(name))
    value = wm.server->arg(name);
  return value;
}

void saveParamCallback()
{
  int City_codeODE = 0;
  //EEPROM参数存储地址位
  int BL_addr = 1;    //被写入数据的EEPROM地址编号  1亮度
  int Ro_addr = 2;    //被写入数据的EEPROM地址编号  2旋转方向
  int Uw_addr = 3;    //被写入数据的EEPROM地址编号  3天气更新时间
  int CC_addr = 10;   //被写入数据的EEPROM地址编号  10城市
  
  Serial.println("[CALLBACK] saveParamCallback fired");
  
  //将从页面中获取的数据保存
  int Updateweater_Time = getParam("WeaterUpdateTime").toInt();
  int City_code         = getParam("CityCode").toInt();
  int LCD_rotation      = getParam("set_rotation").toInt();
  int LCD_backlight     = getParam("LCDBL").toInt();

  //对获取的数据进行处理
  //城市代码
  Serial.print("城市代码： ");
  Serial.println(City_code);
  if(City_code>=101000000 && City_code<=102000000 || City_code == 0)
  {
    for(int cnum=0;cnum<5;cnum++)
    {
      EEPROM.write(CC_addr+cnum,City_code%100);//城市地址写入城市代码
      EEPROM.commit();//保存更改的数据
      City_code = City_code/100;
      delay(5);
    }
  }
  //屏幕方向
  Serial.print("屏幕方向： ");
  Serial.println(LCD_rotation);
  if(EEPROM.read(Ro_addr) != LCD_rotation)
  {
    EEPROM.write(Ro_addr, LCD_rotation);
    EEPROM.commit();
    delay(5);
  }
  //屏幕亮度  
  Serial.print("背光亮度： ");
  Serial.println(LCD_backlight);
  if(EEPROM.read(BL_addr) != LCD_backlight)
  {
    EEPROM.write(BL_addr, LCD_backlight);
    EEPROM.commit();
    delay(5);
  }
  //天气更新时间
  Serial.printf("天气更新时间：");
  Serial.println(Updateweater_Time);
  if(EEPROM.read(Uw_addr) != Updateweater_Time)
  {
    EEPROM.write(Uw_addr, Updateweater_Time);
    EEPROM.commit();
    delay(5);
  } 

  Screen.TFT_init();//显示初始化
  Screen.APP_flag();//app重进标志
}
