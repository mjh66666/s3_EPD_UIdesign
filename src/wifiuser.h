#ifndef _WIFIUSER_H_
#define _WIFIUSER_H_

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

class WifiUser {
public:
	WifiUser(const char *ap_ssid, int timeout);
	void handleConfigWifi();
	void checkConnect(bool reConnect);
	void checkDNS_HTTP();
	void removeWifi();
	void connectWiFi(int timeout_s = 0);
	void wifiConfig();
private:
	IPAddress apIP;
	String wifi_ssid;
	String wifi_pass;
	String scanNetworksID;
	const byte DNS_PORT = 53;
	const int webPort = 80;
	DNSServer dnsserver;
	WebServer server;
	String ap_ssid; // 存储热点的 SSID
	int timeout;    // 存储超时时间
	void HandleRoot();
	void initSoftAp();
	void handleNotFound();
	void initDNS();
	void initWebserver();
	bool scanWiFi();


};

#endif