#ifndef __API_BEAN_HPP__
#define __API_BEAN_HPP__

#include <Arduino.h>
#include <string>

using namespace std;

typedef struct {
    String cityName;
    int cityId;
} LocInfo;

typedef struct {
    s16 maxTemp;
    s16 minTemp;
    s16 curTemp;
    s16 humi;
    s16 aqi;
    String condition;
    String wind;
    s16 weatherCode;
    String uvTxt;
} WeatherInfo;

#endif