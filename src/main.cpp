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
#include "debug.h"  // 包含新的调试系统

#define GPIO0_PIN_WIFIRESET 0
#define GPIO45_PIN_USER 45
#define USER_KEY "0781c49e69024849b7cb76ef017ca453"
const char* ssid = "mate13";
const char* password = "12345678";

const String city = "东莞";

WifiUser *wifiuser = nullptr;
Button *gpio0Button = nullptr;
Button *gpio45Button = nullptr;
Weather *weather = nullptr;
SemaphoreHandle_t epdMutex = nullptr;
UIStatus uis;
display_main_t display_main_data;
display_topbar_t topbar;
weatherHourlyInfo weather7hinfo[7];
tm newtime;


void updata_HourlyWeather(void *param);
void updata_time(void *param);
void display_update(void *param);

void checkGpio45Task()
{
	FUNC_ENTER();
	DEBUG_PRINT("GPIO45 按下，清除网络信息...");

	if (wifiuser != nullptr) {
		wifiuser->removeWifi();
		INFO_PRINT("网络信息清除完成");
	}
	else {
		ERROR_PRINT("wifiuser为空指针");
	}

	FUNC_EXIT();
}

void checkGpio0Task()
{
	FUNC_ENTER();
	DEBUG_PRINTF("按键触发，当前菜单: %d", uis.currentMenu);

	if (uis.currentMenu == MENU_HOME) {
		uis.currentMenu = MENU_7HWEATHER;
		uis.refreshType = REFRESH_FULL;
		INFO_PRINT("切换到天气界面");
	}
	else if (uis.currentMenu == MENU_7HWEATHER) {
		uis.currentMenu = MENU_HOME;
		uis.refreshType = REFRESH_FULL;
		INFO_PRINT("切换回主页");
	}

	uis.updateFlag = true;
	VERBOSE_PRINT("更新标志已设置");
	FUNC_EXIT();
}

void initMutex()
{
	FUNC_ENTER();
	DEBUG_PRINT("初始化互斥锁...");

	epdMutex = xSemaphoreCreateMutex();
	ERROR_IF(epdMutex == nullptr, "epdMutex创建失败");
	DEBUG_IF(epdMutex != nullptr, "epdMutex创建成功: %p", epdMutex);

	FUNC_EXIT();
}

void init_updata()
{
	FUNC_ENTER();
	PERF_START(init_updata);

	DEBUG_PRINT("开始初始化数据...");

	// 检查关键变量状态
	DEBUG_PRINTF("weather指针: %p", weather);
	DEBUG_PRINTF("newtime结构体大小: %d", sizeof(newtime));
	DEBUG_PRINTF("display_main_data地址: %p", &display_main_data);

	display_main_data.new_timeinfo = newtime;
	display_main_data.humi = 66;
	display_main_data.temp = 36;
	VERBOSE_PRINT("基础数据初始化完成");
	topbar.bat_status = BATTERY_100; // 假设电池状态为100%
	topbar.message = "[微信]:";
	topbar.new_timeinfo = newtime;

	// 安全的天气数据获取
	if (weather != nullptr) {
		INFO_PRINT("开始获取天气数据...");

		try {
			VERBOSE_PRINT("调用getToday()前...");
			const weatherDailyInfo &today_ref = weather->getToday();
			VERBOSE_PRINT("getToday()调用成功");

			DEBUG_PRINTF("today_ref地址: %p", &today_ref);
			display_main_data.today = today_ref;
			INFO_PRINT("today数据赋值成功");

		}
		catch (const std::exception &e) {
			ERROR_PRINTF("获取today数据异常: %s", e.what());
		}
		catch (...) {
			ERROR_PRINT("获取today数据发生未知异常");
		}

		try {
			DEBUG_PRINT("开始获取hourly数据...");
			const weatherHourlyInfo *src = weather->getHourly();
			DEBUG_PRINTF("getHourly()返回指针: %p", src);

			if (src != nullptr) {
				VERBOSE_PRINTF("准备拷贝%d字节数据", sizeof(weather7hinfo));
				memcpy(weather7hinfo, src, sizeof(weather7hinfo));
				INFO_PRINT("hourly数据拷贝成功");

				// 显示第一个小时的天气数据（调试用） - 修复结构体成员名
				DEBUG_PRINTF("首个小时天气: 温度=%d", weather7hinfo[0].temp);
			}
			else {
				ERROR_PRINT("getHourly()返回空指针");
				for (int i = 0; i < 7; ++i) {
					weather7hinfo[i] = weatherHourlyInfo();
				}
			}
		}
		catch (...) {
			ERROR_PRINT("获取hourly数据异常");
			for (int i = 0; i < 7; ++i) {
				weather7hinfo[i] = weatherHourlyInfo();
			}
		}
	}
	else {
		ERROR_PRINT("weather对象为空指针");
		memset(&display_main_data.today, 0, sizeof(display_main_data.today));
	}

	// 初始化todos
	display_main_data.todos[0] = "洗衣服";
	display_main_data.todos[1] = "写代码";
	display_main_data.todos[2] = "跑步30分钟";
	display_main_data.todos[3] = "";
	VERBOSE_PRINTF("TODO列表初始化完成，共%d项", TODO_MAX);

	PERF_END(init_updata);
	FUNC_EXIT();
}

void setup()
{
	Serial.begin(115200);
	delay(1000); // 给串口时间初始化
	WiFi.begin(ssid, password);
	printlnA("=== EPD UI Design 启动 ===");
	MEMORY_CHECK();

	if (!SPIFFS.begin(true)) {
		ERROR_PRINT("SPIFFS挂载失败");
		return;
	}
	INFO_PRINT("SPIFFS挂载成功");

	PERF_START(epd_init);
	DEBUG_PRINT("初始化EPD...");
	epd_Init();
	epd_layout_hello();
	PERF_END(epd_init);
	INFO_PRINT("EPD初始化完成");

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

	INFO_PRINT("等待WiFi连接...");
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
		INFO_PRINT("WiFi连接成功");
		initNTP();
		INFO_PRINT("NTP初始化完成");

		getLocalTime(&newtime);
		INFO_PRINTF("本地时间获取完成: %d-%d-%d %d:%d:%d",
		            newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
		            newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

		PERF_START(weather_fetch);
		DEBUG_PRINT("开始获取天气数据...");
		if (weather->GetLocationCode()) {
			INFO_PRINT("位置代码获取成功");

			bool hourlySuccess = weather->GetHourlyWeather();
			bool dailySuccess = weather->Get3dWeather();

			INFO_PRINTF("天气数据获取结果: hourly=%d, daily=%d", hourlySuccess, dailySuccess);

			if (hourlySuccess && dailySuccess) {
				INFO_PRINT("所有天气数据获取成功");
				vTaskDelay(pdMS_TO_TICKS(200));
			}
			else {
				WARNING_PRINT("天气数据获取不完整");
			}
		}
		else {
			ERROR_PRINT("位置代码获取失败");
		}
		PERF_END(weather_fetch);
	}

	// 初始化UI状态
	uis.currentMenu = MENU_HOME;
	uis.updateFlag = false;
	uis.refreshType = REFRESH_FULL;
	VERBOSE_PRINTF("UI状态初始化: menu=%d, flag=%d, refresh=%d",
	               uis.currentMenu, uis.updateFlag, uis.refreshType);

	vTaskDelay(pdMS_TO_TICKS(200));

	DEBUG_PRINT("调用init_updata()...");
	init_updata();
	INFO_PRINT("init_updata()完成");

	// 内存状态检查
	MEMORY_DETAIL();

	DEBUG_PRINT("创建任务...");
	BaseType_t result1 = xTaskCreatePinnedToCore(updata_HourlyWeather, "updata_HourlyWeather", 6144, nullptr, 1, nullptr, 1);
	BaseType_t result2 = xTaskCreatePinnedToCore(updata_time, "updata_time", 2048, nullptr, 1, nullptr, 1);
	BaseType_t result3 = xTaskCreatePinnedToCore(display_update, "display_update", 16384, nullptr, 2, nullptr, 1);

	VERBOSE_PRINTF("任务创建结果: weather=%d, time=%d, display=%d", result1, result2, result3);

	ERROR_IF(result1 != pdPASS || result2 != pdPASS || result3 != pdPASS, "任务创建失败");
	INFO_IF(result1 == pdPASS && result2 == pdPASS && result3 == pdPASS, "所有任务创建成功");

	printlnA("=== setup()完成 ===");
}

void updata_HourlyWeather(void *param)
{
	TASK_INFO("updata_HourlyWeather");
	INFO_PRINT("updata_HourlyWeather任务启动");
	int cycle_count = 0;

	while (true) {
		cycle_count++;
		DEBUG_PRINTF("天气更新任务第%d次循环", cycle_count);
		STACK_CHECK("updata_HourlyWeather");

		PERF_START(weather_update);
		if (weather != nullptr && weather->GetHourlyWeather()) {
			INFO_PRINT("天气数据获取成功");
			const weatherHourlyInfo *src = weather->getHourly();
			if (src != nullptr) {
				memcpy(weather7hinfo, src, sizeof(weather7hinfo));
				VERBOSE_PRINT("天气数据拷贝完成");

				// 显示更新后的天气信息
				DEBUG_PRINTF("更新天气: 第1小时温度=%d°C", weather7hinfo[0].temp);
			}
			else {
				ERROR_PRINT("getHourly()返回空指针");
			}
		}
		else {
			ERROR_PRINT("天气数据获取失败或weather为空");
		}
		PERF_END(weather_update);

		if (uis.currentMenu == MENU_7HWEATHER) {
			DEBUG_PRINT("当前为天气界面，设置更新标志");
			uis.updateFlag = true;
		}

		VERBOSE_PRINT("天气任务进入休眠(30分钟)");
		TASK_DELAY_INFO("updata_HourlyWeather", 1800000);
		vTaskDelay(pdMS_TO_TICKS(1800000)); // 30分钟
	}
}

void updata_time(void *param)
{
	TASK_INFO("updata_time");
	INFO_PRINT("updata_time任务启动");
	int cycle_count = 0;

	while (true) {
		cycle_count++;
		VERBOSE_PRINTF("时间任务循环开始 #%d", cycle_count);
		STACK_CHECK("updata_time");

		try {
			VERBOSE_PRINT("准备调用getLocalTime");
			getLocalTime(&newtime);
			VERBOSE_PRINT("getLocalTime调用成功");

			display_main_data.new_timeinfo = newtime;
			topbar.new_timeinfo = newtime;
			VERBOSE_PRINT("时间数据赋值完成");

			if (uis.currentMenu == MENU_HOME) {
				VERBOSE_PRINT("当前为主页，准备设置更新标志");
				uis.refreshType = REFRESH_PARTIAL;
				uis.updateFlag = true;
				// 显示当前时间
				DEBUG_PRINTF("当前时间: %02d:%02d:%02d",
				             newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
			}

			VERBOSE_PRINTF("时间任务即将休眠，循环#%d", cycle_count);
		}
		catch (...) {
			ERROR_PRINTF("时间任务异常，循环#%d", cycle_count);
		}

		TASK_DELAY_INFO("updata_time", 60000);
		vTaskDelay(pdMS_TO_TICKS(60000));
		VERBOSE_PRINTF("时间任务休眠结束，循环#%d", cycle_count);
	}
}

void display_update(void *param)
{
	TASK_INFO("display_update");
	INFO_PRINT("display_update任务启动");
	int update_count = 0;

	while (true) {
		if (uis.updateFlag == true) {
			update_count++;
			DEBUG_PRINTF("显示更新#%d: menu=%d, refresh=%d",
			             update_count, uis.currentMenu, uis.refreshType);
			STACK_CHECK("display_update");

			if (xSemaphoreTake(epdMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
				VERBOSE_PRINT("获取EPD互斥锁成功");

				try {
					PERF_START(display_render);
					switch (uis.currentMenu) {
						case MENU_HOME:
							DEBUG_PRINT("调用display_main()...");
							display_main(&topbar,&display_main_data, &uis);
							VERBOSE_PRINT("display_main()完成");
							break;
						case MENU_7HWEATHER:
							DEBUG_PRINT("调用display_weather()...");
							display_weather(&topbar,weather7hinfo, &uis);
							VERBOSE_PRINT("display_weather()完成");
							break;
						default:
							WARNING_PRINTF("未知菜单类型: %d", uis.currentMenu);
							break;
					}
					PERF_END(display_render);
				}
				catch (...) {
					ERROR_PRINT("显示函数调用异常");
				}

				xSemaphoreGive(epdMutex);
				VERBOSE_PRINT("释放EPD互斥锁");
				uis.updateFlag = false;
				VERBOSE_PRINT("更新标志已清除");
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
		MEMORY_CHECK();
		last_memory_check = millis();
	}

	vTaskDelay(pdMS_TO_TICKS(100));
}