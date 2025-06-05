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

#define ROOT_HTML  "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>WIFI</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"POST\" action=\"configwifi\"><label class=\"input\"><span>WiFi SSID</span><input type=\"text\" name=\"ssid\" value=\"\"></label><label class=\"input\"><span>WiFi PASS</span> <input type=\"text\"  name=\"pass\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submit\"> <p><span> Nearby wifi:</P></form>"
};

#endif