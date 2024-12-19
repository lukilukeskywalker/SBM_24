#include "led_control.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include "../PF_conf.h"

//#define RGB_TEST 1

/**
*	@brief El modulo led_control realiza el control de cada uno de los leds en el sistema, tanto RGB como Led de alta luminosidad. Las especificaciones se detallan de aqui en adelante:
* 				* 
*/

static stm_err_t conf_led_control(void){
	const GPIO_InitTypeDef RGB_GPIO_InitStruct = {
		.Pin = RGB_LEDR_PIN | RGB_LEDG_PIN | RGB_LEDB_PIN,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_PULLUP,
		.Speed = GPIO_SPEED_FREQ_LOW,
		.Alternate = 0,		
	};
	HAL_GPIO_Init(RGB_LEDS_PORT, &RGB_GPIO_InitStruct);
	
	
	
	
	return STM_OK;
}

static void test_rgb_led_control(void){
	for(int led_flag = 0; led_flag < 8; led_flag++){
		HAL_GPIO_WritePin(RGB_LEDS_PORT, RGB_LEDR_PIN, !(led_flag >> 0) &1ULL);
		HAL_GPIO_WritePin(RGB_LEDS_PORT, RGB_LEDG_PIN, !(led_flag >> 1) &1ULL);
		HAL_GPIO_WritePin(RGB_LEDS_PORT, RGB_LEDB_PIN, !(led_flag >> 2) &1ULL);
		HAL_Delay(1000);
	}
}	

stm_err_t init_led_control(void){
	conf_led_control();
#ifdef RGB_TEST
	test_rgb_led_control();
#endif
	
	
}

