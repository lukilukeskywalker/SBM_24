#include "pwm.h"
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "../stm_err.h"
#include "../stm_log.h"
#include "stm32f4xx_hal.h"

static osThreadId_t *pwm_th_id = NULL;
TIM_HandleTypeDef htim1;

void init_pwm_gpi(){
	__HAL_RCC_GPIOE_CLK_ENABLE();  // Enable the GPIOE clock

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Configure GPIO pin : PE9
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Alternate Function Push-Pull
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1; // Alternate function TIM1_CH1
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}
void init_pwm_tim(){
	__HAL_RCC_TIM1_CLK_ENABLE();  // Enable the TIM1 clock

	TIM_OC_InitTypeDef sConfigOC = {0};

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 4096;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_PWM_Init(&htim1);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

void pwm_thread(void* args){
	osMessageQueueId_t *queueId = (osMessageQueueId_t *)args;
	uint32_t dutyCycle = 0;
    
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	while (1) {
		if (osMessageQueueGet(queueId, &dutyCycle, NULL, osWaitForever) == osOK) {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutyCycle);
		}
		//for(dutyCycle = 0; dutyCycle < 1024; dutyCycle++){
		//	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutyCycle);
		//	osDelay(25);
		//}
		
	}
}

stm_err_t init_pwm_proc(osMessageQueueId_t *queueId){
	STM_RETURN_ON_FALSE(queueId != NULL, STM_ERR_INVALID_ARG, __func__, "queueId can't be null");
	STM_RETURN_ON_FALSE(pwm_th_id == NULL, STM_ERR_INVALID_STATE, __func__, "PWM task exists already");

	pwm_th_id = osThreadNew(pwm_thread, queueId, NULL);
	STM_RETURN_ON_FALSE(pwm_th_id != NULL, STM_FAIL, __func__, "Joystick task gen failed");
    
	init_pwm_gpi();
	init_pwm_tim();
	STM_LOGD(__func__, "Joystick init completed");
    
	return STM_OK;
}