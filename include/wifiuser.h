#ifndef _WIFIUSER_H_
#define _WIFIUSER_H_

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

// 日志开关宏定义
#define WIFIUSER_DEBUG 1 // 设置为 1 启用日志，设置为 0 禁用日志

#if WIFIUSER_DEBUG
	#define LOG(x) Serial.println(x)
	#define LOGF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
	#define LOG(x)
	#define LOGF(fmt, ...)
#endif

class WifiUser {
public:
	WifiUser(const char *ap_ssid, int timeout);
	void handleConfigWifi();          // 处理 WiFi 配置
	void checkConnect(bool reConnect); // 检查 WiFi 连接状态
	void checkDNS_HTTP();             // 处理 DNS 和 HTTP 请求
	void removeWifi();                // 清除 WiFi 配置信息
	void connectWiFi(int timeout_s = 0); // 尝试连接 WiFi
	void wifiConfig();                // 进入 WiFi 配置模式
	bool isConnected();               // 检查是否已连接到 WiFi

private:
	IPAddress apIP;                   // AP 模式的 IP 地址
	String wifi_ssid;                 // WiFi SSID
	String wifi_pass;                 // WiFi 密码
	String scanNetworksID;            // 扫描到的 WiFi 网络信息
	const byte DNS_PORT = 53;         // DNS 服务器端口
	const int webPort = 80;           // Web 服务器端口
	DNSServer dnsserver;              // DNS 服务器实例
	WebServer server;                 // Web 服务器实例
	String ap_ssid;                   // 热点的 SSID
	int timeout;                      // WiFi 连接超时时间
	bool configModeActive;            // 是否处于配置模式

	void HandleRoot();                // 处理 Web 服务器根路径请求
	void initSoftAp();                // 初始化 AP 模式
	void handleNotFound();            // 处理 404 请求
	void initDNS();                   // 初始化 DNS 服务器
	void initWebserver();             // 初始化 Web 服务器
	bool scanWiFi();                  // 扫描 WiFi 网络
	static void reconnectTask(void *param); // 重新连接任务
};

#endif