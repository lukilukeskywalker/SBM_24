#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "../system_err.h"

// PORT B gpios
#ifndef JOY_UP
#define JOY_UP			GPIO_PIN_10
#endif

#ifndef JOY_RIGHT
#define JOY_RIGHT		GPIO_PIN_11
#endif

// PORT E gpios
#ifndef JOY_DOWN
#define JOY_DOWN		GPIO_PIN_12
#endif

#ifndef JOY_CENTER
#define JOY_CENTER	GPIO_PIN_15
#endif

#ifndef JOY_LEFT
#define JOY_LEFT		GPIO_PIN_14
#endif


typedef enum{
	center_pos, 
	up_pos, 
	down_pos, 
	left_pos, 
	right_pos,
	max_pos
}joystick_pos_t;
typedef enum{
	short_dur,
	long_dur,
	max_dur
}joystick_dur_t;

typedef struct{
	joystick_dur_t dur;
	joystick_pos_t pos;
}joy_message_t;

system_err_t init_joystick(osMessageQueueId_t joy_messagequeue);

