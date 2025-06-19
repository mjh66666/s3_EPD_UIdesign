#pragma once
#include <stddef.h>
#include <WiFi.h>
#include <time.h>
#include <string.h>
// NTP初始化
void initNTP();

// 获取当前时间（24小时制）
void getTime(int &hour, int &minute, int &second);

void getdate(char *buf, size_t buflen);