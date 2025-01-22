#include <stdlib.h>
#include "sbminit.h"
#include "stm32f4xx_hal.h"


#define LED_1 GPIO_PIN_0
#define LED_2 GPIO_PIN_7
#define LED_3 GPIO_PIN_14
TIM_HandleTypeDef htim11; //define cual es el manejador que vaamos a utilizar

void leds_nucleo_gpo_init(void){
	/*GPIO_InitTypeDef gpio_conf = {
		.Pin = LED_1 | LED_2 | LED_3,
		.Mode = GPIO_MODE_OUTPUT_PP,
	};*/
	GPIO_InitTypeDef gpio_conf;
	gpio_conf.Pin = LED_1 | LED_2 | LED_3;
	gpio_conf.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_conf.Pull = GPIO_NOPULL;  // Always initialize Pull (default is undefined)
	gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;  // Initialize Speed
	//Init Peripheral CLKs
	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_Init(GPIOB, &gpio_conf);
	return;
}
void tim2_init(void){
	TIM_HandleTypeDef htim2;
	
	__HAL_RCC_TIM2_CLK_ENABLE();

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 6999;
	htim2.Init.Period = 11;
	
	HAL_TIM_OC_Init(&htim2);
}
void tim11_oc_interrupt(uint16_t toggleperiod_us){
	//TIM_HandleTypeDef htim;
	__HAL_RCC_TIM11_CLK_ENABLE();
	uint32_t sysclk = HAL_RCC_GetSysClockFreq();
	
	htim11.Instance = TIM11;
	htim11.Init.Prescaler = 6999;//(uint32_t)((sysclk / 1000000U) - 1);
	htim11.Init.CounterMode = TIM_COUNTERMODE_UP; 
	htim11.Init.Period = 11;//toggleperiod_us - 1; 
	htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	HAL_TIM_Base_Init(&htim11);

	HAL_TIM_Base_Start_IT(&htim11);
	
	//HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 0, 0); // Set interrupt priority
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);         // Enable TIM11 interrupt

	
}
