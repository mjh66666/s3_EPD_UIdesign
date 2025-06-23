/***
 * @Author: mojionghao
 * @Date: 2025-02-28 11:10:42
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-05 17:45:53
 * @FilePath: \s3_EPD_UIdesign\src\main.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-02-28 11:10:42
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-21 15:58:31
 * @FilePath: \s3_EPD_UIdesign\src\main.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-02-28 11:10:42
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-21 12:17:20
 * @FilePath: \s3_EPD_UIdesign\src\main.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-02-28 11:10:42
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-20 19:08:08
 * @FilePath: \s3_EPD_UIdesign\src\main.cpp
 * @Description:
 */
#include <Arduino.h>
#include <display.h>
#include <WiFi.h>
#include <stdio.h>
#include <UrlEncode.h>
#include "display_main.h"

#define GPIO0_PIN_WIFIRESET 0 // GPIO0引脚定义

// 设置你的WiFi信息
const char *ssid     = "mate13";
const char *password = "12345678";
#define USER_KEY "0781c49e69024849b7cb76ef017ca453"
const String  city = "从化" ;

// 连接WiFi
void connectWiFi()
{
	Serial.println("Connecting to WiFi...");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi connected!");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void test_display_main()
{
	// 2. 构造测试数据
	display_main_t test_data;
	UIStatus uis;
	// 设置时间
	struct tm timeinfo;
	getLocalTime(&timeinfo);
	test_data.new_timeinfo = timeinfo;

	Weather weather(USER_KEY, urlEncode(city)); // 创建天气对象
	struct weatherDailyInfo today;

	if (weather.GetLocationCode()) {
		Serial.println("城市 ID 获取成功！");
		// 获取 3 天的天气信息
		if (weather.Get3dWeather()) {
			Serial.println("天气获取成功！");
			Serial.printf("今天日期: %s\n", weather.getToday().fxDate.c_str());
			Serial.printf("今天最高温: %d\n", weather.getToday().tempMax);
			Serial.printf("今天最低温: %d\n", weather.getToday().tempMin);
			Serial.printf("今天白天天气: %s\n", weather.getToday().textDay.c_str());
			Serial.printf("晚上天气code: %d\n", weather.getToday().iconNight);
			Serial.printf("白天天气code: %d\n", weather.getToday().iconDay);
		}
		else {
			Serial.println("天气获取失败！");
		}
	}
	else {
		Serial.println("城市 ID 获取失败！");
	}
	test_data.today = weather.getToday();
	test_data.humi = 55.5;
	test_data.temp = 23.4;

	// 设置待办事项
	test_data.todos[0] = "写代码";
	test_data.todos[1] = "喝水";
	test_data.todos[2] = "锻炼";
	test_data.todos[3] = "休息";

	// 3. 测试主界面全刷
	uis.refreshType = REFRESH_FULL;
	display_main(&test_data, &uis);
	delay(3000);

	// 4. 测试主界面局刷
	test_data.temp = 25.0;
	test_data.humi = 60.0;

	uis.refreshType = REFRESH_PARTIAL;
	display_main(&test_data, &uis);
	delay(3000);

	// 5. 测试待办事项区域局部刷新
	test_data.selected_todo = 2;
	test_data.todos[2] = "看书";
	display_main_todo(&test_data);
	delay(3000);

	// 6. 测试删除待办事项
	test_data.todos[1] = "";
	test_data.selected_todo = 0;
	display_main_todo(&test_data);
	delay(3000);
}
void setup()
{
	Serial.begin(115200);
	connectWiFi();
	initNTP();
	epd_Init();
	test_display_main();
}

void loop()
{
	delay(1000); // 主循环中可以添加其他逻辑
}

