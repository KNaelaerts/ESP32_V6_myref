#pragma once

#include "esp_err.h"
#include "lvgl.h"

#define ST7789_H_RES 240
#define ST7789_V_RES 240

// Pinnen (module: GND, VCC, SCK, SDA, RES, DC, BLK, geen CS)
#define ST7789_PIN_CLK   10  // SCK
#define ST7789_PIN_DATA 11   // SDA (MOSI)
#define ST7789_PIN_RST  12    // RES
#define ST7789_PIN_DC   13
#define ST7789_PIN_CS   -1   // niet aanwezig op deze module
#define ST7789_PIN_BLK  7

#define ST7789_SPI_CLOCK_HZ 40000000

// Modules zonder CS werken meestal in SPI-mode 3 (met CS: mode 0)
#define ST7789_SPI_MODE 3

// Offset van het zichtbare gebied (0/0 voor de meeste 240x240 modules)
#define ST7789_GAP_X 0
#define ST7789_GAP_Y 0

#define ST7789_INVERT_COLORS 1

// Backlight (PWM via LEDC)
#define ST7789_BLK_PWM_HZ        5000
#define ST7789_BLK_ACTIVE_HIGH   1
#define ST7789_BLK_DEFAULT_PCT   100

// Initializes SPI, panel, backlight and LVGL.
esp_err_t st7789_240x240_init(lv_display_t **out_disp);

// Helderheid van de backlight in procent (0-100).
esp_err_t st7789_240x240_set_brightness(uint8_t percent);
