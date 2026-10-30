file(GLOB_RECURSE UI_SRCS "${CMAKE_CURRENT_LIST_DIR}/ui/*.c")

idf_component_register(SRCS "st7789_240x240.c" ${UI_SRCS}
                    INCLUDE_DIRS "include" "ui" "compat"
                    REQUIRES esp_lcd lvgl__lvgl espressif__esp_lvgl_port
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi esp_driver_ledc)
