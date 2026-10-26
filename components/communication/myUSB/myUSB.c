#include <stdio.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "myUSB.h"
#include "driver/usb_serial_jtag.h"

#if BLOCKING
    #define WAIT_TIME portMAX_DELAY
#else
    #define WAIT_TIME 0
#endif

void myUSB_setup(int baud)
{
     usb_serial_jtag_driver_config_t usb_serial_jtag_config = 
    {
        .rx_buffer_size = BUF_SIZE*4,
        .tx_buffer_size = BUF_SIZE*4,
    };
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);
}

int myUSB_getmsg(char * buffer)
{
    char temp;
    static int pos = 0;
    int insize;
    do
    {
        insize = usb_serial_jtag_read_bytes(&temp, 1, WAIT_TIME);
        //uart_read_bytes(uart_num, &temp, 1, WAIT_TIME);
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

void myUSB_printf(const char *fmt, ...) 
{
    char buf[256]; // Adjust size as needed
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) {
        usb_serial_jtag_write_bytes(buf, len , 10);
        //uart_write_bytes(uart_num, buf, len);
    }
}