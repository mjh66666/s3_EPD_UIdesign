#include "get_time.h"
#include <WiFi.h>
#include <time.h>
#include <string.h>

const char *ntpServer = "ntp.aliyun.com";
const long  gmtOffset_sec = 8 * 3600; // 东八区
const int   daylightOffset_sec = 0;

void initNTP()
{
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void getTime(int &hour, int &minute, int &second)
{
	struct tm timeinfo;
	if (getLocalTime(&timeinfo)) {
		hour = timeinfo.tm_hour;
		minute = timeinfo.tm_min;
		second = timeinfo.tm_sec;
	}
	else {
		hour = minute = second = 0;
	}
}


void getdate(char *buf, size_t buflen)
{
	struct tm timeinfo;
	if (getLocalTime(&timeinfo)) {
		strftime(buf, buflen, "%a,%d %b %Y", &timeinfo);
	}
	else {
		strncpy(buf, "N/A", buflen);
	}
}