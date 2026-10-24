#include <stdio.h>
#include <stdbool.h>
#include "myADC.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static adc_oneshot_unit_handle_t unit_handle = NULL;
static adc_cali_handle_t cali_handle[MYADC_CHANNEL_COUNT];
static bool is_setup[MYADC_CHANNEL_COUNT];

static bool channel_ok(int channel)
{
    return channel >= 0 && channel < MYADC_CHANNEL_COUNT;
}

void myADC_setup(int channel)
{
    if (!channel_ok(channel) || is_setup[channel]) return;

    if (unit_handle == NULL) {
        adc_oneshot_unit_init_cfg_t unit_cfg = {
            .unit_id = MYADC_UNIT,
        };
        if (adc_oneshot_new_unit(&unit_cfg, &unit_handle) != ESP_OK) {
            unit_handle = NULL;
            return;
        }
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = MYADC_ATTEN,
        .bitwidth = MYADC_BITWIDTH,
    };
    if (adc_oneshot_config_channel(unit_handle, (adc_channel_t)channel, &chan_cfg) != ESP_OK) return;

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = MYADC_UNIT,
        .chan = (adc_channel_t)channel,
        .atten = MYADC_ATTEN,
        .bitwidth = MYADC_BITWIDTH,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle[channel]) != ESP_OK) {
        cali_handle[channel] = NULL;
    }
    is_setup[channel] = true;
}

int myADC_readRaw(int channel)
{
    if (!channel_ok(channel) || !is_setup[channel]) return -1;

    int raw = 0;
    if (adc_oneshot_read(unit_handle, (adc_channel_t)channel, &raw) != ESP_OK) return -1;
    return raw;
}

int myADC_readMV(int channel)
{
    int raw = myADC_readRaw(channel);
    if (raw < 0 || cali_handle[channel] == NULL) return -1;

    int mv = 0;
    if (adc_cali_raw_to_voltage(cali_handle[channel], raw, &mv) != ESP_OK) return -1;
    return mv;
}
