#pragma once

#include <stdint.h>
#include "driver/rmt_types.h"

//___ interne instellingen ________________________________________________________
// Deze instellingen gebruikt myRMT.c intern.
// Een RMT symbool (rmt_symbol_word_t) bestaat uit twee delen: level0 gedurende
// duration0 ticks, gevolgd door level1 gedurende duration1 ticks.

// Output: resolutie van 1 tick. 10 MHz => 1 tick = 100 ns.
// Voorbeeld WS2812B (bit 0 = 0,4 us hoog + 0,85 us laag, bit 1 = 0,8 us hoog + 0,45 us laag):
//   bit 0 = { .level0 = 1, .duration0 = 4, .level1 = 0, .duration1 = 8 }
//   bit 1 = { .level0 = 1, .duration0 = 8, .level1 = 0, .duration1 = 4 }
//   24 bits per led, volgorde G R B, meest significante bit eerst.
//   Laat tussen twee oproepen minstens 50 us laag (reset).
#define MYRMT_TX_RESOLUTION_HZ      10000000

// Aantal symbolen dat de hardware tegelijk in de wachtrij houdt (transmit)
#define MYRMT_TX_QUEUE_DEPTH        4

// Input: resolutie van 1 tick. 1 MHz => 1 tick = 1 us.
// Voorbeeld infrarood ontvanger (NEC): een frame start met 9 ms laag en 4,5 ms hoog,
// daarna komen 32 bits. Een IR ontvanger trekt zijn uitgang laag tijdens een puls.
#define MYRMT_RX_RESOLUTION_HZ      1000000

// Pulsen korter dan dit worden als ruis genegeerd (ns)
#define MYRMT_RX_MIN_PULSE_NS       1250

// Is de lijn zo lang stil (ns), dan is het frame afgelopen. Moet langer zijn dan de
// langste puls in het frame (NEC: 9 ms).
#define MYRMT_RX_IDLE_NS            12000000

// Maximaal aantal symbolen dat in een frame past
#define MYRMT_RX_MAX_SYMBOLS        128

// Hoelang myRMT_InputRead maximaal wacht op een frame (ms). 0 = niet wachten.
#define MYRMT_RX_READ_TIMEOUT_MS    0

//___ output: pulsen versturen ____________________________________________________

// Stelt de pin in als RMT uitgang (bv. voor WS2812B).
void myRMT_OutputSetup(int pin);

// Verstuurt de symbolen. Wacht tot ze helemaal verstuurd zijn.
// Geeft 0 terug bij succes, -1 bij een fout of als de setup ontbreekt.
int myRMT_Output_WriteBuffer(const rmt_symbol_word_t *symbols, int count);

//___ input: pulsen ontvangen _____________________________________________________

// Stelt de pin in als RMT ingang (bv. voor een infrarood ontvanger) en start de ontvangst.
void myRMT_InputSetup(int pin);

// Kopieert het laatst ontvangen frame naar symbols (maximaal max_count stuks).
// Geeft het aantal symbolen terug, 0 als er (nog) geen frame is, -1 bij een fout.
int myRMT_InputRead(rmt_symbol_word_t *symbols, int max_count);

//___ WS2812: een led aansturen ____________________________________________________
// Gebruikt het output-kanaal: roep eerst myRMT_OutputSetup(pin) op met de data-pin van de led.
// Tijden in ticks van MYRMT_TX_RESOLUTION_HZ (10 MHz => 1 tick = 100 ns).
#define MYRMT_WS_T0H_TICKS      4       // bit 0: 0,4 us hoog
#define MYRMT_WS_T0L_TICKS      8       //        0,8 us laag
#define MYRMT_WS_T1H_TICKS      8       // bit 1: 0,8 us hoog
#define MYRMT_WS_T1L_TICKS      4       //        0,4 us laag
#define MYRMT_WS_RESET_TICKS    500     // reset: 50 us laag (komt na elke led-waarde)

// Geeft de led de kleur r, g, b (elk 0..255). Geeft 0 terug bij succes, -1 bij een fout.
int myRMT_OutputWS2812(int r, int g, int b);

//___ IR: een infraroodcode ontvangen ______________________________________________
// Gebruikt het input-kanaal: roep eerst myRMT_InputSetup(pin) op met de pin van de IR ontvanger.
typedef enum {
    MYRMT_IR_NEC,       // 32 bits: adres, ~adres, commando, ~commando (LSB eerst)
    MYRMT_IR_SONY,      // SIRC, 12, 15 of 20 bits (LSB eerst)
} myRMT_IRType;

// Verwerkt een eventueel nieuw ontvangen frame met het gekozen type en geeft de
// laatste geldige code terug in *code.
// Geeft 1 terug als er al een code is ontvangen, 0 als er nog geen is, -1 bij een fout.
// Een NEC herhaal-frame (knop ingedrukt houden) laat de laatste code ongewijzigd.
int myRMT_InputIR(myRMT_IRType type, uint32_t *code);
