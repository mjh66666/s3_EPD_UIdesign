#include "display_weather.h"

// 保证和 display.cpp 里的宏一致
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

extern U8G2_FOR_ADAFRUIT_GFX u8g2_epd;
extern GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

// 区域定义
#define AREA_TOPBAR_X      0
#define AREA_TOPBAR_Y      0
#define AREA_TOPBAR_W      296
#define AREA_TOPBAR_H      18

// 添加静态变量
static uint8_t partial_count = 0; // 局部刷新计数

/**
 * @brief 主天气显示函数
 */
void display_weather(const display_topbar_t *topbar,const weatherHourlyInfo *hourly, UIStatus *uis)
{
	u8g2_epd.setBackgroundColor(GxEPD_WHITE);
	u8g2_epd.setFontMode(1);
	u8g2_epd.setFontDirection(0);
	u8g2_epd.setForegroundColor(GxEPD_BLACK);

	// 如果当前界面是全局刷新或局部刷新次数超过10次，则进行全局刷新
	if (uis->refreshType == REFRESH_FULL || partial_count > 10) {
		display.setFullWindow();
		partial_count = 0;
	}
	else {
		display.setPartialWindow(0, 0, display.width(), display.height());
		partial_count++;
	}

	display.firstPage();
	do {
		if (uis->refreshType == REFRESH_FULL) {
			display.fillScreen(GxEPD_WHITE);
		}

		// 绘制顶部分隔线
		display.drawLine(AREA_TOPBAR_X, AREA_TOPBAR_Y + AREA_TOPBAR_H,
		                 AREA_TOPBAR_X + AREA_TOPBAR_W - 1,
		                 AREA_TOPBAR_Y + AREA_TOPBAR_H, GxEPD_BLACK);
		display_topbar(topbar);
		// 绘制7小时天气
		draw7HourWeather(hourly, AREA_TOPBAR_H);

	}
	while (display.nextPage());
}

/**
 * @brief 从时间字符串中提取小时，返回String类型
 */
String extractHour(const String &fxTime)
{
	int tIndex = fxTime.indexOf('T');
	if (tIndex != -1) {
		return fxTime.substring(tIndex + 1, tIndex + 3);  // 返回String，不是std::string
	}
	return "00";
}

/**
 * @brief 绘制7小时天气信息
 */
void draw7HourWeather(const weatherHourlyInfo *hourly, int startY)
{
	const int ITEM_WIDTH = 42;
	const int MARGIN_LEFT = 1;
	const int ICON_SIZE = 32;

	// 布局高度分配
	const int TIME_Y = startY;
	const int ICON_Y = startY + 18;
	const int TEMP_Y = startY + 50;
	const int TEXT_Y = startY + 65;

	for (int i = 0; i < 7; i++) {
		int x = MARGIN_LEFT + i * ITEM_WIDTH;
		int centerX = x + ITEM_WIDTH / 2;

		// 显示时间
		String timeStr = extractHour(hourly[i].fxTime);
		printf("Hour %d: %s\n", i, timeStr.c_str()); // 调试输出
		int timeWidth = u8g2_epd.getUTF8Width(timeStr.c_str());
		if (i == 0) {
			text14(timeStr.c_str(), 14, TIME_Y);
		}
		else {
			text14(timeStr.c_str(), centerX - timeWidth / 2, TIME_Y);
		}


		// 显示天气图标
		int iconX = centerX - ICON_SIZE / 2;
		display.drawInvertedBitmap(iconX, ICON_Y,
		                           getWeatherIcon(hourly[i].icon),
		                           ICON_SIZE, ICON_SIZE, GxEPD_BLACK);

		// 显示温度
		char tempStr[6];
		snprintf(tempStr, sizeof(tempStr), "%d°", hourly[i].temp);
		int tempWidth = u8g2_epd.getUTF8Width(tempStr);
		text14(tempStr, centerX - tempWidth / 2, TEMP_Y);

		// 显示降水概率
		char popStr[6];
		snprintf(popStr, sizeof(popStr), "%d%%", hourly[i].pop);
		int popWidth = u8g2_epd.getUTF8Width(popStr);
		text14(popStr, centerX - popWidth / 2, TEXT_Y);

		// 🔗 绘制分隔线
		if (i < 6) {
			int lineX = x + ITEM_WIDTH;
			display.drawLine(lineX, startY + 5, lineX, startY + 95, GxEPD_BLACK);
		}
	}

	drawTemperatureTrend(hourly, startY);
}

/**
 * @brief 绘制温度趋势曲线
 */
void drawTemperatureTrend(const weatherHourlyInfo *hourly, int startY)
{
	const int ITEM_WIDTH = 42;
	const int MARGIN_LEFT = 1;

	int minTemp = 100, maxTemp = -100;
	int temps[7];


	for (int i = 0; i < 7; i++) {
		temps[i] = hourly[i].temp;
		if (temps[i] < minTemp) {
			minTemp = temps[i];
		}
		if (temps[i] > maxTemp) {
			maxTemp = temps[i];
		}
	}

	if (maxTemp == minTemp) {
		maxTemp = minTemp + 1;
	}

	int highTempY = startY + 79;    // 高温在上方（Y值小）
	int lowTempY = 125;     // 低温在下方（Y值大）

	// 绘制趋势线段
	for (int i = 0; i < 6; i++) {
		int x1 = MARGIN_LEFT + i * ITEM_WIDTH + ITEM_WIDTH / 2;
		int x2 = MARGIN_LEFT + (i + 1) * ITEM_WIDTH + ITEM_WIDTH / 2;


		int y1 = map(temps[i], minTemp, maxTemp, lowTempY, highTempY);
		int y2 = map(temps[i + 1], minTemp, maxTemp, lowTempY, highTempY);

		display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
		display.fillCircle(x1, y1, 1, GxEPD_BLACK);
		if (i == 5) {
			display.fillCircle(x2, y2, 1, GxEPD_BLACK);
		}
	}
}

