#include <stdio.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "myUART.h"
#include "driver/uart.h"


void myUART_setup(int baud)
{
   
    
    //uart config
    uart_config_t uart_config = {
    .baud_rate = baud,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};
    uart_param_config(uart_num, &uart_config);
     
    //pin config
    uart_set_pin(uart_num, TX_pin, RX_pin, -1, -1);    // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
    
    //driver install
    
    QueueHandle_t uart_queue;
    uart_driver_install(uart_num, 1024, 1024, 10, &uart_queue, 0);
}

#if BLOCKING
    #define WAIT_TIME portMAX_DELAY
#else
    #define WAIT_TIME 0
#endif

int myUART_getmsg(char * buffer)
{
    char temp;
    static int pos = 0;
    int insize;
    do
    {
        insize = uart_read_bytes(uart_num, &temp, 1, WAIT_TIME);
        switch (temp)
        {
        case '\n':
        case '\r':
            temp = 0;
            __attribute__((fallthrough));   // bewust: ga door naar case 0
            
        case 0:
            buffer[pos++] = temp;
            temp = pos;
            pos = 0;
            return temp;
            break;
        
        default:
            buffer[pos++] = temp;


            break;
        }



    } while (insize > 0);

    return 0;
    
    

}






void myUART_printf(const char *fmt, ...) 
{
    char buf[256]; // Adjust size as needed
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) {
        uart_write_bytes(uart_num, buf, len);
    }
}