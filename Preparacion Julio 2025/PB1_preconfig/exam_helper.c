#include "stm32f4xx_hal.h"
#include "exam_helper.h"

TIM_HandleTypeDef htim4;

/** INTERRUPCIONES */
void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); // deshabilita la interrupción del pin del pulsador azul B1
}
void TIM4_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim4);
}
/** HAL Handlers */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){ 
	static int lock = 0; 
	if(GPIO_Pin == GPIO_PIN_13 && lock == 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);		// Turn LED1 ON
		tim4_init(1000, 1500);					// 1000 Hz (1 ms) and the counter to 1500
		lock = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4)
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);			// Blink led 2
	
}





void BTN1_init(void){
	GPIO_InitTypeDef gpio_conf = {0};
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	
	gpio_conf.Pin = GPIO_PIN_13;
	gpio_conf.Mode = GPIO_MODE_IT_RISING;
	gpio_conf.Pull = GPIO_PULLDOWN;
	
	HAL_GPIO_Init(GPIOC, &gpio_conf);
	
	return;
}

void LEDs_init(void){
	GPIO_InitTypeDef gpio_conf = {0};
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	gpio_conf.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
	gpio_conf.Pull = GPIO_NOPULL;
	gpio_conf.Mode = GPIO_MODE_OUTPUT_PP;
	
	HAL_GPIO_Init(GPIOB, &gpio_conf);
	
	return;
	
}

/** LED Viable configurations
*   LED1   TIM3_CH3
*   LED2   TIM4_CH2
*   LED3   TIM12_CH1
*/
/**
* @param instance
* @param desired_tick_hz desired tick freq in Hz. It will calculate from SystemCoreClock the prescaler to achieve the granularity of the counter

*/
void tim_config(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t desired_tick_hz, uint16_t counter_depth){
	htim->Instance = instance;
	htim->Init.Prescaler = (SystemCoreClock/desired_tick_hz) - 1;
	htim->Init.Period = counter_depth - 1;
	
	return;
}

void tim4_init(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_config(&htim4, TIM4, desired_tick_hz, counter_depth);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start_IT(&htim4);
	
	return;
}
//void tim_IRQ_start(
/*
void tim7_config(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_init(&htim7, TIM7, desired_tick_hz, counter_depth);
	return;
}
*/
