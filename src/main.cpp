#include <Arduino.h>
#include <display.h>
#include <WiFi.h>
#include <stdio.h>
#include <UrlEncode.h>
#include "display_main.h"
#include "display_weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "main.h"

#define GPIO0_PIN_WIFIRESET 0
#define GPIO45_PIN_USER 45
#define USER_KEY "0781c49e69024849b7cb76ef017ca453"

// 调试宏定义
#define DEBUG_PRINT(x) Serial.println("[DEBUG] " + String(x))
#define DEBUG_PRINTF(fmt, ...) Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define ERROR_PRINT(x) Serial.println("[ERROR] " + String(x))


const String city = "东莞";

WifiUser *wifiuser = nullptr;
Button *gpio0Button = nullptr;
Button *gpio45Button = nullptr;
Weather *weather = nullptr;
SemaphoreHandle_t epdMutex = nullptr;
UIStatus uis;
display_main_t display_main_data;
weatherHourlyInfo weather7hinfo[7];
tm newtime;

void updata_HourlyWeather(void *param);
void updata_time(void *param);
void display_update(void *param);

void checkGpio45Task()
{
	DEBUG_PRINT("GPIO0 按下，清除网络信息...");
	if (wifiuser != nullptr) {
		wifiuser->removeWifi();
		DEBUG_PRINT("网络信息清除完成");
	}
	else {
		ERROR_PRINT("wifiuser为空指针");
	}
}

void checkGpio0Task()
{
	DEBUG_PRINTF("按键触发，当前菜单: %d", uis.currentMenu);
	if (uis.currentMenu == MENU_HOME) {
		uis.currentMenu = MENU_7HWEATHER;
		uis.refreshType = REFRESH_FULL;
		DEBUG_PRINT("切换到天气界面");
	}
	else if (uis.currentMenu == MENU_7HWEATHER) {
		uis.currentMenu = MENU_HOME;
		uis.refreshType = REFRESH_FULL;
		DEBUG_PRINT("切换回主页");
	}
	uis.updateFlag = true;
	DEBUG_PRINT("更新标志已设置");
}

void initMutex()
{
	DEBUG_PRINT("初始化互斥锁...");
	epdMutex = xSemaphoreCreateMutex();
	if (epdMutex == nullptr) {
		ERROR_PRINT("epdMutex create failed");
	}
	else {
		DEBUG_PRINT("epdMutex create success");
	}
}

void init_updata()
{
	DEBUG_PRINT("开始初始化数据...");

	// 检查关键变量状态
	DEBUG_PRINTF("weather指针: %p", weather);
	DEBUG_PRINTF("newtime结构体大小: %d", sizeof(newtime));
	DEBUG_PRINTF("display_main_data地址: %p", &display_main_data);

	display_main_data.new_timeinfo = newtime;
	display_main_data.humi = 66;
	display_main_data.temp = 36;
	DEBUG_PRINT("基础数据初始化完成");

	// 安全的天气数据获取
	if (weather != nullptr) {
		DEBUG_PRINT("开始获取天气数据...");

		try {
			// 先检查weather对象状态
			DEBUG_PRINT("调用getToday()前...");
			const weatherDailyInfo &today_ref = weather->getToday();
			DEBUG_PRINT("getToday()调用成功");

			// 检查返回的数据是否有效
			DEBUG_PRINTF("today_ref地址: %p", &today_ref);

			display_main_data.today = today_ref;
			DEBUG_PRINT("today数据赋值成功");

		}
		catch (const std::exception &e) {
			ERROR_PRINT("获取today数据异常: " + String(e.what()));
		}
		catch (...) {
			ERROR_PRINT("获取today数据发生未知异常");
		}

		try {
			DEBUG_PRINT("开始获取hourly数据...");
			const weatherHourlyInfo *src = weather->getHourly();
			DEBUG_PRINTF("getHourly()返回指针: %p", src);

			if (src != nullptr) {
				DEBUG_PRINTF("准备拷贝%d字节数据", sizeof(weather7hinfo));
				memcpy(weather7hinfo, src, sizeof(weather7hinfo));
				DEBUG_PRINT("hourly数据拷贝成功");
			}
			else {
				ERROR_PRINT("getHourly()返回空指针");
				memset(weather7hinfo, 0, sizeof(weather7hinfo));
			}
		}
		catch (...) {
			ERROR_PRINT("获取hourly数据异常");
			memset(weather7hinfo, 0, sizeof(weather7hinfo));
		}
	}
	else {
		ERROR_PRINT("weather对象为空指针");
		memset(&display_main_data.today, 0, sizeof(display_main_data.today));
		memset(weather7hinfo, 0, sizeof(weather7hinfo));
	}

	// 初始化todos
	display_main_data.todos[0] = "洗衣服";
	display_main_data.todos[1] = "写代码";
	display_main_data.todos[2] = "跑步30分钟";
	display_main_data.todos[3] = "";

	DEBUG_PRINT("init_updata完成");
}

void setup()
{
	Serial.begin(115200);
	delay(1000); // 给串口时间初始化
	DEBUG_PRINT("=== EPD UI Design 启动 ===");

	// 内存状态检查
	DEBUG_PRINTF("可用堆内存: %d bytes", esp_get_free_heap_size());
	DEBUG_PRINTF("最小可用堆内存: %d bytes", esp_get_minimum_free_heap_size());

	if (!SPIFFS.begin(true)) {
		ERROR_PRINT("SPIFFS挂载失败");
		return;
	}
	DEBUG_PRINT("SPIFFS挂载成功");

	DEBUG_PRINT("初始化EPD...");
	epd_Init();
	epd_layout_hello();
	DEBUG_PRINT("EPD初始化完成");

	DEBUG_PRINT("创建按钮对象...");
	gpio0Button = new Button(GPIO0_PIN_WIFIRESET, checkGpio0Task);
	gpio45Button = new Button(GPIO45_PIN_USER, checkGpio45Task);
	DEBUG_PRINTF("按钮对象创建完成，gpio0Button: %p, gpio45Button: %p", gpio0Button, gpio45Button);

	DEBUG_PRINT("创建WiFi用户对象...");
	wifiuser = new WifiUser("EZ_EPD", 10);
	DEBUG_PRINTF("wifiuser创建完成: %p", wifiuser);

	DEBUG_PRINT("创建天气对象...");
	weather = new Weather(USER_KEY, urlEncode(city));
	DEBUG_PRINTF("weather创建完成: %p", weather);

	initMutex();

	DEBUG_PRINT("等待WiFi连接...");
	int wifi_retry = 0;
	while (!(wifiuser->isConnected())) {
		if (wifi_retry % 100 == 0) {
			DEBUG_PRINTF("WiFi连接尝试 #%d", wifi_retry / 100);
		}
		wifi_retry++;
		vTaskDelay(pdMS_TO_TICKS(10));

		if (wifi_retry > 3000) { // 30秒超时
			ERROR_PRINT("WiFi连接超时");
			break;
		}
	}

	if (wifiuser->isConnected()) {
		DEBUG_PRINT("WiFi连接成功");
		initNTP();
		DEBUG_PRINT("NTP初始化完成");

		getLocalTime(&newtime);
		DEBUG_PRINTF("本地时间获取完成: %d-%d-%d %d:%d:%d",
		             newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
		             newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

		DEBUG_PRINT("开始获取天气数据...");
		if (weather->GetLocationCode()) {
			DEBUG_PRINT("位置代码获取成功");

			bool hourlySuccess = weather->GetHourlyWeather();
			bool dailySuccess = weather->Get3dWeather();

			DEBUG_PRINTF("天气数据获取结果: hourly=%d, daily=%d", hourlySuccess, dailySuccess);

			if (hourlySuccess && dailySuccess) {
				DEBUG_PRINT("所有天气数据获取成功");
				vTaskDelay(pdMS_TO_TICKS(200));
			}
			else {
				ERROR_PRINT("天气数据获取不完整");
			}
		}
		else {
			ERROR_PRINT("位置代码获取失败");
		}
	}

	// 初始化UI状态
	uis.currentMenu = MENU_HOME;
	uis.updateFlag = false;
	uis.refreshType = REFRESH_FULL;
	DEBUG_PRINTF("UI状态初始化: menu=%d, flag=%d, refresh=%d",
	             uis.currentMenu, uis.updateFlag, uis.refreshType);

	vTaskDelay(pdMS_TO_TICKS(200));

	DEBUG_PRINT("调用init_updata()...");
	init_updata();
	DEBUG_PRINT("init_updata()完成");

	// 内存状态检查
	DEBUG_PRINTF("初始化后可用堆内存: %d bytes", esp_get_free_heap_size());

	DEBUG_PRINT("创建任务...");
	BaseType_t result1 = xTaskCreatePinnedToCore(updata_HourlyWeather, "updata_HourlyWeather", 6144, nullptr, 1, nullptr, 1);
	BaseType_t result2 = xTaskCreatePinnedToCore(updata_time, "updata_time", 2048, nullptr, 1, nullptr, 1);
	BaseType_t result3 = xTaskCreatePinnedToCore(display_update, "display_update", 16384, nullptr, 2, nullptr, 1);

	DEBUG_PRINTF("任务创建结果: weather=%d, time=%d, display=%d", result1, result2, result3);

	if (result1 != pdPASS || result2 != pdPASS || result3 != pdPASS) {
		ERROR_PRINT("任务创建失败");
	}
	else {
		DEBUG_PRINT("所有任务创建成功");
	}

	DEBUG_PRINT("=== setup()完成 ===");
}

void updata_HourlyWeather(void *param)
{
	DEBUG_PRINT("updata_HourlyWeather任务启动");
	int cycle_count = 0;

	while (1) {
		DEBUG_PRINTF("天气更新任务第%d次循环", ++cycle_count);

		if (weather != nullptr && weather->GetHourlyWeather()) {
			DEBUG_PRINT("天气数据获取成功");
			const weatherHourlyInfo *src = weather->getHourly();
			if (src != nullptr) {
				memcpy(weather7hinfo, src, sizeof(weather7hinfo));
				DEBUG_PRINT("天气数据拷贝完成");
			}
			else {
				ERROR_PRINT("getHourly()返回空指针");
			}
		}
		else {
			ERROR_PRINT("天气数据获取失败或weather为空");
		}

		if (uis.currentMenu == MENU_7HWEATHER) {
			DEBUG_PRINT("当前为天气界面，设置更新标志");
			uis.updateFlag = true;
		}

		DEBUG_PRINT("天气任务进入休眠(30分钟)");
		vTaskDelay(pdMS_TO_TICKS(1800000)); // 30分钟
	}
}

void updata_time(void *param)
{
	Serial.println("[DEBUG] updata_time任务启动");
	int cycle_count = 0;

	while (1) {
		cycle_count++;
		Serial.printf("[DEBUG] 时间任务循环开始 #%d\n", cycle_count);

		try {
			Serial.println("[DEBUG] 准备调用getLocalTime");
			getLocalTime(&newtime);
			Serial.println("[DEBUG] getLocalTime调用成功");

			display_main_data.new_timeinfo = newtime;
			Serial.println("[DEBUG] 时间数据赋值完成");

			if (uis.currentMenu == MENU_HOME) {
				Serial.println("[DEBUG] 当前为主页，准备设置更新标志");
				uis.refreshType = REFRESH_PARTIAL;
				uis.updateFlag = true;
				Serial.println("[DEBUG] 更新标志设置完成");
			}

			Serial.printf("[DEBUG] 时间任务即将休眠，循环#%d\n", cycle_count);
		}
		catch (...) {
			Serial.printf("[ERROR] 时间任务异常，循环#%d\n", cycle_count);
		}

		vTaskDelay(pdMS_TO_TICKS(60000));
		Serial.printf("[DEBUG] 时间任务休眠结束，循环#%d\n", cycle_count);
	}
}

void display_update(void *param)
{
	DEBUG_PRINT("display_update任务启动");
	int update_count = 0;

	while (1) {
		if (uis.updateFlag == true) {
			DEBUG_PRINTF("显示更新#%d: menu=%d, refresh=%d",
			             ++update_count, uis.currentMenu, uis.refreshType);
			DEBUG_PRINT("跳过显示函数，仅测试任务循环");
			if (xSemaphoreTake(epdMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
				DEBUG_PRINT("获取EPD互斥锁成功");

				try {
					switch (uis.currentMenu) {
						case MENU_HOME:
							DEBUG_PRINT("调用display_main()...");
							display_main(&display_main_data, &uis);
							DEBUG_PRINT("display_main()完成");
							break;
						case MENU_7HWEATHER:
							DEBUG_PRINT("调用display_weather()...");
							display_weather(weather7hinfo, &uis);
							DEBUG_PRINT("display_weather()完成");
							break;
						default:
							DEBUG_PRINTF("未知菜单类型: %d", uis.currentMenu);
							break;
					}
				}
				catch (...) {
					ERROR_PRINT("显示函数调用异常");
				}

				xSemaphoreGive(epdMutex);
				DEBUG_PRINT("释放EPD互斥锁");
				uis.updateFlag = false;
				DEBUG_PRINT("更新标志已清除");

			}
			else {
				ERROR_PRINT("获取EPD互斥锁超时");
			}

		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void loop()
{
	static unsigned long last_memory_check = 0;

	// 每10秒检查一次内存状态
	if (millis() - last_memory_check > 10000) {
		DEBUG_PRINTF("主循环内存检查 - 可用: %d bytes, 最小: %d bytes",
		             esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
		last_memory_check = millis();
	}

	vTaskDelay(pdMS_TO_TICKS(100));
}