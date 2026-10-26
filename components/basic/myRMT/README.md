# myRMT

Functies om tijdgevoelige signalen te versturen en te ontvangen met de **RMT**-module
(Remote Control Transceiver) van de ESP32-S3. De hardware maakt of meet hele reeksen pulsen,
zonder dat de CPU elke bit moet timen. Twee voorbeelden: een **WS2812B-led** en een **infrarood ontvanger**.

## Het board

![Het board met WS2812B en IR ontvanger](images/board.png)

> Plaats je foto van het board als `images/board.png` in deze map.

## Aansluitingen

De pinnen zijn vrij te kiezen, ze worden meegegeven aan de setup-functie.

| Onderdeel | Aansluiting | Opmerking |
|---|---|---|
| WS2812B | DIN naar een GPIO-pin (output) | voeding 5 V, GND gemeenschappelijk |
| IR ontvanger (bv. TSOP) | OUT naar een GPIO-pin (input) | voeding volgens de datasheet (meestal 3,3 V) |

**Let op:** een WS2812B verwacht voor DIN een hoog niveau van ongeveer 0,7 x VDD. Op 5 V voeding is dat 3,5 V,
en de 3,3 V van de ESP32 is dan grenswaarde. Werkt het onbetrouwbaar, gebruik dan een level shifter
of voed de led op 3,3 V. Een IR ontvanger trekt zijn uitgang **laag** tijdens een puls.

## Hoe werkt RMT

Een RMT-symbool (`rmt_symbol_word_t`) beschrijft twee opeenvolgende niveaus:
`level0` gedurende `duration0` ticks, daarna `level1` gedurende `duration1` ticks.
Hoe lang een tick duurt, bepaalt de resolutie.

* **Output:** je geeft een lijst symbolen en de hardware produceert die golfvorm.
* **Input:** de hardware meet de lengte van de hoge en lage pulsen en legt ze vast in symbolen.
  Een frame is afgelopen als de lijn lang genoeg stil is.

## Functies

### Algemeen (symbolen)

| Functie | Uitleg |
|---|---|
| `myRMT_OutputSetup(pin)` | Stelt de pin in als RMT-uitgang |
| `myRMT_Output_WriteBuffer(symbols, count)` | Verstuurt de symbolen en wacht tot ze verzonden zijn. 0 = ok, -1 = fout |
| `myRMT_InputSetup(pin)` | Stelt de pin in als RMT-ingang en start de ontvangst |
| `myRMT_InputRead(symbols, max_count)` | Kopieert het laatst ontvangen frame. Geeft het aantal symbolen, 0 = niets, -1 = fout |

### WS2812B

| Functie | Uitleg |
|---|---|
| `myRMT_OutputWS2812(r, g, b)` | Geeft de led de kleur r, g, b (elk 0..255). 0 = ok, -1 = fout |

De functie zet de waarden om naar 24 bits in de volgorde **G, R, B** (meest significante bit eerst),
en sluit af met een reset van 50 us laag. Een bit is een puls met een bepaalde duur:

| Bit | Hoog | Laag |
|---|---|---|
| 0 | 0,4 us (4 ticks) | 0,8 us (8 ticks) |
| 1 | 0,8 us (8 ticks) | 0,4 us (4 ticks) |

Bij een resolutie van 10 MHz is 1 tick = 100 ns. Die tijden staan in de header als `MYRMT_WS_*`.

### Infrarood

| Functie | Uitleg |
|---|---|
| `myRMT_InputIR(type, &code)` | Verwerkt een nieuw frame en geeft de laatste geldige code. 1 = er is een code, 0 = nog geen, -1 = fout |

Het type is `MYRMT_IR_NEC` of `MYRMT_IR_SONY`. De code is de ruwe waarde, met het minst significante bit eerst:

* **NEC:** 32 bits: adres, ~adres, commando, ~commando. Een startpuls van 9 ms laag en 4,5 ms hoog gaat eraan vooraf.
  Houd je een knop ingedrukt, dan komen er herhaal-frames. Die laten de laatste code ongewijzigd.
* **SONY (SIRC):** 12, 15 of 20 bits. De lengte van de lage puls bepaalt de bit.

De duur van elke puls wordt gecontroleerd met 30% tolerantie. Ongeldige frames worden genegeerd.

### Instellingen

Bovenaan [myRMT.h](include/myRMT.h) staan de interne instellingen:

| Instelling | Standaard | Uitleg |
|---|---|---|
| `MYRMT_TX_RESOLUTION_HZ` | 10 MHz | resolutie van de output, 1 tick = 100 ns |
| `MYRMT_TX_QUEUE_DEPTH` | 4 | aantal verzendingen dat tegelijk in de wachtrij past |
| `MYRMT_RX_RESOLUTION_HZ` | 1 MHz | resolutie van de input, 1 tick = 1 us |
| `MYRMT_RX_MIN_PULSE_NS` | 1250 | kortere pulsen worden als ruis genegeerd |
| `MYRMT_RX_IDLE_NS` | 12 ms | zo lang stil = einde van het frame (langer dan de langste puls) |
| `MYRMT_RX_MAX_SYMBOLS` | 128 | maximaal aantal symbolen in een frame |
| `MYRMT_RX_READ_TIMEOUT_MS` | 0 | hoelang `myRMT_InputRead` wacht op een frame |
| `MYRMT_WS_*` | zie boven | timing van de WS2812B-bits en de reset |

Het component ondersteunt een output- en een input-kanaal.

## Voorbeelden

WS2812B:

```c
myRMT_OutputSetup(LED_PIN);

myRMT_OutputWS2812(255, 0, 0);     // rood
vTaskDelay(pdMS_TO_TICKS(500));
myRMT_OutputWS2812(0, 0, 255);     // blauw
```

Infrarood ontvanger:

```c
myRMT_InputSetup(IR_PIN);

while (1) {
    uint32_t code;
    if (myRMT_InputIR(MYRMT_IR_NEC, &code) == 1) {
        ESP_LOGI(TAG, "laatste code: 0x%08lX", (unsigned long)code);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

## Een eigen signaal maken

Wil je een ander protocol, dan kan je zelf symbolen opstellen:

```c
rmt_symbol_word_t bit1 = { .level0 = 1, .duration0 = 8,    // 8 ticks hoog
                           .level1 = 0, .duration1 = 4 };  // 4 ticks laag
myRMT_Output_WriteBuffer(&bit1, 1);
```
