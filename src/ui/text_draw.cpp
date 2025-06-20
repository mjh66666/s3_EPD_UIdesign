/*
 * @Author: mojionghao
 * @Date: 2025-05-14 17:28:47
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-14 17:29:36
 * @FilePath: \s3_EPD_UIdesign\src\text_draw.c
 * @Description
 */
#include "text_draw.h"
#include <GxEPD2_BW.h>
#include "bitmaps/Bitmaps128x296.h"
#include <U8g2_for_Adafruit_GFX.h>
#include "front/u8g2_mfxuanren_36_tr.h"
#include <Arduino.h>

// 保证和 display.cpp 里的宏一致
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

extern U8G2_FOR_ADAFRUIT_GFX u8g2_epd;
extern GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;


void text14(const char *str, int16_t x, int16_t y, uint16_t fg_color, uint16_t bg_color)
{
	bool r2l = false;
	int8_t baseline = 14;
	u8g2_epd.setFontMode(1);
	u8g2_epd.setFont(u8g2_font_wqy14_t_gb2312);

	int16_t w = u8g2_epd.getUTF8Width(str);
	int16_t h = baseline;

	if (bg_color == GxEPD_BLACK) {
		display.fillRect(x, y, w, h, bg_color); // 填充背景色
	}
	u8g2_epd.setBackgroundColor(bg_color);
	u8g2_epd.setForegroundColor(fg_color);
	if (!r2l) {
		u8g2_epd.drawUTF8(x, y + baseline, str);
	}
	else {
		int16_t w = u8g2_epd.getUTF8Width(str);
		int16_t new_x = display.width() - x - w;
		u8g2_epd.drawUTF8(new_x, y + baseline, str);
	}
}

void text36(const char *str, int16_t x, int16_t y, uint16_t fg_color, uint16_t bg_color)
{
	bool r2l = false;
	const int8_t baseline = 36;
	u8g2_epd.setFontMode(1);
	u8g2_epd.setFont(u8g2_mfxuanren_36_tr);
	u8g2_epd.setBackgroundColor(bg_color);
	u8g2_epd.setForegroundColor(fg_color);
	if (!r2l) {
		u8g2_epd.drawUTF8(x, y + baseline, str);
	}
	else {
		int16_t w = u8g2_epd.getUTF8Width(str);
		int16_t new_x = display.width() - x - w;
		u8g2_epd.drawUTF8(new_x, y + baseline, str);
	}
}
