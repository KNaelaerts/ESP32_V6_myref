//GPIO staat voor General Perpose Input Output
//De meest eenvoudige voorbeeldjes hiervan zijn LEDje's en Knopje's
//Deze file zal dan ook deze elektronica in gedachte houden 
//Maar natuurlijk kan je het toepassen op andere elektronica ook en kan het dienen als inspiratie

//___LED's____________________________________________________________________________

void myGPIO_LED_Setup(int pin);
void myGPIO_LED_SetValue(int pin, int value);
void myGPIO_LED_On(int pin);
void myGPIO_LED_Off(int pin);
void myGPIO_LED_Toggle(int pin);


//___KNOP's____________________________________________________________________________

void myGPIO_KNOP_Setup(int pin);
void myGPIO_KNOP_SetupInterrupt(int pin);
int myGPIO_KNOP_GetValue(int pin);
int myGPIO_KNOP_GetFlank(int pin);