#pragma once

#include "esp_err.h"
#include "lvgl.h"

#define ST7735_H_RES 80
#define ST7735_V_RES 160

// Initializes SPI, panel, backlight and LVGL. Pins are set in menuconfig.
esp_err_t st7735_80x160_init(lv_display_t **out_disp);
