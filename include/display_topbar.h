//
// Created by m on 2025/7/13.
//

#ifndef DISPLAY_TOPBAR_H
#define DISPLAY_TOPBAR_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "text_draw.h"
#include <GxEPD2_BW.h>
#include "front/u8g2_mfxuanren_36_tr.h"
#include "bitmaps/Bitmaps128x296.h"
#include <string>
#include <time.h>
#include "main.h"
#include <U8g2_for_Adafruit_GFX.h>
#include "image/image.h"
#include "image/weather_icons.h"

typedef enum {
    BATTERY_25,
    BATTERY_50,
    BATTERY_75,
    BATTERY_100,
    BATTERY_CHARGING,
}battery_status_t;

typedef struct {
    String message;
    tm new_timeinfo;
    battery_status_t bat_status;
    _Bool wifi_connected;
}display_topbar_t;

void display_topbar(const display_topbar_t *topbar);
void display_message(const String message);


#endif //DISPLAY_TOPBAR_H
