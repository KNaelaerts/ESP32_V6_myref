# st7735_80x160

ESP-IDF component voor een ST7735 TFT van 80x160 pixels (SPI) met LVGL 9. De UI maak je met EEZ Studio of SquareLine Studio.

![Het scherm](docs/scherm.jpg)

Pinnen van het scherm: CLK, DATA (MOSI), RST, DC, CS en BLK.

## Structuur

```
st7735_80x160/
├── CMakeLists.txt           # compileert st7735_80x160.c + alle .c onder ui/
├── idf_component.yml        # haalt de driver, lvgl en esp_lvgl_port van de registry
├── include/st7735_80x160.h  # pinnen, SPI-klok, offset, inversie (#define)
├── st7735_80x160.c          # st7735_80x160_init(): SPI + paneel + LVGL
├── compat/lvgl/lvgl.h       # laat #include <lvgl/lvgl.h> (EEZ) werken in ESP-IDF
├── docs/scherm.jpg          # foto voor in deze README
├── <project>.eez-project    # EEZ-project (of <project>.spj voor SquareLine)
└── ui/                      # hier komt de gegenereerde code terecht
```

Het UI-project staat in de basismap van de component. De gegenereerde code gaat naar `ui/`, en alleen die map wordt gecompileerd.

## Instellingen van het scherm

Bovenaan [include/st7735_80x160.h](include/st7735_80x160.h) als `#define`s:

| Instelling             | Standaard | Betekenis                              |
|------------------------|-----------|----------------------------------------|
| `ST7735_PIN_CLK`       | 12        | SPI-klok (SCL)                         |
| `ST7735_PIN_DATA`      | 11        | SPI-data (SDA/MOSI)                    |
| `ST7735_PIN_RST`       | 10        | Reset                                  |
| `ST7735_PIN_DC`        | 9         | Data/commando                          |
| `ST7735_PIN_CS`        | 8         | Chip select                            |
| `ST7735_PIN_BLK`       | 7         | Backlight                              |
| `ST7735_SPI_CLOCK_HZ`  | 20000000  | SPI-snelheid                           |
| `ST7735_GAP_X/Y`       | 26 / 1    | Offset van het zichtbare gebied        |
| `ST7735_INVERT_COLORS` | 1         | Kleurinversie (1 = aan, 0 = uit)       |

Staat het beeld verschoven of zijn de kleuren omgekeerd, pas dan de offset of de inversie aan.

## Afhankelijkheden

Staan in `idf_component.yml` en worden bij de eerste build automatisch gedownload naar `managed_components/`:

- `waveshare/esp_lcd_st7735`
- `lvgl/lvgl` (versie 9)
- `espressif/esp_lvgl_port`

## Gebruik in main

`main/CMakeLists.txt` moet `st7735_80x160` in `REQUIRES` hebben.

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lvgl_port.h"
#include "st7735_80x160.h"
#include "ui.h"

void app_main(void)
{
    ESP_ERROR_CHECK(st7735_80x160_init(NULL));

    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();

    while (1) {
        lvgl_port_lock(0);
        ui_tick();            // alleen bij EEZ, zie hieronder
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

LVGL draait in een eigen taak. Elke LVGL-aanroep vanuit je eigen code hoort daarom tussen `lvgl_port_lock()` en `lvgl_port_unlock()`.

## UI maken met EEZ Studio

### Eenmalig instellen

1. Zet het `.eez-project` in de basismap van de component (naast `CMakeLists.txt`).
2. Open het project en kies als projecttype **LVGL**, versie **9.x**. Zet de schermgrootte op 80 breed en 160 hoog.
3. Ga naar de build-instellingen (**Settings → Build → Files**) en zet de **Destination folder** op `ui`. Dat pad is relatief aan het projectbestand.
4. Laat **LVGL include** op `lvgl/lvgl.h` staan. Dat werkt dankzij `compat/lvgl/lvgl.h`.

### Werkwijze

1. Pas het scherm aan en kies **Build**. De bestanden in `ui/` worden overschreven.
2. Voer in VS Code **ESP-IDF: Reconfigure** uit als er bestanden bijgekomen of verdwenen zijn.
3. Build en flash.

EEZ genereert `ui_init()` en `ui_tick()`. De tick is nodig voor variabelen, bindingen en animaties. Je eigen code die de variabelen invult hoort in `vars.h`/`vars.c` volgens de EEZ-documentatie, niet in de gegenereerde bestanden.

## UI maken met SquareLine Studio

### Eenmalig instellen

1. Zet het `.spj`-bestand in de basismap van de component.
2. Maak het project aan met een **custom** scherm van 80 x 160, kleurdiepte **16 bit**, en LVGL **9.x**.
3. Ga naar **Project Settings** en vul in:
   - **LVGL Include Path**: `lvgl.h`
   - **Project Export Root**: de basismap van de component
   - **UI Files Export Path**: `ui`
4. Voer **Export → Export UI Files** uit.

### Werkwijze

1. Pas het scherm aan en kies **Export UI Files**.
2. Voer **ESP-IDF: Reconfigure** uit als er bestanden bijgekomen of verdwenen zijn.
3. Build en flash.

SquareLine genereert wel `ui_init()` maar geen `ui_tick()`. Haal dus de regel `ui_tick();` weg uit de lus in `main`. De lus zelf mag blijven of weg.

SquareLine maakt in `ui/` ook submappen aan (bijvoorbeeld `screens/`, `images/`, `fonts/`). De CMake van deze component compileert die mee.

## Gegenereerde code

Pas bestanden in `ui/` nooit met de hand aan, want ze worden bij de volgende export overschreven. Eigen code zet je in `main` of in eigen bestanden buiten `ui/`.

## Ontbrekende LVGL-opties

Gebruikt je UI een lettertype of widget dat standaard uitgeschakeld is, dan geeft de build een melding zoals `undeclared` of `implicit declaration of function`. Zet de bijhorende optie dan aan:

- in `menuconfig` onder **Component config → LVGL configuration**, of
- in `sdkconfig.defaults`, zodat de instelling bewaard blijft.

Voorbeelden:

```
CONFIG_LV_FONT_MONTSERRAT_12=y
CONFIG_LV_USE_QRCODE=y
```

Een wijziging in `sdkconfig.defaults` werkt alleen door als `sdkconfig` die waarde nog niet heeft. Pas ze dus ook in menuconfig aan, of verwijder `sdkconfig` en voer Reconfigure uit.
