/***
 * @Author: mojionghao
 * @Date: 2025-02-28 11:17:52
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-14 20:01:44
 * @FilePath: \s3_EPD_UIdesign\src\display.cpp
 * @Description:
 */
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <GxEPD2_BW.h>// 包含GxEPD2_BW库，用于电子墨水屏的驱动
#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
#include <Arduino.h>
#include <Fonts/FreeMonoBold9pt7b.h> // 字体库
#include <U8g2_for_Adafruit_GFX.h> // Adafruit GFX库的U8g2接口
#include "esp_mac.h"
#include "image/image.h"
#include "front/u8g2_mfxuanren_36_tr.h"
#include "text_draw.h"
/*
    mfxuanren_36为日历字体
*/

// u8g2Fonts引擎和GxEPD2原生绘图之间的冲突问题，目前只能先写完文字刷新后再重新画图然后再刷新。存在二次刷新问题。
#define USE_HSPI_FOR_EPD // 定义使用HSPI接口用于电子墨水屏

#define ENABLE_GxEPD2_GFX 0 // 禁用GxEPD2的GFX功能
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW // 定义显示类为GxEPD2的黑白屏

#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2 // 定义使用的具体电子墨水屏型号

#define GxEPD2_BW_IS_GxEPD2_BW true // 定义用于条件编译

#define IS_GxEPD(c, x) (c##x) // 用于拼接宏定义的辅助宏
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x) // 用于判断是否是GxEPD2的黑白屏
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x) // 用于判断是否是GxEPD2的三色屏 
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x) // 用于判断是否是GxEPD2的七色屏 
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x) // 用于判断是否是GxEPD2的1248屏

#if defined(ESP32)
	#define MAX_DISPLAY_BUFFER_SIZE 65536ul // 定义最大显示缓冲区大小
	#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)

		#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
		// 根据显示缓冲区大小调整最大高度
	#endif
	// GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/15, /*DC=*/16, /*RST=*/17, /*BUSY=*/3));
	GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/15, /*DC=*/16, /*RST=*/17, /*BUSY=*/3));
	// 初始化显示类，指定特定引脚
#endif

U8G2_FOR_ADAFRUIT_GFX u8g2_epd; // 创建 U8g2 对象

SPIClass hspi(HSPI); // 初始化HSPI接口

// 区域1：顶部栏（如时间日期区）
#define AREA_TOPBAR_X      0
#define AREA_TOPBAR_Y      0
#define AREA_TOPBAR_W      296
#define AREA_TOPBAR_H      18

#define AREA_TIME_X        0
#define AREA_TIME_Y        24
#define AREA_TIME_W        175
#define AREA_TIME_H        48

#define AREA_DATA_X        0
#define AREA_DATA_Y        71
#define AREA_DATA_W        175
#define AREA_DATA_H        57  // 128-71

// 区域3：右侧功能区（如TO-DO清单区）
#define AREA_TODO_X       175
#define AREA_TODO_Y       18
#define AREA_TODO_W       121  // 296-175
#define AREA_TODO_H       112  // 128-16

/***
 * @description:
 * @return null
 */
void epd_Init()
{

	// GXEPD2初始化
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
	//hspi.begin(13, 12, 14, 15); // 引脚修改
	hspi.begin(13, 12, 14, 15); // 引脚修改
	display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
	display.setRotation(1); // 即为横向
	display.init(115200);
	display.setRotation(1);
	display.setFont(&FreeMonoBold9pt7b);
	display.setTextColor(GxEPD_BLACK);
	// u8g2库初始化
	u8g2_epd.begin(display);                               //  初始化u8g2接口
	display.fillScreen(GxEPD_WHITE);                       // 清空屏幕，背景色为白色
	u8g2_epd.setBackgroundColor(GxEPD_WHITE);              // 设置背景色为白色
	u8g2_epd.setFontMode(1);                               // 设置字体透明模式
	u8g2_epd.setFontDirection(0);                          // 设置字体方向，从左到右
	u8g2_epd.setForegroundColor(GxEPD_BLACK);              // 设置前景色为黑色
	u8g2_epd.setFont(u8g2_mfxuanren_36_tr);
}

void epd_layout_hello()
{
	int16_t tbx, tby;
	uint16_t tbw, tbh;
	display.getTextBounds("hello", 0, 0, &tbx, &tby, &tbw, &tbh);
	// center bounding box by transposition of origin:
	uint16_t x = ((display.width() - tbw) / 2) - tbx;
	uint16_t y = ((display.height() - tbh) / 2) - tby;
	display.setFullWindow();
	display.firstPage();
	do {
		display.fillScreen(GxEPD_WHITE);
		display.setCursor(x, y);
		display.print("hello");
	}
	while (display.nextPage());
}



void epd_layout_TIME(int hour, int minute)
{
	char str[6];
	snprintf(str, sizeof(str), "%02d:%02d", hour, minute);

	u8g2_epd.setFont(u8g2_mfxuanren_36_tr);
	int16_t str_w = u8g2_epd.getUTF8Width(str);
	int16_t ascent = u8g2_epd.getFontAscent();
	int16_t descent = u8g2_epd.getFontDescent();
	int16_t font_h = ascent - descent;
	int16_t x = 0 + (175 - str_w) / 2;
	int16_t y = 27 + (55 - font_h) / 2 + ascent;
	u8g2_epd.drawUTF8(x, y, str);
}

void epd_layout_static()
{
	//todo logo
	display.drawInvertedBitmap(AREA_TODO_X + ((AREA_TODO_W - 32) / 2), AREA_TODO_Y, todo, 32, 32, GxEPD_BLACK);

	//分隔线
	display.drawLine(AREA_TOPBAR_X, AREA_TOPBAR_Y + AREA_TOPBAR_H, AREA_TOPBAR_X + AREA_TOPBAR_W - 1, AREA_TOPBAR_Y + AREA_TOPBAR_H, GxEPD_BLACK);
	display.drawLine(AREA_TODO_X, AREA_TODO_Y, AREA_TODO_X, AREA_TODO_Y + AREA_TODO_H - 1, GxEPD_BLACK);
}

//写入日期
void epd_layout_date(char *date)
{
	text14(date, AREA_DATA_X + 30, AREA_DATA_Y);
}

//写入温湿度
void epd_layout_humi_temp(float humi, float temp)
{
	char humi_str[20];
	char temp_str[20];
	snprintf(humi_str, sizeof(humi_str), "温度:%.1fC", humi);
	snprintf(temp_str, sizeof(temp_str), "温度:%.1fC", temp);
	text14(temp_str, AREA_DATA_X, AREA_DATA_Y + 14);
	text14(humi_str, AREA_DATA_X, AREA_DATA_Y + 14 + 14);
}



void epd_full_reflash()
{
	display.firstPage();
	do {
		text14("微信消息：卧槽[动画表情]", 0, 0);
		epd_layout_static();
	}
	while (display.nextPage());
	display.firstPage();
}

void epd_refresh_time(int hour, int minute, char *date)
{
	// 设置局部刷新窗口为时间区域
	display.setPartialWindow(AREA_TIME_X, AREA_TIME_Y, AREA_TIME_W, AREA_TIME_H + 12);
	display.firstPage();
	do {
		// 清除时间区域背景
		display.fillRect(AREA_TIME_X, AREA_TIME_Y, AREA_TIME_W, AREA_TIME_H + 12, GxEPD_WHITE);
		// 居中显示时间
		epd_layout_TIME(hour, minute);
		text14(date, AREA_DATA_X + 30, AREA_DATA_Y);
	}
	while (display.nextPage());
}
