/***
 * @Author: mojionghao
 * @Date: 2025-05-21 20:36:49
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-21 22:51:04
 * @FilePath: \s3_EPD_UIdesign\src\wifiuser.cpp
 * @Description:
 */
#include "wifiuser.h"

WifiUser::WifiUser(const char *ap_ssid, int timeout)
	: server(webPort), apIP(192, 168, 4, 1)
{

	this->ap_ssid = ap_ssid; // 将传入的参数赋值给成员变量
	this->timeout = timeout; // 同样处理 timeout
}


/**
 * @brief 处理客户端对根路径 ("/") 的 HTTP 请求。
 *
 * 此函数检查请求是否包含参数 "selectSSID"，并根据结果返回一个 HTML 页面。
 * HTML 页面包含 WiFi 配置界面和动态生成的 WiFi 列表。
 *
 * @details
 * - 如果请求包含参数 "selectSSID"，返回包含 WiFi 列表的 HTML 页面。
 * - 如果请求不包含参数，仍然返回相同的 HTML 页面。
 * - HTML 页面由 ROOT_HTML 和 scanNetworksID 拼接而成。
 *
 * @return void
 */
void WifiUser::HandleRoot()
{
	File file = SPIFFS.open("/index.html", "r");
	if (!file) {
		server.send(500, "text/plain", "Failed to open index.html");
		return;
	}

	String html = file.readString(); // 读取文件内容
	file.close();

	// 替换占位符 ${scanNetworksID} 为实际的 WiFi 列表
	html.replace("${scanNetworksID}", scanNetworksID);

	server.send(200, "text/html", html);
}

void WifiUser::handleConfigWifi()
{
	//检查是否收到了wifi的账号密码
	if (server.hasArg("ssid") && server.hasArg("pass")) {
		Serial.print("got ssid:");
		wifi_ssid = server.arg("ssid");
		Serial.println(wifi_ssid);

		Serial.print("got password:");
		wifi_pass = server.arg("pass");
		Serial.println(wifi_pass);
	}
	else {
		Serial.println("error, not found ssid or password");
		server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid or password");
		return;  // Removed the '0' since this is a void function
	}

	server.send(200, "text/html", "<meta charset='UTF-8'>SSID:" + wifi_ssid + "<br />password:" + wifi_pass + "<br />已取得WiFi信息,正在尝试连接,请手动关闭此页面。");
	delay(2000);

	WiFi.softAPdisconnect(true);//关闭wifi
	server.close();//关闭web服务

	WiFi.softAPdisconnect();
	Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("开始调用连接方法");
		connectWiFi(timeout);
	}
	else {
		Serial.println("WiFi已连接");
	}
}

void WifiUser::handleNotFound()
{
	WifiUser::HandleRoot();
	server.send(404, "text/html", "<h1>404 Not Found</h1><p>The requested resource was not found on this server.</p>");
}

void WifiUser::initSoftAp()
{
	WiFi.mode(WIFI_AP);
	if (!WiFi.softAPConfig(this->apIP, this->apIP, IPAddress(255, 255, 255, 0))) {
		Serial.println("AP Config Failed");
	}

	if (WiFi.softAP(this->ap_ssid)) {
		Serial.println("AP Started");
		Serial.print("Soft-AP IP address = ");
		Serial.println(WiFi.softAPIP());                                                //接入点ip
		Serial.println(String("MAC address = ")  + WiFi.softAPmacAddress().c_str());    //接入点mac
	}
	else {
		Serial.println("AP Start Failed");
		delay(1000);
		Serial.println("restart now...");
		ESP.restart();
	}
}



void WifiUser::initDNS()
{
	if (dnsserver.start(this->DNS_PORT, "*", this->apIP)) {
		Serial.println("start dnsserver success.");
	}
	else {
		Serial.println("start dnsserver failed.");
	}
}

/**
 * @brief 初始化 Web 服务器并设置路由。
 *
 * 此函数负责启动 MDNS 服务，并为 Web 服务器设置路由规则。
 * - "/" 路由：处理 HTTP GET 请求，返回 WiFi 配置页面。
 * - "/configwifi" 路由：处理 HTTP POST 请求，接收 WiFi 配置信息。
 *
 * @details
 * - 使用 MDNS 服务为设备提供易记的域名（如 "esp32-wifi-config.local"）。
 * - 使用 std::bind 将类的成员函数绑定到 Web 服务器的路由中。
 *
 * @return void
 */
void WifiUser::initWebserver()
{
	// 启动 MDNS 服务，设置设备的域名为 "esp32-wifi-config.local"
	if (MDNS.begin("esp32-wifi-config")) {
		Serial.println("MDNS responder started");
	}
	else {
		Serial.println("Error setting up MDNS responder!");
	}

	// 设置 "/" 路由，处理 HTTP GET 请求，返回 WiFi 配置页面
	server.on("/", HTTP_GET, std::bind(&WifiUser::HandleRoot, this));

	// 设置 "/configwifi" 路由，处理 HTTP POST 请求，接收 WiFi 配置信息
	server.on("/configwifi", HTTP_POST, std::bind(&WifiUser::handleConfigWifi, this));

	server.onNotFound(std::bind(&WifiUser::handleNotFound, this));

	server.begin(); // 启动 Web 服务器
	Serial.println("Web server started");
}

bool WifiUser::scanWiFi()
{
	Serial.println("scan start");
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0) {
		scanNetworksID = "<p>未找到可用的 WiFi 网络</p>";
		return false;
	}
	else {
		scanNetworksID = "";
		for (int i = 0; i < n; i++) {
			scanNetworksID += "<div class='wifi-item'>";
			scanNetworksID += "<span class='wifi-name'>" + WiFi.SSID(i) + "</span>";
			scanNetworksID += "<span class='wifi-signal'>信号强度: " + String(WiFi.RSSI(i)) + "dBm</span>";
			scanNetworksID += "</div>";
		}
		return true;
	}
}

void WifiUser::connectWiFi(int timeout_s)
{
	if (timeout_s == 0) {
		timeout_s = this->timeout; // 如果未传入参数，使用成员变量 timeout
	}
	Serial.println("Connecting to WiFi...");
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	if (wifi_ssid != "" && wifi_pass != "") {
		WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
	}
	else {
		Serial.println("use nvs data connect wifi");
		WiFi.begin();
	}
	int ConnectedTime = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		ConnectedTime++;
		Serial.print(".");

		if (ConnectedTime >= timeout_s) {
			Serial.println("");
			Serial.println("连接超时,请检查WiFi账号密码是否正确");
			WiFi.disconnect(); // 确保断开之前的连接
			wifiConfig();      // 重新进入配置模式
			return;
		}
	}
	if (WiFi.status() == WL_CONNECTED) {        //如果连接成功
		Serial.println("WIFI connect Success");
		Serial.printf("SSID:%s", WiFi.SSID().c_str());
		Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
		Serial.print("LocalIP:");
		Serial.print(WiFi.localIP());
		Serial.print(" ,GateIP:");
		Serial.println(WiFi.gatewayIP());
		Serial.print("WIFI status is:");
		Serial.print(WiFi.status());
		server.stop();                            //停止开发板所建立的网络服务器。
	}
}

void WifiUser::wifiConfig()
{
	// 初始化软AP
	initSoftAp();

	// 初始化 DNS 服务器
	initDNS();

	// 初始化 Web 服务器
	initWebserver();

	// 扫描 WiFi 网络
	scanWiFi();
}

void WifiUser::removeWifi()
{
	esp_wifi_restore();
	Serial.println("连接信息已清空,准备重启设备..");
	delay(1000);
	ESP.restart(); // 重启设备
}

void WifiUser::checkConnect(bool reConnect)
{
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("WiFi is not connected, attempting to reconnect...");
		if (reConnect == true && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
			Serial.println("WIFI未连接.");
			Serial.println("WiFi Mode:");
			Serial.println(WiFi.getMode());
			Serial.println("正在连接WiFi...");
			connectWiFi(timeout);          //连接wifi函数
		}
		else {
			Serial.println("WiFi is already connected.");
		}
	}
}
void  WifiUser::checkDNS_HTTP()
{
	dnsserver.processNextRequest();   //检查客户端DNS请求
	server.handleClient();            //检查客户端(浏览器)http请求
}