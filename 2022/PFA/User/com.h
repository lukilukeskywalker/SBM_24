#ifndef COM_H
#define COM_H

#define MSGQUEUE_OBJECTS_COM 16

#define USART_NEW_LINE		//Definir para contestar en nueva linea

#include "Driver_USART.h"

typedef enum{
	HOUR_SETUP = 0x20,
	COUNTDOWN_SETUP = 0x25,
	COUNTDOWN_READ = 0x35,
	START_MEASURE =0x70,
	RET_NUM_MEASURE = 0x40,
	RET_LAST_MEASURE = 0x50,
	RET_ALL_MEASURE = 0x55,
	DEL_ALL_MEASURE = 0x60
}Operaciones_t;

typedef struct{
	Operaciones_t op;
	uint8_t size;
	uint8_t last_msg;
	char data[32];
}MSGQUEUE_OBJ_COM;

int Init_Th_com(void);

#endif
