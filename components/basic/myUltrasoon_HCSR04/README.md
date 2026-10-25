# myUltrasoon_HCSR04

Functies om een afstand te meten met de ultrasoon sensor HC-SR04.
Dit component is ook bedoeld om uit te leggen hoe je **tijd meet** met een timer.

## Het board

![Het board met de HC-SR04](images/board.png)

> Plaats je foto van het board als `images/board.png` in deze map.

## Aansluitingen

| HC-SR04 | ESP32-S3 | Opmerking |
|---|---|---|
| VCC | 5 V | de sensor werkt op 5 V |
| GND | GND | |
| TRIG | GPIO 4 (`MB_PWM_PIN`) | uitgang van de ESP32 |
| ECHO | GPIO 5 (`MB_INT_PIN`) | **via een spanningsdeler** |

De pinnen zijn vrij te kiezen, ze worden meegegeven aan de setup-functie.
Het voorbeeld gebruikt GPIO 4 en 5.

**Let op:** ECHO geeft 5 V, maar de ESP32-S3 verdraagt maximaal 3,3 V.
Gebruik een spanningsdeler, bv. ECHO → 1 kΩ → ESP32-pin → 2 kΩ → GND, of een level shifter.

## Hoe werkt de sensor

1. De ESP32 zet **TRIG** minstens 10 µs hoog.
2. De sensor stuurt een ultrasoon pakket uit.
3. **ECHO** gaat hoog en blijft hoog tot de echo terugkomt.
4. De duur van die ECHO-puls is de tijd die het geluid nodig had voor heen en terug.

`afstand (cm) = echotijd (µs) / 58`

Het bereik is ongeveer 2 cm tot 4 m, en er is minstens 60 ms nodig tussen twee metingen.

## Hoe werkt het component

Er zijn twee manieren om te meten. Elke manier heeft een eigen setup.
Alleen de setup krijgt de pinnen mee.

| Functie | Uitleg |
|---|---|
| `myUltrasoon_singleSetup(trig, echo)` | Stelt de pinnen in voor losse metingen |
| `myUltrasoon_singleReadUS()` | Doet een meting en geeft de echotijd in µs |
| `myUltrasoon_singleReadCM()` | Doet een meting en geeft de afstand in cm |
| `myUltrasoon_continueSetup(trig, echo)` | Stelt de pinnen in en start het continu meten |
| `myUltrasoon_multiReadUS()` | Geeft de laatste echotijd in µs |
| `myUltrasoon_multiReadCM()` | Geeft de laatste afstand in cm |

Alle lees-functies geven `-1` bij een fout, een timeout, een ontbrekende setup,
of (bij `multi`) als er nog geen geldige meting is.

### Single: een meting op aanvraag

De functie stuurt de trigger en wacht dan (blokkerend) tot ECHO hoog en daarna weer laag is.
De tijd meet ze met `esp_timer_get_time()`: twee keer lezen en het verschil nemen.
Als de echo niet binnen 25 ms terug is, krijg je `-1`.

### Multi: continu op de achtergrond

* Een periodieke `esp_timer` stuurt om de 60 ms een trigger.
* Een interrupt op beide flanken van ECHO onthoudt de starttijd en berekent de duur.
* De read-functies blokkeren niet en geven meteen de laatste meting.

### Instellingen

Bovenaan [myUltrasoon_HCSR04.h](include/myUltrasoon_HCSR04.h) staan de interne instellingen:

| Instelling | Standaard | Uitleg |
|---|---|---|
| `MYULTRASOON_TRIG_US` | 10 | lengte van de triggerpuls in µs |
| `MYULTRASOON_TIMEOUT_US` | 25000 | maximale echotijd, ongeveer 4 m |
| `MYULTRASOON_PERIOD_MS` | 60 | tijd tussen twee metingen in de multi-modus |
| `MYULTRASOON_US_PER_CM` | 58 | omrekening van µs naar cm |

Het component ondersteunt één sensor tegelijk. Roep je een andere setup op,
dan stopt de vorige modus en geven de read-functies van die modus `-1`.

## Tijd meten met een timer

| | Doel | Start nodig? | Handler nodig? |
|---|---|---|---|
| `esp_timer_get_time()` | tijd lezen (stopwatch) | nee | nee |
| `esp_timer_create()` + `esp_timer_start_periodic()` | iets op een tijdstip laten uitvoeren (wekker) | ja | ja |

`esp_timer_get_time()` leest een teller in µs die al loopt sinds het opstarten.

Er zijn ook andere timers:

| | `esp_timer` | `gptimer` |
|---|---|---|
| Niveau | software-laag boven een hardware-timer | directe hardware-timer |
| Aantal | heel veel tegelijk | 2 per timergroep (ESP32-S3: 4) |
| Resolutie | 1 µs | tot 80 MHz (12,5 ns) |
| Callback | aparte task, enkele tot tientallen µs jitter | in de ISR, zeer nauwkeurig |
| Gebruik | timeouts, periodieke taken, tijdstempels | nauwkeurige tijdsmeting |

De `gptimer` heeft zelf geen capture-ingang. Hardware-capture zit in de **MCPWM**-module.
Een uitwerking met `gptimer` staat in comment onderaan
[myUltrasoon_HCSR04.c](myUltrasoon_HCSR04.c).

## Voorbeeld

```c
#define TRIG 4
#define ECHO 5

myUltrasoon_continueSetup(TRIG, ECHO);

while (1) {
    int afstand = myUltrasoon_multiReadCM();
    ESP_LOGI(TAG, "de afstand is %d", afstand);
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

Een volledig voorbeeld staat in de map [example](example).
