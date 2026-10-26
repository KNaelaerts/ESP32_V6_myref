#define BLOCKING true

#define uart_num UART_NUM_2
#define TX_pin 17
#define RX_pin 18



void myUART_setup(int baud);

void myUART_printf(const char *fmt, ...) ;
int myUART_getmsg(char * buffer);