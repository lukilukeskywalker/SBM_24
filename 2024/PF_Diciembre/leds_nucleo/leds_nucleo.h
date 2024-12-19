#ifndef __LEDS_NUCLEO_H__
#define __LEDS_NUCLEO_H__
#include "../stm_err.h"
#include "cmsis_os2.h"


typedef enum{
	LED_1_ON = 0,
	LED_2_ON = 1,
	LED_3_ON = 2,
	LED_1_OFF = 3,
	LED_2_OFF = 4,
	LED_3_OFF = 5
}LEDS_NUCLEO_FLAG;

stm_err_t leds_nucleo_init(osThreadId_t *thread_id);



#endif
