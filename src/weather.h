/***
 * @Author: mojionghao
 * @Date: 2025-05-21 10:51:18
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-05-21 22:43:13
 * @FilePath: \s3_EPD_UIdesign\src\weather.h
 * @Description:
 */
#ifndef  _WEATHER_H_
#define  _WEATHER_H_

#include <Arduino.h>
#include <ArduinoJson.h>

struct weatherDailyInfo {
	String fxDate;
	int tempMax;
	int tempMin;
	String textDay;
	String windDirDay;
	String winddScaleDay;
	int iconDay;
	String textNight;
	String windDirNight;
	String winddScaleNight;
	int iconNight;
};

struct weatherHourlyInfo {
	String fxTime;
	int temp;
	int icon;
	String text;
	String windDir;
	String windScale;
};

class Weather {
public:
	Weather(String key, String city);
	bool GetLocationCode();
	bool Get3dWeather();
	bool GetHourlyWeather();

	// 只读getter方法
	const weatherDailyInfo &getToday() const
	{
		return today;
	}
	const weatherDailyInfo &getTomorrow() const
	{
		return tomorrow;
	}
	const weatherDailyInfo &getDayAfterTomorrow() const
	{
		return dayAfterTomorrow;
	}
	const weatherHourlyInfo &getHourly(int idx) const
	{
		return hourly[idx];
	}

private:
	static const char *weather_host; //天气查询服务器地址
	static const char *location_host; // 位置查询地址
	String city;
	String key;
	String location_id;
	weatherDailyInfo today;
	weatherDailyInfo tomorrow;
	weatherDailyInfo dayAfterTomorrow;
	weatherHourlyInfo hourly[24];

	bool parseDailyWeatherInfo(const JsonObject &src, weatherDailyInfo &dest);
	bool parseHourlyWeatherInfo(const JsonObject &src, weatherHourlyInfo &dest);
};
#endif // ! _WEATHER_H_