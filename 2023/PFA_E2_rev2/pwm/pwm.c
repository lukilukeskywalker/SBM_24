/**
 * @author Lukas Gdanietz
 * @note Modulo encargado de generar el pwm para el motor. Aparentemente no esta descrito si el los leds deben tener un pwm, por tanto, 
 * por ahora lo ignorare
 * He dicho por ahora...
*/

#include "pwm.h"

TIM_HandleTypeDef htim1;
TIM_OC_InitTypeDef htim1OC;

static int conf_pinout(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOE_CLK_ENABLE(); 
	
	GPIO_InitStruct.Pin = PWM_GPIO_OUTPUT;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
		
	htim1.Instance = TIM1;
	htim1.Init.Period = 100;
	htim1.Init.Prescaler = 20;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	__HAL_RCC_TIM1_CLK_ENABLE();
	HAL_TIM_PWM_Init(&htim1);
	htim1OC.OCMode = TIM_OCMODE_PWM1;
	htim1OC.OCPolarity = TIM_OCPOLARITY_LOW;
	htim1OC.Pulse = 50;
	
	if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim1, &htim1OC, TIM_CHANNEL_1)) 
		return (-1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		
	return(0);
}
void pwm_task(void *arg){
	uint32_t osstatus = 0, pwm_value = 0;
	osMessageQueueId_t pwm_messagequeue = *(osMessageQueueId_t *) arg;
	while(1){
		osstatus = osMessageQueueGet(pwm_messagequeue, &pwm_value, NULL, osWaitForever);
    if(osstatus == osOK){
			htim1OC.Pulse = pwm_value;
			HAL_TIM_PWM_ConfigChannel(&htim1, &htim1OC, TIM_CHANNEL_1);
			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		}
	}
}

system_err_t init_pwm(osMessageQueueId_t pwm_messagequeue){
	osThreadId_t pwm_task_id;
	if(pwm_messagequeue == NULL)
		return sys_err_invalid_arg;
	pwm_task_id = osThreadNew(pwm_task, &pwm_messagequeue, NULL);
	if(pwm_task_id == NULL)
		return sys_fail;
}