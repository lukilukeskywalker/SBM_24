#ifndef JOYSTICK_H
#define JOYSTICK_H

#define MSGQUEUE_OBJECTS_JOY 16

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

typedef enum {Arriba = 0, Abajo} teclas_t;
typedef enum {Corta, Larga} duraciones_t;

typedef struct {
  teclas_t tecla;
	duraciones_t duracion;
} MSGQUEUE_OBJ_JOY;

int Init_Th_joy(void);

#endif
