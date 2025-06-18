#include "get_time.h"

void setup() {
    Serial.begin(115200);

    // 初始化 NTP 时间同步
    initNTP();
}

void loop() {
    int hour, minute, second;
    char date_str[36];

    // 获取当前时间
    getTime(hour, minute, second);

    // 获取当前日期
    getdate(date_str, sizeof(date_str));

    // 打印时间和日期
    Serial.printf("当前时间: %02d:%02d:%02d\n", hour, minute, second);
    Serial.printf("当前日期: %s\n", date_str);

    delay(1000); // 每秒刷新一次
}