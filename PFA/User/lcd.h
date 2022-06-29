#ifndef LCD_H
#define LCD_H

#define MSGQUEUE_OBJECTS_LCD 16

#include "Driver_SPI.h"  
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h" 

typedef struct {
	char data_L1[32];
	uint16_t init_L1;
	char data_L2[32];
	uint16_t init_L2;
} MSGQUEUE_OBJ_LCD;


int Init_Th_lcd(void);

#endif
