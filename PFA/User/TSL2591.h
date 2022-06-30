#ifndef __TSL_H
#define __TSL_H
#include "cmsis_os2.h"
#include "Driver_I2C.h"

#define INIT_LIGHT_MEASURE 0x10000000
#define INIT_LIGHT_CYCLE 0x20000000
#define TMR_RESP_FLAG 0x00000800		//Fuera de ualquier dato que pueda introducir I2C
#define N_CYCLE_MASK 	0x0FF00000
#define N_CYCLE_SHIFT 20
#define N_TIME_MASK 	0x000FF000		//LSB Byte es gestor de I2C
#define N_TIME_SHIFT 12
#define MSGQUEUE_OBJECTS_I2C 16

#define MAX_VALUE_VIS 38000 //37889
#define MIN_VALUE_VIS 25

typedef struct{
	uint8_t light_cen;
	uint8_t light_dec;
	uint8_t light_uni;
	uint8_t ciclo;
}MSGQUEUE_OBJ_I2C;

int Init_Th_I2C (void);

#endif
