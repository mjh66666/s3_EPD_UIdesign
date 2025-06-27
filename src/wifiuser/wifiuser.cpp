/***
 * @Author: mojionghao
 * @Date: 2025-06-18 20:07:46
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-27 21:52:18
 * @FilePath: \s3_EPD_UIdesign\src\wifiuser\wifiuser.cpp
 * @Description:
 */
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
	connectWiFi(timeout);
	xTaskCreatePinnedToCore(WifiUser::reconnectTask, "ReconnectTask", 8192, this, 1, NULL, 0);
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
	scanWiFi();
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

/**
 * @brief 处理 WiFi 配置请求
 *
 * 此函数接收客户端提交的 WiFi SSID 和密码，并尝试连接到指定的 WiFi 网络。
 */
void WifiUser::handleConfigWifi()
{
	// 检查是否收到了 WiFi 的账号密码
	if (server.hasArg("ssid") && server.hasArg("pass")) {
		LOG("Received SSID:");
		wifi_ssid = server.arg("ssid");
		LOG(wifi_ssid);

		LOG("Received password:");
		wifi_pass = server.arg("pass");
		LOG(wifi_pass);
	}
	else {
		LOG("Error: SSID or password not found");
		server.send(200, "text/html", "<meta charset='UTF-8'>Error: SSID or password not found");
		return;
	}

	server.send(200, "text/html", "<meta charset='UTF-8'>SSID:" + wifi_ssid + "<br />Password:" + wifi_pass + "<br />WiFi credentials received. Please close this page manually.");
	delay(2000);

	server.close();              // 关闭 Web 服务

	WiFi.softAPdisconnect();
	LOG("WiFi Connect SSID: " + wifi_ssid + "  PASS: " + wifi_pass);
	if (WiFi.status() != WL_CONNECTED) {
		LOG("Starting connection method");
		connectWiFi(timeout);
	}
	else {
		LOG("WiFi is already connected");
	}
}

/**
 * @brief 处理未找到的 HTTP 请求
 *
 * 返回 404 页面。
 */
void WifiUser::handleNotFound()
{
	String uri = server.uri();
	LOG("404 - Request for: " + uri);

	// 对于浏览器的自动请求，直接返回404
	if (uri == "/favicon.ico" || uri == "/robots.txt" || uri.startsWith("/css/") || uri.startsWith("/js/") ||
	        uri.startsWith("/images/")) {
		server.send(404, "text/plain", "Not Found");
		return;
	}

	// 对于其他未知路径，重定向到根路径（不要直接调用HandleRoot）
	server.sendHeader("Location", "/");
	server.send(302, "text/plain", "Redirecting to main page...");

}

/**
 * @brief 初始化 AP 模式
 *
 * 设置设备为 AP 模式，并配置 IP 地址。
 */
void WifiUser::initSoftAp()
{
	WiFi.mode(WIFI_AP);
	if (!WiFi.softAPConfig(this->apIP, this->apIP, IPAddress(255, 255, 255, 0))) {
		LOG("AP Config Failed");
	}

	if (WiFi.softAP(this->ap_ssid)) {
		LOG("AP Started");
		LOGF("Soft-AP IP address = %s", WiFi.softAPIP().toString().c_str());
		LOGF("MAC address = %s", WiFi.softAPmacAddress().c_str());
	}
	else {
		LOG("AP Start Failed");
		delay(1000);
		LOG("Restarting now...");
		ESP.restart();
	}
}

/**
 * @brief 初始化 DNS 服务器
 *
 * 启动 DNS 服务器，用于处理客户端的 DNS 请求。
 */
void WifiUser::initDNS()
{
	if (dnsserver.start(this->DNS_PORT, "*", this->apIP)) {
		LOG("DNS server started successfully.");
	}
	else {
		LOG("Failed to start DNS server.");
	}
}

/**
 * @brief 初始化 Web 服务器并设置路由
 *
 * 启动 MDNS 服务，并为 Web 服务器设置路由规则。
 */
void WifiUser::initWebserver()
{
	// 启动 MDNS 服务，设置设备的域名为 "esp32-wifi-config.local"
	if (MDNS.begin("esp32-wifi-config")) {
		LOG("MDNS responder started");
	}
	else {
		LOG("Error setting up MDNS responder!");
	}

	// 设置 "/" 路由，处理 HTTP GET 请求，返回 WiFi 配置页面
	server.on("/", HTTP_GET, std::bind(&WifiUser::HandleRoot, this));

	// 设置 "/configwifi" 路由，处理 HTTP POST 请求，接收 WiFi 配置信息
	server.on("/configwifi", HTTP_POST, std::bind(&WifiUser::handleConfigWifi, this));

	server.onNotFound(std::bind(&WifiUser::handleNotFound, this));

	server.begin(); // 启动 Web 服务器
	LOG("Web server started");
}

/**
 * @brief 扫描 WiFi 网络
 *
 * 扫描周围的 WiFi 网络，并生成 HTML 格式的 WiFi 列表。
 *
 * @return 是否成功扫描到 WiFi 网络
 */
bool WifiUser::scanWiFi()
{
	LOG("Scan started");
	int n = WiFi.scanNetworks();
	LOG("Scan completed");
	if (n == 0) {
		scanNetworksID = "<p>No available WiFi networks found</p>";
		return false;
	}
	else {
		scanNetworksID = "";
		for (int i = 0; i < n; i++) {
			scanNetworksID += "<div class='wifi-item'>";
			scanNetworksID += "<span class='wifi-name'>" + WiFi.SSID(i) + "</span>";
			scanNetworksID += "<span class='wifi-signal'>Signal strength: " + String(WiFi.RSSI(i)) + "dBm</span>";
			scanNetworksID += "</div>";
		}
		return true;
	}
}

/**
 * @brief 尝试连接 WiFi
 *
 * 根据提供的 SSID 和密码，尝试连接到 WiFi 网络。
 *
 * @param timeout_s 超时时间（秒）
 */
void WifiUser::connectWiFi(int timeout_s)
{
	if (timeout_s == 0) {
		timeout_s = this->timeout;
	}
	LOG("Connecting to WiFi...");
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);

	if (wifi_ssid != "" && wifi_pass != "") {
		WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
	}
	else {
		LOG("Using NVS data to connect WiFi");
		WiFi.begin();
	}

	unsigned long startTime = millis();
	while (WiFi.status() != WL_CONNECTED) {
		if (millis() - startTime > timeout_s * 1000) {
			LOG("Connection timed out. Please check if the WiFi SSID and password are correct.");
			WiFi.disconnect();
			wifiConfig(); // 进入 WiFi 配置模式
			return;
		}
		delay(100); // 短暂延时，避免占用过多 CPU 时间
	}

	LOG("WiFi connected successfully");
	LOGF("SSID: %s", WiFi.SSID().c_str());
	LOGF("Password: %s", WiFi.psk().c_str());
	LOGF("Local IP: %s, Gateway IP: %s", WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str());
	LOGF("WiFi status: %d", WiFi.status());
	server.stop();
}

/**
 * @brief 进入 WiFi 配置模式
 *
 * 初始化 AP 模式、DNS 服务器和 Web 服务器。
 */
void WifiUser::wifiConfig()
{
	// 停止可能正在运行的服务
	dnsserver.stop();
	server.close();
	server.stop();


	// 初始化软AP
	initSoftAp();

	// 初始化 DNS 服务器
	initDNS();

	// 初始化 Web 服务器
	initWebserver();
}

/**
 * @brief 清除 WiFi 配置信息
 *
 * 恢复 WiFi 出厂设置，并重启设备。
 */
void WifiUser::removeWifi()
{
	esp_wifi_restore();
	LOG("WiFi credentials cleared. Restarting device...");
	delay(1000);
	ESP.restart(); // 重启设备
}

/**
 * @brief 检查 WiFi 连接状态
 *
 * 如果未连接且允许重新连接，则尝试重新连接。
 *
 * @param reConnect 是否允许重新连接
 */
void WifiUser::checkConnect(bool reConnect)
{
	static wl_status_t lastStatus = WL_IDLE_STATUS; // 上一次的 WiFi 状态
	static unsigned long lastReconnectAttempt = 0; // 上次重新连接的时间
	static int reconnectAttempts = 0; // 重连尝试次数
	static bool tryingReconnect = false; // 是否正在尝试重连

	wl_status_t currentStatus = WiFi.status();

	if (currentStatus != lastStatus) {
		if (currentStatus == WL_CONNECTED) {
			LOG("WiFi connected successfully.");
			LOGF("SSID: %s", WiFi.SSID().c_str());
			LOGF("Local IP: %s", WiFi.localIP().toString().c_str());
			reconnectAttempts = 0; // 重置重连次数
			tryingReconnect = false;
		}
		else {
			LOG("WiFi is not connected.");
			reconnectAttempts = 0; // 重置重连次数，开始新的重连周期
			tryingReconnect = false;
		}
		lastStatus = currentStatus; // 更新状态
	}

	// 如果断开连接且允许重连
	if (reConnect && currentStatus != WL_CONNECTED &&
	        millis() - lastReconnectAttempt > 15000) { // 每 15 秒尝试一次

		lastReconnectAttempt = millis();

		if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
			if (reconnectAttempts < 10) { // 先尝试10次重连之前的WiFi
				LOGF("Attempting to reconnect to previous WiFi (attempt %d/10)...", reconnectAttempts + 1);
				tryingReconnect = true;
				tryReconnectPreviousWiFi();
				reconnectAttempts++;
			}
			else {
				LOG("Failed to reconnect to previous WiFi after 3 attempts. Starting AP mode for WiFi configuration...");
				wifiConfig(); // 进入 WiFi 配置模式
				reconnectAttempts = 0; // 重置计数器
			}
		}
		else {
			LOG("AP mode is already active. Please connect to the AP to configure WiFi.");
		}
	}
}

/**
 * @brief 尝试重连之前连接过的WiFi
 *
 * 使用NVS中保存的WiFi凭据尝试重新连接
 */
void WifiUser::tryReconnectPreviousWiFi()
{
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);

	// 尝试使用NVS中保存的凭据连接
	WiFi.begin();

	LOG("Trying to reconnect to previous WiFi...");
	// 不等待连接完成，让checkConnect函数在下次调用时检查状态
}


/**
 * @brief 处理 DNS 和 HTTP 请求
 *
 * 在 AP 模式下处理客户端的 DNS 和 HTTP 请求。
 */
void WifiUser::checkDNS_HTTP()
{
	if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
		dnsserver.processNextRequest();   // 检查客户端 DNS 请求
		server.handleClient();            // 检查客户端 HTTP 请求
	}
}

/**
 * @brief 检查是否已连接到 WiFi
 *
 * @return 是否已连接到 WiFi
 */
bool WifiUser::isConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

void WifiUser::reconnectTask(void *param)
{
	WifiUser *self = static_cast<WifiUser *>(param);
	while (true) {
		self->checkConnect(true); // 检查连接状态并尝试重新连接
		self->checkDNS_HTTP(); // 处理 DNS 和 HTTP 请求
		vTaskDelay(pdMS_TO_TICKS(200)); // 每 200ms 检查一次
	}
}

