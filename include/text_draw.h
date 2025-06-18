#pragma once

#include <stdint.h>
#include <GxEPD2_BW.h>

void text14(const char *str, int16_t x, int16_t y, uint16_t fg_color = GxEPD_BLACK, uint16_t bg_color = GxEPD_WHITE);
void text36(const char *str, int16_t x, int16_t y, uint16_t fg_color = GxEPD_BLACK, uint16_t bg_color = GxEPD_WHITE);