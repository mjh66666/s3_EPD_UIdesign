/***
 * @Author: mojionghao
 * @Date: 2025-05-21 10:56:36
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-07-05 09:25:12
 * @FilePath: \s3_EPD_UIdesign\src\weather\weather.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-05-21 10:56:36
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-21 11:20:24
 * @FilePath: \s3_EPD_UIdesign\src\weather.cpp
 * @Description:
 */
#include "weather.h"
#include "HttpsGetUtils.h"
#include <algorithm>

const char *Weather::weather_host = "https://devapi.qweather.com";
const char *Weather::location_host = "https://geoapi.qweather.com";

Weather::Weather(String key, String city)
{

	this->key = key;
	this->city = city;
}

bool Weather::GetLocationCode()
{
	String location_url = String(location_host) + "/v2/city/lookup?location=" + city + "&key=" + key;
	uint8_t *outbuf = NULL;
	size_t len = 0;
	Serial.println("get locationcode now");
	bool result = HttpsGetUtils::getString(location_url.c_str(), outbuf, len);
	if (outbuf && len) {
		StaticJsonDocument<512> doc;
		DeserializationError err = deserializeJson(doc, (char *)outbuf, len);
		if (!err) {
			JsonArray locations = doc["location"].as<JsonArray>();
			if (locations.size() > 0) {
				this->location_id = locations[0]["id"].as<String>();
				Serial.print("location_id: ");
				Serial.println(this->location_id);
			}
		}
		else {
			Serial.print("JSON解析失败: ");
			Serial.println(err.c_str());
		}
		if (outbuf != NULL) {
			free(outbuf);
			outbuf = NULL;
		}
	}
	return result;
}

bool Weather::Get3dWeather()
{
	String url_3d = String(weather_host) + "/v7/weather/3d?location=" + location_id + "&key=" + key;
	uint8_t *outbuf = NULL;
	size_t len = 0;
	Serial.println("get 3dweather now");
	bool result = HttpsGetUtils::getString(url_3d.c_str(), outbuf, len);
	if (outbuf && len) {
		StaticJsonDocument<512> doc;
		DeserializationError err = deserializeJson(doc, (char *)outbuf, len);
		if (!err) {
			JsonArray daily = doc["daily"].as<JsonArray>();
			if (daily.size() >= 3) {
				if (!parseDailyWeatherInfo(daily[0], this->today)) {
					result = false;
				}
				if (!parseDailyWeatherInfo(daily[1], this->tomorrow)) {
					result = false;
				}
				if (!parseDailyWeatherInfo(daily[2], this->dayAfterTomorrow)) {
					result = false;
				}
			}
			else {
				Serial.println("JSON daily 数组长度不足3天，数据异常！");
				result = false;
			}
		}
		else {
			Serial.print("JSON解析失败: ");
			Serial.println(err.c_str());
			result = false;
		}
		if (outbuf != NULL) {
			free(outbuf);
			outbuf = nullptr;
		}
	}
	else {
		Serial.println("天气数据获取失败或数据长度为0！");
		result = false;
	}
	return result;
}

bool Weather::GetHourlyWeather()
{
	String url_hourly = String(weather_host) + "/v7/weather/24h?location=" + location_id + "&key=" + key;
	uint8_t *outbuf = NULL;
	size_t len = 0;
	Serial.println("get Hourlyweather now");
	bool result = HttpsGetUtils::getString(url_hourly.c_str(), outbuf, len);
	if (outbuf && len) {
		StaticJsonDocument<5120> doc;
		DeserializationError err = deserializeJson(doc, (char *)outbuf, len);
		if (!err) {
			JsonArray hourly = doc["hourly"].as<JsonArray>();
			int count = 24;
			for (int i = 0; i < count; ++i) {
				if (!parseHourlyWeatherInfo(hourly[i], this->hourly[i])) {
					Serial.printf("第%d小时数据解析不完整！\n", i);
					result = false;
				}
			}
		}
		else {
			Serial.print("JSON解析失败: ");
			Serial.println(err.c_str());
			result = false;
		}
		free(outbuf);
		outbuf = nullptr;
	}
	else {
		Serial.println("逐小时天气数据获取失败或数据长度为0！");
		result = false;
	}
	return result;
}




bool Weather::parseDailyWeatherInfo(const JsonObject &src, weatherDailyInfo &dest)
{
	dest = weatherDailyInfo(); // 先清空
	bool ok = true;
	if (src.containsKey("fxDate")) {
		dest.fxDate = src["fxDate"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("tempMax")) {
		dest.tempMax = src["tempMax"].as<int>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("tempMin")) {
		dest.tempMin = src["tempMin"].as<int>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("textDay")) {
		dest.textDay = src["textDay"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windDirDay")) {
		dest.windDirDay = src["windDirDay"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windScaleDay")) {
		dest.winddScaleDay = src["windScaleDay"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("iconDay")) {
		dest.iconDay = src["iconDay"].as<int>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("textNight")) {
		dest.textNight = src["textNight"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windDirNight")) {
		dest.windDirNight = src["windDirNight"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windScaleNight")) {
		dest.winddScaleNight = src["windScaleNight"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("iconNight")) {
		dest.iconNight = src["iconNight"].as<int>();
	}
	else {
		ok = false;
	}
	return ok;
}

bool Weather::parseHourlyWeatherInfo(const JsonObject &src, weatherHourlyInfo &dest)
{
	dest = weatherHourlyInfo(); // 先清空
	bool ok = true;
	if (src.containsKey("fxTime")) {
		dest.fxTime = src["fxTime"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("temp")) {
		dest.temp = src["temp"].as<int>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("icon")) {
		dest.icon = src["icon"].as<int>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("text")) {
		dest.text = src["text"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windDir")) {
		dest.windDir = src["windDir"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("windScale")) {
		dest.windScale = src["windScale"].as<String>();
	}
	else {
		ok = false;
	}
	if (src.containsKey("pop")) {
		dest.pop = src["pop"].as<int>();
	}
	else {
		ok = false;
	}
	return ok;
}
