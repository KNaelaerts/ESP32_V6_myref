#include <stdbool.h>
#include "myUltrasoon_HCSR04.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

typedef enum { MODE_NONE, MODE_SINGLE, MODE_CONTINUE } us_mode_t;

static int trig = -1;
static int echo = -1;
static us_mode_t mode = MODE_NONE;

static esp_timer_handle_t periodic_timer = NULL;
static bool isr_service_ready = false;

static volatile int64_t echo_start = 0;
static volatile int last_us = -1;
static volatile bool got_echo = false;

static void pins_init(int trig_pin, int echo_pin)
{
    trig = trig_pin;
    echo = echo_pin;

    gpio_reset_pin(trig);
    gpio_set_direction(trig, GPIO_MODE_OUTPUT);
    gpio_set_level(trig, 0);

    gpio_reset_pin(echo);
    gpio_set_direction(echo, GPIO_MODE_INPUT);
}

static void send_trigger(void)
{
    gpio_set_level(trig, 1);
    esp_rom_delay_us(MYULTRASOON_TRIG_US);
    gpio_set_level(trig, 0);
}

static void stop_continue(void)
{
    if (mode != MODE_CONTINUE) return;
    esp_timer_stop(periodic_timer);
    gpio_isr_handler_remove(echo);
    gpio_set_intr_type(echo, GPIO_INTR_DISABLE);
}

static int us_to_cm(int us)
{
    return us < 0 ? -1 : us / MYULTRASOON_US_PER_CM;
}

//___ single _____________________________________________________________________

void myUltrasoon_singleSetup(int trig_pin, int echo_pin)
{
    stop_continue();
    pins_init(trig_pin, echo_pin);
    mode = MODE_SINGLE;
}

int myUltrasoon_singleReadUS(void)
{
    if (mode != MODE_SINGLE) return -1;

    send_trigger();

    // wachten tot de echo-puls begint
    int64_t t0 = esp_timer_get_time();
    while (!gpio_get_level(echo)) {
        if (esp_timer_get_time() - t0 > MYULTRASOON_TIMEOUT_US) return -1;
    }

    // meten hoelang de echo-puls duurt
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(echo)) {
        if (esp_timer_get_time() - start > MYULTRASOON_TIMEOUT_US) return -1;
    }
    return (int)(esp_timer_get_time() - start);
}

int myUltrasoon_singleReadCM(void)
{
    return us_to_cm(myUltrasoon_singleReadUS());
}

//___ multi ______________________________________________________________________

static void IRAM_ATTR echo_isr(void *arg)
{
    int64_t now = esp_timer_get_time();
    if (gpio_get_level(echo)) {
        echo_start = now;
    } else if (echo_start != 0) {
        int64_t duur = now - echo_start;
        echo_start = 0;
        if (duur <= MYULTRASOON_TIMEOUT_US) {
            last_us = (int)duur;
            got_echo = true;
        }
    }
}

static void periodic_cb(void *arg)
{
    // geen echo ontvangen sinds de vorige trigger: meting ongeldig
    if (!got_echo) last_us = -1;
    got_echo = false;
    echo_start = 0;
    send_trigger();
}

void myUltrasoon_continueSetup(int trig_pin, int echo_pin)
{
    stop_continue();
    pins_init(trig_pin, echo_pin);

    last_us = -1;
    got_echo = false;
    echo_start = 0;

    if (!isr_service_ready) {
        gpio_install_isr_service(0);
        isr_service_ready = true;
    }
    gpio_set_intr_type(echo, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(echo, echo_isr, NULL);

    if (periodic_timer == NULL) {
        const esp_timer_create_args_t args = {
            .callback = periodic_cb,
            .name = "myUltrasoon",
        };
        if (esp_timer_create(&args, &periodic_timer) != ESP_OK) {
            periodic_timer = NULL;
            return;
        }
    }
    esp_timer_start_periodic(periodic_timer, MYULTRASOON_PERIOD_MS * 1000);
    mode = MODE_CONTINUE;
}

int myUltrasoon_multiReadUS(void)
{
    if (mode != MODE_CONTINUE) return -1;
    return last_us;
}

int myUltrasoon_multiReadCM(void)
{
    return us_to_cm(myUltrasoon_multiReadUS());
}

//___ alternatief met gptimer (in comment) ________________________________________
// Om dit te gebruiken:
//  - voeg esp_driver_gptimer toe aan PRIV_REQUIRES in CMakeLists.txt
//  - voeg #include "driver/gptimer.h" toe bovenaan
//  - vervang in singleReadUS en echo_isr esp_timer_get_time() door gptimer_now_us()
//  - roep gptimer_setup() op in singleSetup en continueSetup, na pins_init()
//
// De gptimer draait als stopwatch aan 1 MHz (1 tick = 1 us). Hij wordt een keer
// gestart en blijft daarna doortellen. De tijd lees je met gptimer_get_raw_count().
// Dit kan ook in een ISR.
//
// static gptimer_handle_t gptimer = NULL;
//
// static void gptimer_setup(void)
// {
//     if (gptimer != NULL) return;
//
//     gptimer_config_t config = {
//         .clk_src = GPTIMER_CLK_SRC_DEFAULT,
//         .direction = GPTIMER_COUNT_UP,
//         .resolution_hz = 1000000,           // 1 MHz => 1 tick = 1 us
//     };
//     if (gptimer_new_timer(&config, &gptimer) != ESP_OK) {
//         gptimer = NULL;
//         return;
//     }
//     gptimer_enable(gptimer);
//     gptimer_start(gptimer);                 // een keer starten, telt daarna door
// }
//
// static inline int64_t IRAM_ATTR gptimer_now_us(void)
// {
//     uint64_t count = 0;
//     gptimer_get_raw_count(gptimer, &count);
//     return (int64_t)count;
// }
//
// Wil je ook de periodieke trigger van de multi-modus met de gptimer doen, dan
// gebruik je een alarm met automatisch herladen en een callback in de ISR:
//
// static bool IRAM_ATTR alarm_cb(gptimer_handle_t timer,
//                                const gptimer_alarm_event_data_t *edata,
//                                void *user_ctx)
// {
//     if (!got_echo) last_us = -1;
//     got_echo = false;
//     echo_start = 0;
//     send_trigger();     // esp_rom_delay_us is toegelaten in een ISR
//     return false;       // geen hogere-prioriteitstask wakker gemaakt
// }
//
// static void gptimer_start_periodic(void)
// {
//     gptimer_event_callbacks_t cbs = { .on_alarm = alarm_cb };
//     gptimer_register_event_callbacks(gptimer, &cbs, NULL);   // voor gptimer_enable()
//
//     gptimer_alarm_config_t alarm = {
//         .alarm_count = MYULTRASOON_PERIOD_MS * 1000,         // in ticks (us)
//         .reload_count = 0,
//         .flags.auto_reload_on_alarm = true,
//     };
//     gptimer_set_alarm_action(gptimer, &alarm);
// }
//
// Let op: gptimer_register_event_callbacks() moet je oproepen voor
// gptimer_enable(). Zet je de periodieke versie aan, verplaats die oproep dan in
// gptimer_setup(), voor gptimer_enable().
