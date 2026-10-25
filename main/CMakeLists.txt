idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES myESP_S3_mini_board myGPIO myADC myUltrasoon_HCSR04)
