/***
 * @Author: mojionghao
 * @Date: 2025-06-18 21:12:40
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-19 16:37:52
 * @FilePath: \s3_EPD_UIdesign\src\ui\display_main.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-06-18 21:12:40
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-18 21:15:45
 * @FilePath: \s3_EPD_UIdesign\src\ui\display_main.cpp
 * @Description:
 */
#include "display_main.h"

// 保证和 display.cpp 里的宏一致
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

extern U8G2_FOR_ADAFRUIT_GFX u8g2_epd;
extern GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

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
static uint8_t partial_count = 0; // 局部刷新标志
void display_main(display_main_t display_main_data, UIStatus uis)
{

	u8g2_epd.setBackgroundColor(GxEPD_WHITE);              // 设置背景色为白色
	u8g2_epd.setFontMode(1);                               // 设置字体透明模式
	u8g2_epd.setFontDirection(0);                          // 设置字体方向，从左到右
	u8g2_epd.setForegroundColor(GxEPD_BLACK);              // 设置前景色为黑色

	//如果当前界面是全局刷新或局部刷新次数超过10次，则进行全局刷新
	if (uis.refreshType == REFRESH_FULL || partial_count > 10) {
		display.setFullWindow(); // 设置全局刷新窗口
		partial_count = 0; // 重置局部刷新计数
		//如果上一个界面不是当前界面则重新绘制

	}
	else {
		display.setPartialWindow(0, 0, display.width(), display.height()); // 设置局部刷新窗口
		partial_count++; // 增加局部刷新计数
	}
	display.firstPage(); // 开始绘制第一页
	do {

		if (uis.refreshType == REFRESH_FULL) {
			display.fillScreen(GxEPD_WHITE); // 清空屏幕，背景色为白色
		}
		display.drawInvertedBitmap(AREA_TODO_X + ((AREA_TODO_W - 32) / 2), AREA_TODO_Y, todo, 32, 32, GxEPD_BLACK);
		// 绘制顶部栏
		display.drawLine(AREA_TOPBAR_X, AREA_TOPBAR_Y + AREA_TOPBAR_H, AREA_TOPBAR_X + AREA_TOPBAR_W - 1, AREA_TOPBAR_Y + AREA_TOPBAR_H, GxEPD_BLACK);
		display.drawLine(AREA_TODO_X, AREA_TODO_Y, AREA_TODO_X, AREA_TODO_Y + AREA_TODO_H - 1, GxEPD_BLACK);
		// 绘制时间
		char str[6];
		snprintf(str, sizeof(str), "%02d:%02d", display_main_data.new_timeinfo.tm_hour, display_main_data.new_timeinfo.tm_min);

		u8g2_epd.setFont(u8g2_mfxuanren_36_tr);
		int16_t str_w = u8g2_epd.getUTF8Width(str);
		int16_t ascent = u8g2_epd.getFontAscent();
		int16_t descent = u8g2_epd.getFontDescent();
		int16_t font_h = ascent - descent;
		int16_t x = 0 + (175 - str_w) / 2;
		int16_t y = 27 + (55 - font_h) / 2 + ascent;
		u8g2_epd.drawUTF8(x, y, str);

		//日期不一致时更新日期
		char date_str[20];
		strftime(date_str, 20, "%a,%d %b %Y", &display_main_data.new_timeinfo);
		text14(date_str, AREA_DATA_X + 30, AREA_DATA_Y);


		// 绘制温湿度
		char humi_str[20];
		char temp_str[20];
		snprintf(humi_str, sizeof(humi_str), "湿度:%.1f%%", display_main_data.humi);
		snprintf(temp_str, sizeof(temp_str), "温度:%.1fC", display_main_data.temp);
		text14(temp_str, AREA_DATA_X, AREA_DATA_Y + 14);
		text14(humi_str, AREA_DATA_X, AREA_DATA_Y + 14 + 14);

		// 绘制待办事项
		for (int i = 0; i < TODO_MAX; i++) {
			if (!display_main_data.todos[i].empty()) {
				if (i == display_main_data.selected_todo) {
					text14(display_main_data.todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14, GxEPD_WHITE, GxEPD_BLACK);// 选中待办事项时反色显示
				}
				else {
					text14(display_main_data.todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14);//    未选中待办事项正常显示
				}
			}
		}
	}
	while (display.nextPage());   // 绘制下一页
}

void display_main_todo(display_main_t display_main_data)
{
	display.setPartialWindow(AREA_TODO_X + 5, AREA_TODO_Y + 32, AREA_TODO_W, AREA_TODO_H - 32);

	display.firstPage();
	do {
		for (int i = 0; i < TODO_MAX; i++) {
			if (!display_main_data.todos[i].empty()) {
				if (i == display_main_data.selected_todo) {
					text14(display_main_data.todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14, GxEPD_WHITE, GxEPD_BLACK);// 选中待办事项时反色显示
				}
				else {
					text14(display_main_data.todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14);//    未选中待办事项正常显示
				}
			}
		}
	}
	while (display.nextPage());
	partial_count++; // 增加局部刷新计数
}