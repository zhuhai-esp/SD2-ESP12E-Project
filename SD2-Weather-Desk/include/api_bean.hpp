#ifndef __API_BEAN_HPP__
#define __API_BEAN_HPP__

#include <Arduino.h>
#include <string>

using namespace std;

typedef struct {
    string cityName;
    int cityId;
} LocInfo;

typedef struct {
    s16 maxTemp;
    s16 minTemp;
    s16 curTemp;
    s16 humi;
    s16 aqi;
    string condition;
    string wind;
    s16 weatherCode;
    string uvTxt;
} WeatherInfo;

#endif