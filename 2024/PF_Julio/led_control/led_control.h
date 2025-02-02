#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__


#define MSGQUEUE_OBJECTS_RGB 16

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef enum{OFF = 0, ON}led_t;

typedef struct{
	led_t estado_rgb;
	led_t estado_led2;
	led_t estado_led3;
}MSGQUEUE_OBJ_RGB;

int Init_Th_rgb(void);
osMessageQueueId_t led_control_msgQueue(void);
#endif

