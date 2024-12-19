#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "../system_err.h"

typedef struct{
	ADC_HandleTypeDef *adc_handle;
	osMessageQueueId_t pots_messagequeue;
	osThreadId_t pots_task_id;
}pots_handle_t;
typedef struct{
	uint16_t pot_1;
	uint16_t pot_2;
}pots_message_t;

system_err_t init_pots(osMessageQueueId_t pots_messagequeue);