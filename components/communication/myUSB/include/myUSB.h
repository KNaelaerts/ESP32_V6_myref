#define BLOCKING true
#define BUF_SIZE 50



void myUSB_setup(int baud);

void myUSB_printf(const char *fmt, ...) ;
int myUSB_getmsg(char * buffer);