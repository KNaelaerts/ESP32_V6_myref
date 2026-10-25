#pragma once

//___ interne instellingen ________________________________________________________
// Deze instellingen gebruikt myUltrasoon_HCSR04.c intern.

// Lengte van de triggerpuls in microseconden (de HC-SR04 vraagt minstens 10 us)
#define MYULTRASOON_TRIG_US     10

// Maximale echotijd in microseconden: 25 ms is ongeveer 4 m, de max van de sensor
#define MYULTRASOON_TIMEOUT_US  25000

// Tijd tussen twee metingen in de continue modus, in milliseconden (minstens 60 ms)
#define MYULTRASOON_PERIOD_MS   60

// Omrekening: afstand (cm) = echotijd (us) / 58
#define MYULTRASOON_US_PER_CM   58

//___ single: een meting op aanvraag ______________________________________________
// De functie wacht (blokkeert) tot de echo terug is of tot de timeout verstreken is.

// Stelt de pinnen in voor losse metingen.
void myUltrasoon_singleSetup(int trig_pin, int echo_pin);

// Doet een meting en geeft de echotijd terug in us, of -1 bij een fout of timeout.
int myUltrasoon_singleReadUS(void);

// Doet een meting en geeft de afstand terug in cm, of -1 bij een fout of timeout.
int myUltrasoon_singleReadCM(void);

//___ multi: continu meten op de achtergrond ______________________________________
// Een timer start om de MYULTRASOON_PERIOD_MS een meting en een interrupt op de
// echo-pin meet de puls. De read-functies geven meteen de laatste meting terug.

// Stelt de pinnen in en start het continu meten.
void myUltrasoon_continueSetup(int trig_pin, int echo_pin);

// Geeft de laatste echotijd terug in us, of -1 als er (nog) geen geldige meting is.
int myUltrasoon_multiReadUS(void);

// Geeft de laatste afstand terug in cm, of -1 als er (nog) geen geldige meting is.
int myUltrasoon_multiReadCM(void);

//___ uitleg: esp_timer versus gptimer ____________________________________________
// Deze component gebruikt esp_timer. Een alternatief met gptimer staat in comment
// onderaan myUltrasoon_HCSR04.c.
//
//                          esp_timer                      gptimer
// Niveau                   software-laag boven            directe hardware-timer
//                          een hardware-timer
// Aantal                   heel veel tegelijk             beperkt: 2 per timergroep
//                                                         (ESP32-S3: 4 in totaal)
// Resolutie                1 us                           instelbaar, tot 80 MHz (12,5 ns)
// Callback                 aparte task (of optioneel      in de ISR, zeer nauwkeurig
//                          in de ISR), enkele tot
//                          tientallen us jitter
// Teller lezen             esp_timer_get_time()           gptimer_get_raw_count()
// Alarmen                  eenmalig of periodiek,         een alarm per timer, met
//                          een callback per timer         automatisch herladen
// Gebruik                  timeouts, periodieke taken,    nauwkeurige tijdsmeting,
//                          tijdstempels                   snelle periodieke interrupts
// Moeite                   eenvoudig                      meer instelwerk: aanmaken,
//                                                         configureren, enable, start
//
// - esp_timer is voor "doe dit over 60 ms" en "hoe laat is het". Dat volstaat voor
//   de HC-SR04: 1 us komt overeen met ongeveer 0,017 cm.
// - gptimer heb je nodig als de timing strak moet zijn, bv. voor het meten van
//   pulsen van enkele us.
// - esp_timer gebruikt zelf intern een hardware-timer die je deelt met de rest van
//   het systeem. Een gptimer is helemaal van jou.
// - esp_timer_get_time() hoeft niet gestart te worden en heeft geen handler nodig:
//   de teller loopt al sinds het opstarten.
//   esp_timer_create() + esp_timer_start_periodic() heeft wel een handler en een
//   start nodig.
