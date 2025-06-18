#include "weather.h"

#define USER_KEY "0781c49e69024849b7cb76ef017ca453"
const String city = "从化";

void setup() {
    Serial.begin(115200);

    // 创建 Weather 对象
    Weather weather(USER_KEY, city);

    // 获取城市 ID
    if (weather.GetLocationCode()) {
        Serial.println("城市 ID 获取成功！");

        // 获取 3 天的天气信息
        if (weather.Get3dWeather()) {
            Serial.println("天气获取成功！");
            Serial.printf("今天日期: %s\n", weather.getToday().fxDate.c_str());
            Serial.printf("今天最高温: %d\n", weather.getToday().tempMax);
            Serial.printf("今天最低温: %d\n", weather.getToday().tempMin);
            Serial.printf("今天白天天气: %s\n", weather.getToday().textDay.c_str());
        } else {
            Serial.println("天气获取失败！");
        }
    } else {
        Serial.println("城市 ID 获取失败！");
    }
}

void loop() {
    // 可根据需要定时刷新天气
}