/***
 * @Author: mojionghao
 * @Date: 2025-06-18 21:13:20
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-19 17:11:06
 * @FilePath: \s3_EPD_UIdesign\include\display_main.h
 * @Description:
 */
#ifndef  _DISPLAY_MAIN_H_
#define  _DISPLAY_MAIN_H_
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
using std::string;

// 待办事项数组，最多存储4条中文待办事项
const int TODO_MAX = 4;
struct display_main_t {
	struct tm new_timeinfo;
	float humi;
	float temp;
	struct weatherDailyInfo today;
	string todos[TODO_MAX];
	int selected_todo;

	// 构造函数
	display_main_t() : selected_todo(-1) {}
};

void display_main(const display_main_t *display_main_data, UIStatus *uis);
void display_main_todo(const display_main_t *display_main_data);
void show_weathericons(int weather_code);
#endif // _DISPLAY_MAIN_H_