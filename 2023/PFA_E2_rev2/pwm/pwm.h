/**
 * @author Lukas Gdanietz
 * @note Modulo encargado de generar el pwm para el motor. Aparentemente no esta descrito si el los leds deben tener un pwm, por tanto, 
 * por ahora lo ignorare
 * 
 * 
*/
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "../system_err.h"

#ifndef PWM_GPIO_OUTPUT
#define PWM_GPIO_OUTPUT			GPIO_PIN_9
#endif


typedef uint32_t pwm_queue_t;
system_err_t init_pwm(osMessageQueueId_t pwm_messagequeue);


