#include "display_main.h"
#include "debug.h"  // 包含调试系统

// 保证和 display.cpp 里的宏一致
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

extern U8G2_FOR_ADAFRUIT_GFX u8g2_epd;
extern GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

// 区域1：顶部栏（如时间日期区）
#define AREA_TOPBAR_X      0
#define AREA_TOPBAR_Y      0
#define AREA_TOPBAR_W      296
#define AREA_TOPBAR_H      18

#define AREA_TIME_X        0
#define AREA_TIME_Y        24
#define AREA_TIME_W        175
#define AREA_TIME_H        48

#define AREA_DATA_X        0
#define AREA_DATA_Y        71
#define AREA_DATA_W        175
#define AREA_DATA_H        57  // 128-71

// 区域3：右侧功能区（如TO-DO清单区）
#define AREA_TODO_X       175
#define AREA_TODO_Y       18
#define AREA_TODO_W       121  // 296-175
#define AREA_TODO_H       112  // 128-16

static uint8_t partial_count = 0; // 局部刷新标志

void display_main(const display_topbar_t *topbar,const display_main_t *display_main_data, UIStatus *uis)
{
    FUNC_ENTER();
    PERF_START(display_main);

    u8g2_epd.setBackgroundColor(GxEPD_WHITE);
    u8g2_epd.setFontMode(1);
    u8g2_epd.setFontDirection(0);
    u8g2_epd.setForegroundColor(GxEPD_BLACK);

    //如果当前界面是全局刷新或局部刷新次数超过10次，则进行全局刷新
    if (uis->refreshType == REFRESH_FULL || partial_count > 10) {
        display.setFullWindow();
        INFO_PRINTF("执行全局刷新 - partial_count重置前: %d", partial_count);
        partial_count = 0;
    }
    else {
        display.setPartialWindow(0, 0, display.width(), display.height());
        partial_count++;
        VERBOSE_PRINTF("执行局部刷新 #%d", partial_count);
    }

    display.firstPage();
    do {
        if (uis->refreshType == REFRESH_FULL) {
            display.fillScreen(GxEPD_WHITE);
        }

        display.drawInvertedBitmap(AREA_TODO_X + ((AREA_TODO_W - 32) / 2), AREA_TODO_Y, todo, 32, 32, GxEPD_BLACK);
        // 绘制顶部栏
        display.drawLine(AREA_TOPBAR_X, AREA_TOPBAR_Y + AREA_TOPBAR_H, AREA_TOPBAR_X + AREA_TOPBAR_W - 1, AREA_TOPBAR_Y + AREA_TOPBAR_H, GxEPD_BLACK);
        display.drawLine(AREA_TODO_X, AREA_TODO_Y, AREA_TODO_X, AREA_TODO_Y + AREA_TODO_H - 1, GxEPD_BLACK);
        display_topbar(topbar);
        // 绘制时间
        char str[6];
        snprintf(str, sizeof(str), "%02d:%02d", 
                display_main_data->new_timeinfo.tm_hour, 
                display_main_data->new_timeinfo.tm_min);

        u8g2_epd.setFont(u8g2_mfxuanren_36_tr);
        int16_t str_w = u8g2_epd.getUTF8Width(str);
        int16_t ascent = u8g2_epd.getFontAscent();
        int16_t descent = u8g2_epd.getFontDescent();
        int16_t font_h = ascent - descent;
        int16_t x = 0 + (175 - str_w) / 2;
        int16_t y = 27 + (55 - font_h) / 2 + ascent;
        u8g2_epd.drawUTF8(x, y, str);

        //日期不一致时更新日期
        char date_str[20];
        strftime(date_str, 20, "%a,%d %b %Y", &display_main_data->new_timeinfo);
        text14(date_str, AREA_DATA_X + 30, AREA_DATA_Y);

        // 绘制温湿度
        char humi_str[20];
        char temp_str[20];
        snprintf(humi_str, sizeof(humi_str), "湿度:%.1f%%", display_main_data->humi);
        snprintf(temp_str, sizeof(temp_str), "温度:%.1fC", display_main_data->temp);
        text14(temp_str, AREA_DATA_X, AREA_DATA_Y + 20);
        text14(humi_str, AREA_DATA_X, AREA_DATA_Y + 20 + 14 + 4);

        char weather_str[50];
        if (display_main_data->new_timeinfo.tm_hour >= 6 && display_main_data->new_timeinfo.tm_hour < 18) {
            snprintf(weather_str, sizeof(weather_str), "%s %d-%d", display_main_data->today.textDay.c_str(), display_main_data->today.tempMin, display_main_data->today.tempMax);
            text14(weather_str, 100, AREA_DATA_Y + 20);

            snprintf(weather_str, sizeof(weather_str), "%s %s", display_main_data->today.windDirDay, display_main_data->today.winddScaleDay);
            text14(weather_str, 100, AREA_DATA_Y + 20 + 14 + 4);

            show_weathericons(display_main_data->today.iconDay);
        }
        else {
            snprintf(weather_str, sizeof(weather_str), "%s %d-%d", display_main_data->today.textNight.c_str(), display_main_data->today.tempMin, display_main_data->today.tempMax);
            text14(weather_str, 100, AREA_DATA_Y + 20);

            snprintf(weather_str, sizeof(weather_str), "%s %s", display_main_data->today.windDirNight, display_main_data->today.winddScaleNight);
            text14(weather_str, 100, AREA_DATA_Y + 20 + 14 + 4);

            show_weathericons(display_main_data->today.iconNight);
        }

        // 绘制待办事项
        for (int i = 0; i < TODO_MAX; i++) {
            if (!display_main_data->todos[i].empty()) {
                if (i == display_main_data->selected_todo) {
                    text14(display_main_data->todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14, GxEPD_WHITE, GxEPD_BLACK);// 选中待办事项时反色显示
                }
                else {
                    text14(display_main_data->todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14);//    未选中待办事项正常显示
                }
            }
        }

    } while (display.nextPage());

    PERF_END(display_main);
    FUNC_EXIT();
}

void display_main_todo(const display_main_t *display_main_data)
{
    FUNC_ENTER();
    DEBUG_PRINT("更新TODO区域");

    display.setPartialWindow(AREA_TODO_X + 5, AREA_TODO_Y + 32, AREA_TODO_W, AREA_TODO_H - 32);

    display.firstPage();
    do {
        for (int i = 0; i < TODO_MAX; i++) {
            if (!display_main_data->todos[i].empty()) {
                VERBOSE_PRINTF("绘制TODO[%d]: %s", i, display_main_data->todos[i].c_str());
                
                if (i == display_main_data->selected_todo) {
                    text14(display_main_data->todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14, GxEPD_WHITE, GxEPD_BLACK);
                    DEBUG_PRINTF("TODO[%d] 反色显示（已选中）", i);
                }
                else {
                    text14(display_main_data->todos[i].c_str(), AREA_TODO_X + 5, AREA_TODO_Y + 32 + i * 14);
                }
            }
        }
    } while (display.nextPage());

    partial_count++;
    VERBOSE_PRINTF("TODO区域刷新完成 - partial_count: %d", partial_count);
    FUNC_EXIT();
}

void show_weathericons(int weather_code)
{
    FUNC_ENTER();
    DEBUG_PRINTF("显示天气图标，代码: %d", weather_code);
    
    display.drawInvertedBitmap(68, AREA_DATA_Y + 14 + 8, getWeatherIcon(weather_code), 32, 32, GxEPD_BLACK);
    
    VERBOSE_PRINTF("天气图标绘制完成，位置: (68, %d)", AREA_DATA_Y + 14 + 8);
    FUNC_EXIT();
}