# st7735_80x160

ESP-IDF component voor een ST7735 TFT van 80x160 pixels (SPI) met LVGL 9 en een UI uit EEZ Studio.

Pinnen: CLK, DATA (MOSI), RST, DC, CS en BLK.

## Structuur

```
st7735_80x160/
├── CMakeLists.txt        # compileert st7735_80x160.c + alle ui/*.c
├── idf_component.yml     # haalt driver, lvgl en esp_lvgl_port van de registry
├── Kconfig               # pinnen, SPI-klok, offset, inversie (menuconfig)
├── include/st7735_80x160.h
├── st7735_80x160.c       # st7735_80x160_init(): SPI + paneel + LVGL
├── compat/lvgl/lvgl.h    # laat #include <lvgl/lvgl.h> (EEZ) werken in ESP-IDF
└── ui/                   # door EEZ gegenereerde code (ui.c, screens.c, ...)
    └── testje/           # het EEZ-project zelf (*.eez-project)
```

## Gebruik in main

```c
#include "esp_lvgl_port.h"
#include "st7735_80x160.h"
#include "ui.h"

ESP_ERROR_CHECK(st7735_80x160_init(NULL));
lvgl_port_lock(0);
ui_init();
lvgl_port_unlock();

while (1) {
    lvgl_port_lock(0);
    ui_tick();
    lvgl_port_unlock();
    vTaskDelay(pdMS_TO_TICKS(20));
}
```

`main/CMakeLists.txt` moet `st7735_80x160` in `REQUIRES` hebben.
Elke LVGL-aanroep buiten de LVGL-taak moet tussen `lvgl_port_lock()` en `lvgl_port_unlock()`.

## Werken met EEZ Studio

### Waar zet je de UI?

Het project in `ui/testje/` laten staan en de gegenereerde code **in `ui/`** laten schrijven werkt het best:

- De CMake pakt alleen `ui/*.c` (niet recursief). De standaardmap `testje/src/ui` wordt dus **niet** gecompileerd.
- Het EEZ-project (`.eez-project`) staat in een submap, dus EEZ kan de gegenereerde bestanden niet per ongeluk in je eigen code mengen.
- Je hoeft nooit bestanden te kopiëren na een export.

De UI uit de component halen kan ook, maar dan moet de component `ui_init()` van buiten krijgen. Voor één scherm met vaste pinnen is het eenvoudiger om de UI erin te houden.

### Eenmalig instellen in EEZ Studio

1. Open `ui/testje/testje.eez-project`.
2. Ga naar **Settings → Build → Files** (of de build-configuratie) en zet **Destination folder** op `..`. Dat is `ui/`, relatief aan het projectbestand. Nu staat er geen `src\ui` meer.
3. Controleer dat **LVGL include** op `lvgl/lvgl.h` staat. Dat werkt dankzij `compat/lvgl/lvgl.h`.
4. Zet bij het project de LVGL-versie op 9.x. De registry-versie in `idf_component.yml` is `^9`.
5. Schermgrootte: 80 breed, 160 hoog (portret).
6. Verwijder de oude gegenereerde bestanden in `testje/src/ui` zodra de export naar `ui/` werkt.

### Werkwijze

1. Pas het scherm aan in EEZ Studio.
2. **Build** in EEZ Studio. De bestanden worden in `ui/` overschreven.
3. Voer **Reconfigure** uit als er `.c` bestanden bijgekomen of verdwenen zijn in `ui/`, anders ziet CMake ze niet. Daarna Build in ESP-IDF.

Pas gegenereerde bestanden nooit met de hand aan. Ze worden overschreven. Eigen code zet je in aparte bestanden, bijvoorbeeld in `main` of in eigen bestanden naast `ui/`.

## LVGL-instellingen

De gegenereerde code gebruikt bepaalde LVGL-functies die standaard uit staan. Zet ze aan in `menuconfig` onder **Component config → LVG (Light and Versatile Graphics Library)** of via `sdkconfig.defaults`. Dan blijven ze behouden na een nieuwe configuratie.

Wat nu nodig is (staat al in `sdkconfig.defaults`):

```
CONFIG_LV_FONT_MONTSERRAT_8=y
CONFIG_LV_FONT_MONTSERRAT_10=y
CONFIG_LV_FONT_MONTSERRAT_12=y
CONFIG_LV_USE_QRCODE=y
```

Regel: gebruikt je EEZ-scherm een lettertype of widget en je krijgt `undeclared` of `implicit declaration of function`, dan staat de bijhorende optie uit.

| Fout                                           | Oplossing                                                         |
|------------------------------------------------|-------------------------------------------------------------------|
| `lv_font_montserrat_XX undeclared`             | `CONFIG_LV_FONT_MONTSERRAT_XX=y`                                  |
| `lv_qrcode_*` implicit declaration             | `CONFIG_LV_USE_QRCODE=y`                                          |
| `lv_chart_*`, `lv_table_*`, `lv_calendar_*`... | `CONFIG_LV_USE_<WIDGET>=y`                                        |
| `lvgl/lvgl.h: No such file`                    | `compat` ontbreekt in `INCLUDE_DIRS` of lvgl staat niet in `REQUIRES` |

Na een wijziging in `sdkconfig.defaults` werken deze alleen door als `sdkconfig` nog niet bestaat of dezelfde waarde mist. Pas ze dus ook in menuconfig aan, of verwijder `sdkconfig` en voer Reconfigure uit.

Let op het RAM-gebruik: elk extra lettertype en elke extra widget kost flash.

## Menuconfig-opties van de component

Onder **ST7735 80x160 display**:

| Optie                | Standaard | Betekenis                              |
|----------------------|-----------|----------------------------------------|
| `ST7735_PIN_CLK`     | 12        | SPI-klok (SCL)                         |
| `ST7735_PIN_DATA`    | 11        | SPI-data (SDA/MOSI)                    |
| `ST7735_PIN_RST`     | 10        | Reset                                  |
| `ST7735_PIN_DC`      | 9         | Data/commando                          |
| `ST7735_PIN_CS`      | 8         | Chip select                            |
| `ST7735_PIN_BLK`     | 7         | Backlight                              |
| `ST7735_SPI_CLOCK_HZ`| 20000000  | SPI-snelheid                           |
| `ST7735_GAP_X/Y`     | 26 / 1    | Offset van het zichtbare gebied        |
| `ST7735_INVERT_COLORS` | aan     | Kleurinversie                          |

Beeld verschoven of kleuren omgekeerd: pas de offset of de inversie aan.

## Afhankelijkheden (`idf_component.yml`)

- `waveshare/esp_lcd_st7735` ^2.0.0
- `lvgl/lvgl` ^9
- `espressif/esp_lvgl_port` ^2.6.0

Bij de eerste build downloadt de component manager die naar `managed_components/`. Commit `dependencies.lock` mee, maar niet `managed_components/` of `build/`.

## Veelvoorkomende problemen

- **Leeg of wit scherm**: controleer de pinnen en de backlight (BLK).
- **Verkeerde kleuren**: zet `ST7735_INVERT_COLORS` om, of pas `rgb_ele_order` aan in `st7735_80x160.c`.
- **Beeld verschoven of een rand**: pas `ST7735_GAP_X/Y` aan.
- **Animaties of variabelen werken niet**: roep `ui_tick()` periodiek aan (zie hierboven).
