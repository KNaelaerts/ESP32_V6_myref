# st7789_240x240

ESP-IDF component voor een ST7789 TFT van 240x240 pixels (SPI, zonder CS) met LVGL 9. De UI maak je met EEZ Studio of SquareLine Studio.

Modulepinnen: GND, VCC, SCK, SDA, RES, DC, BLK.

De werkwijze voor de UI (EEZ/SquareLine, `ui/`-map, `compat/`, ontbrekende LVGL-opties) is identisch aan die van de component `st7735_80x160`. Zie de [README daarvan](../st7735_80x160/README.md). Gebruik hier schermgrootte 240 x 240.

## Instellingen

Bovenaan [include/st7789_240x240.h](include/st7789_240x240.h):

| Instelling              | Standaard | Betekenis                                          |
|-------------------------|-----------|----------------------------------------------------|
| `ST7789_PIN_CLK`        | 12        | SCK                                                |
| `ST7789_PIN_DATA`       | 10        | SDA (MOSI)                                         |
| `ST7789_PIN_RST`        | 7         | RES                                                |
| `ST7789_PIN_DC`         | 5         | Data/commando                                      |
| `ST7789_PIN_CS`         | -1        | Niet aanwezig op deze module                       |
| `ST7789_PIN_BLK`        | 4         | Backlight (PWM)                                    |
| `ST7789_SPI_CLOCK_HZ`   | 40000000  | SPI-snelheid (zakt bij problemen naar 20 MHz)      |
| `ST7789_SPI_MODE`       | 3         | Modules zonder CS werken meestal in mode 3         |
| `ST7789_GAP_X/Y`        | 0 / 0     | Offset van het zichtbare gebied                    |
| `ST7789_INVERT_COLORS`  | 1         | Kleurinversie                                      |
| `ST7789_BLK_*`          |           | PWM-frequentie, polariteit en startwaarde backlight |

Helderheid wijzigen: `st7789_240x240_set_brightness(0-100)`.

## Gebruik

`st7789_240x240` toevoegen aan `REQUIRES` in `main/CMakeLists.txt`.

```c
#include "esp_lvgl_port.h"
#include "st7789_240x240.h"
#include "ui.h"

ESP_ERROR_CHECK(st7789_240x240_init(NULL));
lvgl_port_lock(0);
ui_init();
lvgl_port_unlock();
```

## Naast de st7735-component

Deze component gebruikt SPI3, LEDC-timer 1 en kanaal 1, de st7735 SPI2/timer 0/kanaal 0. Beide kunnen dus tegelijk draaien, maar dan moeten de pinnen verschillen en heeft elke component zijn eigen `ui.h`. Dat geeft een naamconflict (`ui_init`, `ui.h`); gebruik dus maar één UI tegelijk of hernoem de functies.

## Problemen

- **Wit of zwart scherm**: probeer `ST7789_SPI_MODE` 0 en 3, en verlaag de SPI-klok.
- **Kleuren omgekeerd**: zet `ST7789_INVERT_COLORS` op 0.
- **Rode en blauwe kleur verwisseld**: wissel `LCD_RGB_ELEMENT_ORDER_RGB` naar `BGR` in `st7789_240x240.c`.
