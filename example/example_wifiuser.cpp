#include "wifiuser.h"

#define GPIO0_PIN_WIFIRESET 0 // GPIO0 引脚定义

// 创建 WifiUser 对象
WifiUser wifiUser("ESP32-Config", 10);

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
	Serial.begin(115200);

	// 创建 GPIO 检测任务
	xTaskCreate(
	    checkGpioTask,    // 任务函数
	    "Check GPIO0",    // 任务名称
	    2048,             // 任务堆栈大小（字节）
	    NULL,             // 任务参数
	    1,                // 任务优先级
	    NULL              // 任务句柄
	);

	Serial.println("Starting WiFi User Test...");

	// 尝试连接 WiFi
	wifiUser.connectWiFi();
}

void loop()
{
	// 检查 WiFi 连接状态
	wifiUser.checkConnect(true);

	// 检查 DNS 和 HTTP 请求
	wifiUser.checkDNS_HTTP();

	delay(100); // 延时 100 毫秒，避免占用过多 CPU 时间
}