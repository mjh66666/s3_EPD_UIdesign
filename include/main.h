#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <wifiuser.h>
#include "get_time.h"
#include "weather.h"
#include "button.h"
// 菜单索引枚举
typedef enum {
	MENU_HOME = 0, //主菜单：时间 今日天气 待办
	MENU_TOMATO, //番茄钟菜单
	MENU_CLASS,  //课程表菜单
	MENU_7HWEATHER, //7日天气菜单
	MENU_SETTINGS,  //设置菜单
} MenuIndex;

// 刷新类型枚举
typedef enum {
	REFRESH_PARTIAL = 0, // 局部刷新
	REFRESH_FULL    // 全局刷新
} RefreshType;



// UI信息结构体
typedef struct {
	MenuIndex currentMenu;      // 当前菜单索引
	volatile bool updateFlag; // 更新标志
	RefreshType refreshType; // 刷新类型
} UIStatus;



#endif // MAIN_H