#include <Arduino.h>
#include <display.h>
#include <WiFi.h>
#include <stdio.h>
#include <UrlEncode.h>
#include "display_main.h"
#include "display_weather.h"  //添加天气显示头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#define GPIO0_PIN_WIFIRESET 0 // GPIO0引脚定义

// 设置你的WiFi信息
const char *ssid     = "mate13";
const char *password = "12345678";
#define USER_KEY "0781c49e69024849b7cb76ef017ca453"
const String  city = "东莞" ;

WifiUser *wifiuser = nullptr; // 创建 WifiUser 对象
Button *gpio0Button = nullptr;
Weather *weather = nullptr;

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

void checkGpioTask()
{
	Serial.println("GPIO0 按下，清除网络信息...");
	wifiuser->removeWifi(); // 调用清除网络信息的函数
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting EPD UI Design...");

	if (!SPIFFS.begin(true)) {
		Serial.println("Failed to mount SPIFFS");
		return;
	}

	gpio0Button = new Button(GPIO0_PIN_WIFIRESET, checkGpioTask);
	wifiuser = new WifiUser("EZ_EPD", 10);
	weather = new Weather(USER_KEY, urlEncode(city));


	if (wifiuser->isConnected()) {
		Serial.println("\nWiFi connected successfully!");
		Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

		if (weather->GetLocationCode()) {
			if (weather->GetHourlyWeather()) {
				Serial.printf("天气数据获取成功\n");
			}

		}

		// 初始化其他组件
		initNTP();

		Serial.println("NTP initialized");

		epd_Init();
		Serial.println("EPD initialized");

		delay(2000); // 等待NTP同步
		UIStatus uis;
		uis.last_currentMenu = MENU_HOME;  // 🔥 修复：使用枚举值而不是0
		uis.updateFlag = updata_flag_none;
		uis.refreshType = REFRESH_FULL;

		display_weather(weather->getHourly(), &uis);

	}
	else {
		Serial.println("\nWiFi connection failed! Please configure WiFi.");
		// 可以显示一个配网提示页面
	}


}

// 在loop中可以添加定期更新
void loop()
{

}