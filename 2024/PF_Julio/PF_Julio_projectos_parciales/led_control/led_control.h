#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__
#include "stdio.h"
#include "stdint.h"
#include "../stm_err.h"
typedef union{
	uint32_t flags;
	struct{
		uint8_t RESERVED: 8; 				//!< Reservado para flags de Interrupcion de timer
		uint8_t sys_state: 2;				//!< Estado del systema. Determinara como se comportaran los LEDs
		uint8_t pwm_percentage: 8;			//!< Porcentaje de PWM [
	};
}led_control_flags_t;
stm_err_t init_led_control(void);

#endif
