#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "myESP_S3_mini_board.h"
#include "myGPIO.h"

#define KNOP 5
#define LED 4


void app_main(void)
{
    myGPIO_LED_Setup(LED);
    myGPIO_KNOP_Setup(KNOP);

    while (1)
    {
        if (myGPIO_KNOP_GetFlank(KNOP))
        {
            myGPIO_LED_Toggle(LED);
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // pollen elke 10 ms, vangt ook bounce grotendeels op
    }
}
