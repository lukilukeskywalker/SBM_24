#ifndef __TIM_PARAM_CALC_H__
#define __TIM_PARAM_CALC_H__
#include <stdint.h>
#include "stm32f4xx_hal.h"

int tim_config(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t desired_tick_hz, uint16_t counter_depth);

#endif
