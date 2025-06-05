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
#include "get_time.h"
#include "weather.h"
#include <UrlEncode.h>
#include <wifiuser.h>

#define GPIO0_PIN_WIFIRESET 0 // GPIO0引脚定义

// // 设置你的WiFi信息
// const char *ssid     = "lbd";
// const char *password = "lbd123456";
// #define USER_KEY "0781c49e69024849b7cb76ef017ca453"
// const String  city = "从化" ;




// // 连接WiFi
// void connectWiFi()
// {
//  Serial.println("Connecting to WiFi...");
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//      delay(500);
//      Serial.print(".");
//  }
//  Serial.println("\nWiFi connected!");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
// }
// void setup()
// {
//  Serial.begin(115200);
//  connectWiFi();
//  initNTP();
//  epd_Init();
//  epd_full_reflash();
// }

// void loop()
// {
//  int hour, minute, second;
//  char date_str[36];
//  getTime(hour, minute, second);
//  getdate(date_str, sizeof(date_str));
//  epd_refresh_time(hour, minute, date_str); // 假设小时固定为12
// }

// void setup()
// {
//  Serial.begin(115200);
//  connectWiFi();
//  String encodecity = urlEncode(city);

//  Weather weather(USER_KEY, encodecity);
//  if (weather.GetLocationCode()) {
//      Serial.println("城市ID获取成功！");
//      if (weather.Get3dWeather()) {
//          Serial.println("天气获取成功！");
//          Serial.print("今天日期: ");
//          Serial.println(weather.getToday().fxDate);
//          Serial.print("今天最高温: ");
//          Serial.println(weather.getToday().tempMax);
//          Serial.print("今天最低温: ");
//          Serial.println(weather.getToday().tempMin);
//          Serial.print("今天白天天气: ");
//          Serial.println(weather.getToday().textDay);
//          Serial.print("明天最高温: ");
//          Serial.println(weather.getTomorrow().tempMax);
//          Serial.print("后天最高温: ");
//          Serial.println(weather.getDayAfterTomorrow().tempMax);

//          测试逐小时天气
//          if (weather.GetHourlyWeather()) {
//              Serial.println("逐小时天气获取成功！");
//              for (int i = 0; i < 24; ++i) {
//                  Serial.print("第");
//                  Serial.print(i);
//                  Serial.print("小时温度: ");
//                  Serial.println(weather.getHourly(i).temp);
//                  Serial.print("小时天气: ");
//                  Serial.println(weather.getHourly(i).text);
//                  Serial.print("小时时间: ");
//                  Serial.println(weather.getHourly(i).fxTime);
//              }
//          }
//          else {
//              Serial.println("逐小时天气获取失败！");
//          }
//      }
//      else {
//          Serial.println("天气获取失败！");
//      }
//  }
//  else {
//      Serial.println("城市ID获取失败！");
//  }
// }

// void loop()
// {
//  可根据需要定时刷新天气
// }



WifiUser wifiUser("ESP32-Config", 10); // 创建 WifiUser 对象，传入热点 SSID 和超时时间

void checkGpioTask(void *parameter)
{
	pinMode(GPIO0_PIN_WIFIRESET, INPUT_PULLUP); // 配置 GPIO0 为输入模式，启用内部上拉电阻

	while (true) {
		if (digitalRead(GPIO0_PIN_WIFIRESET) == LOW) { // 检测 GPIO0 是否被按下（低电平）
			Serial.println("GPIO0 按下，清除网络信息...");
			wifiUser.removeWifi(); // 调用清除网络信息的函数
			delay(1000); // 防止重复触发
		}
		vTaskDelay(pdMS_TO_TICKS(10)); // 每 10 毫秒检测一次
	}
}

void setup()
{
	Serial.begin(115200); // 初始化串口
	if (!SPIFFS.begin(true)) {
		Serial.println("Failed to mount SPIFFS");
		return;
	}
	xTaskCreate(
	    checkGpioTask,    // 任务函数
	    "Check GPIO0",    // 任务名称
	    2048,             // 任务堆栈大小（字节）
	    NULL,             // 任务参数
	    1,                // 任务优先级
	    NULL              // 任务句柄
	);
	Serial.println("Starting WiFi User Test...");
	// 初始化 WiFi 配置模式
	//wifiUser.wifiConfig();

	// 尝试连接 WiFi
	wifiUser.connectWiFi();

}

void loop()
{
	// 检查 WiFi 连接状态
	wifiUser.checkConnect(true);

	// 检查 DNS 和 HTTP 请求
	wifiUser.checkDNS_HTTP();

	// 延时 100 毫秒，避免占用过多 CPU 时间
	delay(100);
}