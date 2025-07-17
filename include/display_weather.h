/***
 * @Author: mojionghao
 * @Date: 2025-07-04 18:43:45
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-07-04 22:48:03
 * @FilePath: \s3_EPD_UIdesign\include\display_weather.h
 * @Description:
 */
#ifndef  _DISPLAY_WEATHER_H_
#define  _DISPLAY_WEATHER_H_
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "text_draw.h"
#include <GxEPD2_BW.h>
#include "front/u8g2_mfxuanren_36_tr.h"
#include "bitmaps/Bitmaps128x296.h"
#include <string>
#include <time.h>
#include "main.h"
#include <U8g2_for_Adafruit_GFX.h>
#include "image/image.h"
#include "image/weather_icons.h"
#include "display_topbar.h"
using std::string;

// 函数声明
void display_weather(const display_topbar_t *topbar,const weatherHourlyInfo *hourly, UIStatus *uis);
String extractHour(const String &fxTime);
void draw7HourWeather(const weatherHourlyInfo *hourly, int startY);
void drawTemperatureTrend(const weatherHourlyInfo *hourly, int startY);

#endif // _DISPLAY_WEATHER_H_