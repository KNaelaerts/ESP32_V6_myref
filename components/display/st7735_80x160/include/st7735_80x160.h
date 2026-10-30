#pragma once

#include "esp_err.h"
#include "lvgl.h"

#define ST7735_H_RES 80
#define ST7735_V_RES 160

#define ST7735_PIN_CLK  12
#define ST7735_PIN_DATA 10
#define ST7735_PIN_RST  7
#define ST7735_PIN_DC   5
#define ST7735_PIN_CS   13
#define ST7735_PIN_BLK  4

#define ST7735_SPI_CLOCK_HZ 20000000

// Offset van het zichtbare gebied (26/1 voor de meeste 0.96" 80x160 panelen)
#define ST7735_GAP_X 26
#define ST7735_GAP_Y 1

#define ST7735_INVERT_COLORS 1

// Backlight (PWM via LEDC)
#define ST7735_BLK_PWM_HZ        5000
#define ST7735_BLK_ACTIVE_HIGH   1     // 0 als de backlight actief laag is (bv. PNP-transistor)
#define ST7735_BLK_DEFAULT_PCT   100

// Initializes SPI, panel, backlight and LVGL.
esp_err_t st7735_80x160_init(lv_display_t **out_disp);

// Helderheid van de backlight in procent (0-100).
esp_err_t st7735_80x160_set_brightness(uint8_t percent);
