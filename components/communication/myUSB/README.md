# myUSB

Functies om tekst te versturen en te ontvangen via de ingebouwde **USB Serial/JTAG**-poort van de
ESP32-S3. Je hebt hiervoor geen extra draden of een USB-naar-serieel chip nodig: de gewone USB-kabel
naar de computer volstaat.

## Het board

![Het board met de USB-poort](images/board.png)

> Plaats je foto van het board als `images/board.png` in deze map.

## Aansluitingen

| Onderdeel | Aansluiting |
|---|---|
| USB-poort van de ESP32-S3 (USB Serial/JTAG) | USB-kabel naar de computer |

Op de computer verschijnt een COM-poort. Open die met een seriële monitor (bv. de ESP-IDF monitor).

## Belangrijk: zet de console uit

Standaard gebruikt ESP-IDF dezelfde USB Serial/JTAG-poort voor de **console** (`printf`, `ESP_LOGI`, ...).
Als je die niet uitzet, dan strijden de console en `myUSB` om dezelfde driver en poort. Dat geeft
fouten bij het opstarten of door elkaar lopende tekst.

Zet in `idf.py menuconfig` (of via de ESP-IDF extensie: *ESP-IDF: SDK Configuration Editor*):

1. **Component config → ESP System Settings → Channel for console output**
   kies `None` (`CONFIG_ESP_CONSOLE_NONE`)
2. **Component config → ESP System Settings → Channel for console secondary output**
   kies `No secondary console` (`CONFIG_ESP_CONSOLE_SECONDARY_NONE`)

De tweede instelling is de belangrijkste: zolang daar `USB_SERIAL_JTAG` staat, installeert ESP-IDF zelf
de USB-driver en kan `myUSB_setup` die niet meer installeren.

Bouw daarna het project opnieuw. Let op: `sdkconfig` staat in `.gitignore`, dus je moet dit op
elke computer opnieuw instellen.

Een gevolg hiervan is dat je geen logberichten van ESP-IDF meer ziet op deze poort. Gebruik `myUSB_printf`
om tekst te sturen.

## Hoe werkt het

| Functie | Uitleg |
|---|---|
| `myUSB_setup(baud)` | Installeert de USB Serial/JTAG-driver met een ontvangst- en zendbuffer |
| `myUSB_printf(fmt, ...)` | Werkt zoals `printf` en stuurt de tekst via USB (maximaal 255 tekens per oproep) |
| `myUSB_getmsg(buffer)` | Leest tekens tot een regeleinde (`\n` of `\r`) en zet ze in `buffer` |

`myUSB_getmsg` geeft het aantal tekens terug **inclusief de afsluitende 0** als er een volledige regel is.
Is er nog geen volledige regel, dan geeft ze `0`. Het regeleinde zelf wordt vervangen door een 0, zodat
`buffer` een gewone C-string is. Zorg dat `buffer` groot genoeg is voor de langste regel.

De parameter `baud` van `myUSB_setup` wordt niet gebruikt: USB Serial/JTAG heeft geen baudrate.
Hij staat er om de functie gelijk te houden aan `myUART_setup`.

### Instellingen

Bovenaan [myUSB.h](include/myUSB.h) staan de interne instellingen:

| Instelling | Standaard | Uitleg |
|---|---|---|
| `BLOCKING` | `true` | `true`: `myUSB_getmsg` wacht tot er tekens zijn. `false`: ze wacht niet |
| `BUF_SIZE` | 50 | basisgrootte van de buffers, de driver gebruikt 4 x deze waarde |

## Voorbeeld

```c
myUSB_setup(115200);

char buffer[64];
while (1) {
    if (myUSB_getmsg(buffer) > 0) {
        myUSB_printf("ontvangen: %s\n", buffer);
    }
}
```
