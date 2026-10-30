#include "st7735_80x160.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7735.h"
#include "esp_lvgl_port.h"

static const char *TAG = "st7735_80x160";

#define LCD_SPI_HOST SPI2_HOST
#define BLK_LEDC_MODE LEDC_LOW_SPEED_MODE
#define BLK_LEDC_TIMER LEDC_TIMER_0
#define BLK_LEDC_CHANNEL LEDC_CHANNEL_0
#define BLK_LEDC_RES LEDC_TIMER_10_BIT

esp_err_t st7735_80x160_set_brightness(uint8_t percent)
{
    if (percent > 100) {
        percent = 100;
    }
    uint32_t max_duty = (1U << BLK_LEDC_RES) - 1;
    uint32_t duty = max_duty * percent / 100;
    if (!ST7735_BLK_ACTIVE_HIGH) {
        duty = max_duty - duty;
    }
    ESP_RETURN_ON_ERROR(ledc_set_duty(BLK_LEDC_MODE, BLK_LEDC_CHANNEL, duty), TAG, "set duty");
    return ledc_update_duty(BLK_LEDC_MODE, BLK_LEDC_CHANNEL);
}

static esp_err_t backlight_init(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = BLK_LEDC_MODE,
        .duty_resolution = BLK_LEDC_RES,
        .timer_num = BLK_LEDC_TIMER,
        .freq_hz = ST7735_BLK_PWM_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "ledc timer");
    const ledc_channel_config_t ch = {
        .gpio_num = ST7735_PIN_BLK,
        .speed_mode = BLK_LEDC_MODE,
        .channel = BLK_LEDC_CHANNEL,
        .timer_sel = BLK_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&ch), TAG, "ledc channel");
    return st7735_80x160_set_brightness(0);
}

esp_err_t st7735_80x160_init(lv_display_t **out_disp)
{
    ESP_RETURN_ON_ERROR(backlight_init(), TAG, "backlight");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = ST7735_PIN_CLK,
        .mosi_io_num = ST7735_PIN_DATA,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = ST7735_H_RES * 40 * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "spi bus");

    esp_lcd_panel_io_handle_t io = NULL;
    const esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = ST7735_PIN_DC,
        .cs_gpio_num = ST7735_PIN_CS,
        .pclk_hz = ST7735_SPI_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_cfg, &io),
                        TAG, "panel io");

    esp_lcd_panel_handle_t panel = NULL;
    const esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = ST7735_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7735(io, &panel_cfg, &panel), TAG, "panel");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel), TAG, "reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel), TAG, "init");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(panel, ST7735_GAP_X, ST7735_GAP_Y), TAG, "gap");
#if ST7735_INVERT_COLORS
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(panel, true), TAG, "invert");
#endif
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel, true), TAG, "disp on");

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "lvgl port");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
        .panel_handle = panel,
        .buffer_size = ST7735_H_RES * 40,
        .double_buffer = true,
        .hres = ST7735_H_RES,
        .vres = ST7735_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        },
    };
    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
    ESP_RETURN_ON_FALSE(disp, ESP_FAIL, TAG, "add display");

    st7735_80x160_set_brightness(ST7735_BLK_DEFAULT_PCT);
    if (out_disp) {
        *out_disp = disp;
    }
    return ESP_OK;
}
