#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os2.h"

//#include "stm_err.h"
//#include "stm_log.h"


typedef enum{
	JOY_UP_SPULSE 	= 1,
	JOY_RIGHT_SPULSE = 2,
	JOY_DOWN_SPULSE	= 3,
	JOY_LEFT_SPULSE	= 4,
	JOY_CENTER_SPULSE = 5,
	JOY_UP_LPULSE =	6,
	JOY_RIGHT_LPULSE = 7,
	JOY_DOWN_LPULSE	= 8,
	JOY_LEFT_LPULSE	 = 9,
	JOY_CENTER_LPULSE = 10,
	JOY_ERR,
}joy_pulse_t;

//#if JOY_PULSE_VERBOSE
/*
const char* joy_pulse_strings[] = {
    "JOY_UP_SPULSE",
    "JOY_RIGHT_SPULSE",
    "JOY_DOWN_SPULSE",
    "JOY_LEFT_SPULSE",
    "JOY_CENTER_SPULSE",
    "JOY_UP_LPULSE",
    "JOY_RIGHT_LPULSE",
    "JOY_DOWN_LPULSE",
    "JOY_LEFT_LPULSE",
    "JOY_CENTER_LPULSE",
    "JOY_ERR"
};
*/


typedef uint32_t joy_event_t;

void init_joystick_proc(osMessageQueueId_t *queueId);
void joystick_isr(void);
	

#endif
