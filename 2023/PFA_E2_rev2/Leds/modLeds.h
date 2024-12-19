#ifndef __MOD_LEDS_H
#define __MOD_LEDS_H

#include "cmsis_os2.h"  
#include "stm32f4xx_hal.h"

#define SIZE_MSGQUEUE_LEDS			16

//Header file only contains datatypes and functions that we made available for other modules
typedef struct {                               
  uint8_t LD1;
	uint8_t LD2;
	uint8_t LD3;
} MSGQUEUE_LEDS_t;

//Module initialización
void initModLeds(void);
osMessageQueueId_t getModLedQueueID(void);
//Launching a Thread to test the module
int Init_Th_Leds_Test(void);

#endif
