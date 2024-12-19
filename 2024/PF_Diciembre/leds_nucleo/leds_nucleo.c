#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "leds_nucleo.h"

#define LED_1 GPIO_PIN_0
#define LED_2 GPIO_PIN_7
#define LED_3 GPIO_PIN_14

#define FLAG_IS_SET(flags, pos) ((flags >> pos) & 1ULL) 

/*
*	@brief Initializes Outputs for leds
*/
static void leds_nucleo_gpo_init(void){
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
static void leds_nucleo_th(void *args){
	uint32_t flags = 0, leds_state = 0;
	while(1){
		flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, osWaitForever);
		leds_state = 0;
		if(FLAG_IS_SET(flags, LED_1_ON))
			leds_state |= LED_1;
		if(FLAG_IS_SET(flags, LED_2_ON))
			leds_state |= LED_2;
		if(FLAG_IS_SET(flags, LED_3_ON))
			leds_state |= LED_3;
		HAL_GPIO_WritePin(GPIOB, leds_state, GPIO_PIN_SET);
		leds_state = 0;
		if(FLAG_IS_SET(flags, LED_1_OFF))
			leds_state |= LED_1;
		if(FLAG_IS_SET(flags, LED_2_OFF))
			leds_state |= LED_2;
		if(FLAG_IS_SET(flags, LED_3_OFF))
			leds_state |= LED_3;
		HAL_GPIO_WritePin(GPIOB, leds_state, GPIO_PIN_RESET);
		
	}
}
stm_err_t leds_nucleo_init(osThreadId_t *thread_id){
	leds_nucleo_gpo_init();
	STM_RETURN_ON_FALSE(thread_id != NULL, STM_ERR_INVALID_ARG, "leds_nucleo_init", "thread_id must be allocated");
	*thread_id = osThreadNew(leds_nucleo_th, NULL, NULL);
	if(*thread_id == NULL)
		return STM_ERR_NOT_FINISHED;
	return STM_OK;
}




