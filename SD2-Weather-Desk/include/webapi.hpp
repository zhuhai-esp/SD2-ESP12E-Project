#ifndef __WEB_API_HPP__
#define __WEB_API_HPP__

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

WiFiClient wifiClient;

/**
 * https://zhwnl.weatherat.com/
 *
{"province":"广东","nation":"中国","city":"深圳","district":"宝安","lon":"113.88311","key":"true","lat":"22.55371"}
*/
inline String get_location_info() {
  HTTPClient http;
  http.begin(wifiClient, "http://data.weatherat.com/live/ip");
  if (http.GET() > 0) {
    auto ret = http.getString();
    http.end();
    return ret;
  }
  http.end();
  return "";
}

/**
{"area":"CN","cityId":3284}
*/
inline JsonDocument get_cur_city_info() {
  String ip_info = get_location_info();
  JsonDocument ret;
  if (ip_info.isEmpty()) {
    return ret;
  }
  HTTPClient http;
  http.begin(wifiClient, "http://data.weatherat.com/location/getCityId");
  http.addHeader("Content-Type", "application/json");
  if (http.POST(ip_info) > 0) {
    deserializeJson(ret, http.getString());
    http.end();
    JsonDocument ip;
    deserializeJson(ip, ip_info);
    ret["ip"] = ip;
    return ret;
  }
  http.end();
  return ret;
}

/**
{"condition":"晴","windPower":1,"temperature":27,"aqi":47,"humidity":65,"airCondition":"优","wind":"东风"}
*/
inline JsonDocument get_city_weather_info(int cityId) {
  HTTPClient http;
  String url = "http://api.weatherat.com/frontend/realtime/brief/";
  url.concat(cityId);
  http.begin(wifiClient, url);
  JsonDocument ret;
  if (http.GET() > 0) {
    deserializeJson(ret, http.getString());
    http.end();
    auto jsObj = ret["data"];
    return jsObj;
  }
  http.end();
  return ret;
}

/**
{"token":"08a9c1ff7802a29e1fc1cad1280d1c74","cityId":2371}
{"code":200,"msg":"ok","data":{"date":"2023-10-17
19:26:00","uvindex":0,"MINTEM":23,"weatherCode":14,"isDayTime":false,"HUMI":57,"cnEra":"癸卯年","aqiDesc":"空气质量优，是一个出行，会友，运动的好天气","lunarMonth":9,"bgColor":["#587BB0","#627DA7"],"weekDay":3,"MAXTEM":27,"leap":false,"windSpeed":1.3,"WIND":"东风","rain":"0.0","visibility":30000,"ULTRAVIOLETRAYS":"最弱","updateTime":"10-17
19:00","pressure":1007,"comfort":2,"quality":1,"CONDITIONSTEXT":"阴","td":16,"levelIndex":"优","TEMP":26,"thwindex":27,"CLOUDCOVER":83,"aqi":38,"WINP":1,"lunarDay":3,"lunarYear":2023}}
*/
inline JsonDocument get_now_huge_info(int cityId) {
  HTTPClient http;
  http.begin(wifiClient, "http://data.weatherat.com/live/getNow");
  http.addHeader("Content-Type", "application/json");
  JsonDocument doc;
  doc["token"] = "08a9c1ff7802a29e1fc1cad1280d1c74";
  doc["cityId"] = cityId;
  String txt;
  serializeJson(doc, txt);
  JsonDocument ret;
  if (http.POST(txt) > 0) {
    deserializeJson(ret, http.getString());
    http.end();
    auto jsObj = ret["data"];
    return jsObj;
  }
  http.end();
  return ret;
}

#endif