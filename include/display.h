/***
 * @Author: mojionghao
 * @Date: 2025-02-28 20:23:10
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-14 16:34:00
 * @FilePath: \s3_EPD_UIdesign\src\display.h
 * @Description:
 */
#ifndef _IMAGE_H
#define _IMAGE_H

#include <pgmspace.h>
#include <GxEPD2_BW.h> // 包含GxEPD2_BW库，用于电子墨水屏的驱动

#include <Arduino.h>
#include <Fonts/FreeMonoBold9pt7b.h> // 字体库
#include <U8g2_for_Adafruit_GFX.h>   // Adafruit GFX库的U8g2接口
#include "esp_mac.h"
#include "image/image.h"
#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w

void epd_Init();

void epd_layout_hello();

void epd_layout_TIME(int hour, int minute);

void epd_layout_static();

void epd_layout_date(char *date);

void epd_full_reflash();

void epd_refresh_date(char *date);

void epd_refresh_time(int hour, int minute, char *date);

#endif