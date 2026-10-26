#include <string.h>
#include <stdbool.h>
#include "myRMT.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static rmt_channel_handle_t tx_channel = NULL;
static rmt_encoder_handle_t copy_encoder = NULL;

static rmt_channel_handle_t rx_channel = NULL;
static QueueHandle_t rx_queue = NULL;
static rmt_symbol_word_t rx_buffer[MYRMT_RX_MAX_SYMBOLS];

//___ output _____________________________________________________________________

void myRMT_OutputSetup(int pin)
{
    if (tx_channel != NULL) return;

    rmt_tx_channel_config_t cfg = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = MYRMT_TX_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = MYRMT_TX_QUEUE_DEPTH,
    };
    if (rmt_new_tx_channel(&cfg, &tx_channel) != ESP_OK) {
        tx_channel = NULL;
        return;
    }

    // de symbolen worden ongewijzigd doorgegeven aan de hardware
    rmt_copy_encoder_config_t enc_cfg = {};
    if (rmt_new_copy_encoder(&enc_cfg, &copy_encoder) != ESP_OK ||
        rmt_enable(tx_channel) != ESP_OK) {
        rmt_del_channel(tx_channel);
        tx_channel = NULL;
        copy_encoder = NULL;
    }
}

int myRMT_Output_WriteBuffer(const rmt_symbol_word_t *symbols, int count)
{
    if (tx_channel == NULL || symbols == NULL || count <= 0) return -1;

    rmt_transmit_config_t tx_cfg = {
        .loop_count = 0,    // een keer versturen
    };
    if (rmt_transmit(tx_channel, copy_encoder, symbols,
                     count * sizeof(rmt_symbol_word_t), &tx_cfg) != ESP_OK) return -1;
    if (rmt_tx_wait_all_done(tx_channel, -1) != ESP_OK) return -1;
    return 0;
}

//___ input ______________________________________________________________________

static rmt_receive_config_t rx_cfg = {
    .signal_range_min_ns = MYRMT_RX_MIN_PULSE_NS,
    .signal_range_max_ns = MYRMT_RX_IDLE_NS,
};

static bool IRAM_ATTR rx_done_cb(rmt_channel_handle_t channel,
                                 const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR((QueueHandle_t)user_ctx, edata, &woken);
    return woken == pdTRUE;
}

void myRMT_InputSetup(int pin)
{
    if (rx_channel != NULL) return;

    rmt_rx_channel_config_t cfg = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = MYRMT_RX_RESOLUTION_HZ,
        .mem_block_symbols = 64,
    };
    if (rmt_new_rx_channel(&cfg, &rx_channel) != ESP_OK) {
        rx_channel = NULL;
        return;
    }

    rx_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    rmt_rx_event_callbacks_t cbs = { .on_recv_done = rx_done_cb };

    if (rx_queue == NULL ||
        rmt_rx_register_event_callbacks(rx_channel, &cbs, rx_queue) != ESP_OK ||
        rmt_enable(rx_channel) != ESP_OK ||
        rmt_receive(rx_channel, rx_buffer, sizeof(rx_buffer), &rx_cfg) != ESP_OK) {
        rmt_del_channel(rx_channel);
        rx_channel = NULL;
        if (rx_queue != NULL) {
            vQueueDelete(rx_queue);
            rx_queue = NULL;
        }
    }
}

int myRMT_InputRead(rmt_symbol_word_t *symbols, int max_count)
{
    if (rx_channel == NULL || symbols == NULL || max_count <= 0) return -1;

    rmt_rx_done_event_data_t frame;
    if (xQueueReceive(rx_queue, &frame, pdMS_TO_TICKS(MYRMT_RX_READ_TIMEOUT_MS)) != pdTRUE) {
        return 0;
    }

    int count = (int)frame.num_symbols;
    if (count > max_count) count = max_count;
    memcpy(symbols, frame.received_symbols, count * sizeof(rmt_symbol_word_t));

    // meteen klaarmaken voor het volgende frame
    if (rmt_receive(rx_channel, rx_buffer, sizeof(rx_buffer), &rx_cfg) != ESP_OK) return -1;
    return count;
}

//___ WS2812 _____________________________________________________________________

static void ws_byte(rmt_symbol_word_t *s, int value)
{
    for (int i = 0; i < 8; i++) {
        bool bit = value & (0x80 >> i);     // meest significante bit eerst
        s[i].level0 = 1;
        s[i].duration0 = bit ? MYRMT_WS_T1H_TICKS : MYRMT_WS_T0H_TICKS;
        s[i].level1 = 0;
        s[i].duration1 = bit ? MYRMT_WS_T1L_TICKS : MYRMT_WS_T0L_TICKS;
    }
}

static int clamp8(int v)
{
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

int myRMT_OutputWS2812(int r, int g, int b)
{
    rmt_symbol_word_t symbols[25];

    // de WS2812 verwacht de volgorde G, R, B
    ws_byte(&symbols[0], clamp8(g));
    ws_byte(&symbols[8], clamp8(r));
    ws_byte(&symbols[16], clamp8(b));

    // reset: lijn laag houden zodat de led de waarde overneemt
    symbols[24].level0 = 0;
    symbols[24].duration0 = MYRMT_WS_RESET_TICKS / 2;
    symbols[24].level1 = 0;
    symbols[24].duration1 = MYRMT_WS_RESET_TICKS / 2;

    return myRMT_Output_WriteBuffer(symbols, 25);
}

//___ IR _________________________________________________________________________

// ticks naar microseconden, ongeacht de ingestelde resolutie
#define TICKS_TO_US(t)  ((int)((int64_t)(t) * 1000000 / MYRMT_RX_RESOLUTION_HZ))

// ligt de gemeten duur binnen 30% van de verwachte duur?
static bool near_us(int ticks, int expected_us)
{
    int us = TICKS_TO_US(ticks);
    return us > expected_us * 7 / 10 && us < expected_us * 13 / 10;
}

// Geeft 1 bij een nieuwe code, 2 bij een herhaal-frame, 0 bij een ongeldig frame
static int decode_nec(const rmt_symbol_word_t *s, int n, uint32_t *code)
{
    // startpuls: 9 ms laag + 4,5 ms hoog
    if (n < 2 || !near_us(s[0].duration0, 9000)) return 0;

    // herhaal-frame: 9 ms laag + 2,25 ms hoog
    if (near_us(s[0].duration1, 2250)) return 2;
    if (!near_us(s[0].duration1, 4500) || n < 33) return 0;

    uint32_t value = 0;
    for (int i = 0; i < 32; i++) {
        const rmt_symbol_word_t *b = &s[1 + i];
        if (!near_us(b->duration0, 560)) return 0;
        if (near_us(b->duration1, 1690)) value |= (1UL << i);   // LSB eerst
        else if (!near_us(b->duration1, 560)) return 0;
    }
    *code = value;
    return 1;
}

static int decode_sony(const rmt_symbol_word_t *s, int n, uint32_t *code)
{
    // startpuls: 2,4 ms laag + 0,6 ms hoog
    if (n < 2 || !near_us(s[0].duration0, 2400)) return 0;

    int bits = n - 1;
    if (bits != 12 && bits != 15 && bits != 20) return 0;

    uint32_t value = 0;
    for (int i = 0; i < bits; i++) {
        // de lengte van de lage puls bepaalt de bit: 0,6 ms = 0, 1,2 ms = 1
        int d = s[1 + i].duration0;
        if (near_us(d, 1200)) value |= (1UL << i);              // LSB eerst
        else if (!near_us(d, 600)) return 0;
    }
    *code = value;
    return 1;
}

int myRMT_InputIR(myRMT_IRType type, uint32_t *code)
{
    static uint32_t last_code = 0;
    static bool has_code = false;

    if (code == NULL) return -1;

    rmt_symbol_word_t symbols[MYRMT_RX_MAX_SYMBOLS];
    int n = myRMT_InputRead(symbols, MYRMT_RX_MAX_SYMBOLS);
    if (n < 0) return -1;

    if (n > 0) {
        uint32_t value = 0;
        int result = 0;
        if (type == MYRMT_IR_NEC) result = decode_nec(symbols, n, &value);
        else if (type == MYRMT_IR_SONY) result = decode_sony(symbols, n, &value);

        if (result == 1) {
            last_code = value;
            has_code = true;
        }
    }

    if (!has_code) return 0;
    *code = last_code;
    return 1;
}
