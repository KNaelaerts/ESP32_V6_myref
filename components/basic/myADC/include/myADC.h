#pragma once

//___ interne instellingen ________________________________________________________
// Deze instellingen gebruikt myADC.c intern. Pas ze hier aan indien nodig.

// ADC eenheid: ADC1 (ADC2 kan storen met wifi, dus die laten we links liggen)
#define MYADC_UNIT          ADC_UNIT_1

// Resolutie: DEFAULT = de maximale resolutie van de chip (12 bit op de ESP32-S3 => 0..4095)
#define MYADC_BITWIDTH      ADC_BITWIDTH_DEFAULT

// Verzwakking (attenuation): bepaalt het meetbereik van de pin
//   ADC_ATTEN_DB_0   ~ 0 .. 950 mV    (meest nauwkeurig, klein bereik)
//   ADC_ATTEN_DB_2_5 ~ 0 .. 1250 mV
//   ADC_ATTEN_DB_6   ~ 0 .. 1750 mV
//   ADC_ATTEN_DB_12  ~ 0 .. 3100 mV   (bijna het hele 3.3V bereik)
#define MYADC_ATTEN         ADC_ATTEN_DB_12

// Aantal ADC kanalen van ADC1 op de ESP32-S3 (kanaal 0..9 = GPIO1..GPIO10)
#define MYADC_CHANNEL_COUNT 10

//___ functies ____________________________________________________________________
// Alle functies krijgen enkel het ADC kanaal mee (0..9 voor ADC1 op de ESP32-S3)

// Stelt het kanaal in met bovenstaande instellingen. Roep dit eenmaal op voor je meet.
void myADC_setup(int channel);

// Geeft de ruwe ADC waarde terug (0..4095 bij 12 bit), of -1 bij een fout.
int myADC_readRaw(int channel);

// Geeft de gekalibreerde spanning terug in millivolt, of -1 bij een fout.
int myADC_readMV(int channel);
