#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "st7735_80x160.h"
#include "esp_lvgl_port.h"
#include "ui.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_ERROR_CHECK(st7735_80x160_init(NULL));
    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "display test gestart");

    while (1) {
        lvgl_port_lock(0);
        ui_tick();
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
