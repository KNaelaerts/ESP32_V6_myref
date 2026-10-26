# myUART

Functies om tekst te versturen en te ontvangen via een **UART** van de ESP32-S3, bijvoorbeeld
naar een andere microcontroller of een USB-naar-serieel adapter.

## Het board

![Het board met de UART-aansluitingen](images/board.png)

> Plaats je foto van het board als `images/board.png` in deze map.

## Aansluitingen

De standaardwaarden staan in [myUART.h](include/myUART.h) en komen overeen met de pinnen van het board
(zie [myESP_S3_mini_board.h](../../boards/myESP_S3_mini_board/include/myESP_S3_mini_board.h)):

| Signaal | ESP32-pin | Define | Verbinden met |
|---|---|---|---|
| TX | GPIO 17 | `MB_TX_PIN` | RX van de andere kant |
| RX | GPIO 18 | `MB_RX_PIN` | TX van de andere kant |
| GND | GND | | GND van de andere kant |

TX en RX worden dus **gekruist** aangesloten. De signaalniveaus zijn 3,3 V.
Gebruik je een 5 V-apparaat, plaats dan een level shifter.

## Belangrijk: zet de console uit

ESP-IDF stuurt standaard zijn console (`printf`, `ESP_LOGI`, ...) naar een UART. `myUART` gebruikt
`UART_NUM_2` (zie `uart_num` in de header). Zolang de console op een andere UART staat dan
`myUART`, is er geen conflict. Zodra ze dezelfde UART gebruiken, (bijvoorbeeld als je `uart_num` op
`UART_NUM_0` zet, of de console op een eigen UART legt), botsen ze: de tekst loopt door elkaar en de
driver kan niet twee keer geïnstalleerd worden.

Wil je zeker zijn dat de console niet in de weg zit, zet hem dan uit in `idf.py menuconfig`
(of via de ESP-IDF extensie: *ESP-IDF: SDK Configuration Editor*):

1. **Component config → ESP System Settings → Channel for console output**
   kies `None` (`CONFIG_ESP_CONSOLE_NONE`)
2. **Component config → ESP System Settings → Channel for console secondary output**
   kies `No secondary console` (`CONFIG_ESP_CONSOLE_SECONDARY_NONE`)

Bouw daarna het project opnieuw. Let op: `sdkconfig` staat in `.gitignore`, dus je moet dit op
elke computer opnieuw instellen. Zonder console zie je geen ESP-IDF logberichten meer. Gebruik
`myUART_printf` om tekst te sturen.

Gebruik je `myUART` samen met [myUSB](../myUSB/README.md), dan staat de console toch al uit en
zijn beide vrij.

## Hoe werkt het

| Functie | Uitleg |
|---|---|
| `myUART_setup(baud)` | Stelt de UART in (8 databits, geen pariteit, 1 stopbit), koppelt de pinnen en installeert de driver |
| `myUART_printf(fmt, ...)` | Werkt zoals `printf` en stuurt de tekst via de UART (maximaal 255 tekens per oproep) |
| `myUART_getmsg(buffer)` | Leest tekens tot een regeleinde (`\n` of `\r`) en zet ze in `buffer` |

`myUART_getmsg` geeft het aantal tekens terug **inclusief de afsluitende 0** als er een volledige regel is.
Is er nog geen volledige regel, dan geeft ze `0`. Het regeleinde zelf wordt vervangen door een 0, zodat
`buffer` een gewone C-string is. Zorg dat `buffer` groot genoeg is voor de langste regel.

De driver gebruikt een ontvangstbuffer en een zendbuffer van elk 1024 bytes.

### Instellingen

Bovenaan [myUART.h](include/myUART.h) staan de interne instellingen:

| Instelling | Standaard | Uitleg |
|---|---|---|
| `BLOCKING` | `true` | `true`: `myUART_getmsg` wacht tot er tekens zijn. `false`: ze wacht niet |
| `uart_num` | `UART_NUM_2` | welke UART gebruikt wordt |
| `TX_pin` | 17 | pin voor TX |
| `RX_pin` | 18 | pin voor RX |

## Voorbeeld

```c
myUART_setup(115200);

char buffer[64];
while (1) {
    if (myUART_getmsg(buffer) > 0) {
        myUART_printf("ontvangen: %s\n", buffer);
    }
}
```
