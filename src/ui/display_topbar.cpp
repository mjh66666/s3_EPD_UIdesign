//
// Created by m on 2025/7/13.
//
#include  "display_topbar.h"

// 保证和 display.cpp 里的宏一致
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

extern U8G2_FOR_ADAFRUIT_GFX u8g2_epd;
extern GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

// 区域定义
#define AREA_TOPBAR_X      0
#define AREA_TOPBAR_Y      0
#define AREA_TOPBAR_W      296
#define AREA_TOPBAR_H      18

void display_topbar(const display_topbar_t *topbar)
{
    if (topbar->wifi_connected == true) {
        display.drawInvertedBitmap(AREA_TOPBAR_W-18,0,epd_bitmap_wifi_status_allArray[0],16, 16, GxEPD_BLACK);
    }
    else {
        display.drawInvertedBitmap(AREA_TOPBAR_W-18,0,epd_bitmap_wifi_status_allArray[1],16, 16, GxEPD_BLACK);
    }

    switch (topbar->bat_status) {
        case BATTERY_25:
            display.drawInvertedBitmap(AREA_TOPBAR_W-18-18, 0, batteryallArray[0], 16, 16, GxEPD_BLACK);
            break;
        case BATTERY_50:display.drawInvertedBitmap(AREA_TOPBAR_W-18-18, 0, batteryallArray[1], 16, 16, GxEPD_BLACK);
        case BATTERY_75:display.drawInvertedBitmap(AREA_TOPBAR_W-18-18, 0, batteryallArray[2], 16, 16, GxEPD_BLACK);
        case BATTERY_100:display.drawInvertedBitmap(AREA_TOPBAR_W-18-18, 0, batteryallArray[3], 16, 16, GxEPD_BLACK);
        default: ;
    }

    char str[6];
    snprintf(str, sizeof(str), "%02d:%02d",
            topbar->new_timeinfo.tm_hour,
            topbar->new_timeinfo.tm_min);
    int timeWidth = u8g2_epd.getUTF8Width(str);
    text14(str,AREA_TOPBAR_W-18-18-timeWidth-2,0,GxEPD_BLACK);

    text14(topbar->message.c_str(),AREA_TOPBAR_X,AREA_TOPBAR_Y,GxEPD_BLACK);
}

void display_message(const String message)
{
    display.setPartialWindow(0, 0, display.width(), display.height());
    do {
        text14(message.c_str(),AREA_TOPBAR_X,AREA_TOPBAR_Y,GxEPD_BLACK);
    }while (display.nextPage());
}





