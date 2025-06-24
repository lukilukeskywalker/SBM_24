#include "pb2_exam_helper.h"
#include <string.h>
#include "lcd.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

#define DRAW_BUF_SIZE 512
#define FLAG_BTN 0x02
#define FLAG_LOCK 0x04

TIM_HandleTypeDef htim4;
static uint8_t draw_buf[DRAW_BUF_SIZE] = {0};
static osThreadId_t id_pb2_thread;


int init_pb2_thread(void);
void tim4_init(uint32_t desired_tick_hz, uint16_t counter_depth);
void tim4_restart(void);
static void pb2_thread(void *arguments);

/** INTERRUPCIONES */
void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); // deshabilita la interrupción del pin del pulsador azul B1
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){ 
	static int lock = 0; 
	if(GPIO_Pin == GPIO_PIN_13 && lock == 0){
		osThreadFlagsSet(id_pb2_thread, FLAG_BTN);
	}
}

void TIM4_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim4);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4){
		osThreadFlagsSet(id_pb2_thread, FLAG_LOCK);
	}
}


void init_pb2(void){

	//Init_Th_lcd();
	//Init_Th_lcd_test();
	BTN1_init();
	init_pb2_thread();
	
	return;
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

int init_pb2_thread(void){
	id_pb2_thread = osThreadNew(pb2_thread, NULL, NULL);
	if (id_pb2_thread == NULL)
		return -1;
	return 0;
}

static int draw_knight_rider(uint8_t restart){
	const uint16_t max_progress = 64;
	static uint16_t progress = 0, position = 0;
	if(restart != 0){
		progress = 0;
		memset(draw_buf, 0, DRAW_BUF_SIZE);
		lcd_clean();
	}
	for(uint8_t line = 0; line < 4; line++){
		position = 64 + (line * 128) + progress;
		draw_buf[position] = 0xFF;
		position = 64 + (line * 128) - progress;
		draw_buf[position] = 0xFF;
	}
	lcd_write_drawbuf(draw_buf);
	osDelay(200);
	progress++;
	progress %=max_progress;
	return progress - 1;
}

static void pb2_thread(void *arguments){
	lcd_init(&id_pb2_thread);
	tim4_init(1000, 5000);		// 5seg
	memset(draw_buf, 0, DRAW_BUF_SIZE);
	uint8_t restart = 0, flags;
	osThreadFlagsClear(FLAG_BTN | FLAG_LOCK);
	while(1){
		//if(osThreadFlagsWait(FLAG_BTN, osFlagsWaitAll, 0) != osFlagsErrorTimeout)
			//restart = 1;
		flags = osThreadFlagsGet();
		if(FLAG_BTN == flags){
			osThreadFlagsClear(FLAG_BTN);
			tim4_restart();
			restart = 1;
		}
		if(FLAG_LOCK == flags){
			memset(draw_buf, 0xFF, DRAW_BUF_SIZE);
			lcd_write_drawbuf(draw_buf);
			break;
		}
		draw_knight_rider(restart);
		restart = 0;
	}
	while(1);
	
}

/**
* @param Timer handle
* @param Timer instance
* @param desired tick grane
* @param desired counter depth
* @return Returns multipler of counter_depth in case the prescaler can't hold the required prescaler */
uint16_t tim_config(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t desired_tick_hz, uint16_t counter_depth){
	htim->Instance = instance;
	
	RCC_ClkInitTypeDef clkconf;
	uint32_t flash_latency;
	HAL_RCC_GetClockConfig(&clkconf, &flash_latency);
	uint32_t pclk;
	if (instance == TIM1 || instance == TIM8 || instance == TIM9 || instance == TIM10 || instance == TIM11) {
        // APB2 timers
		pclk = HAL_RCC_GetPCLK2Freq();
		if ((clkconf.APB2CLKDivider != RCC_HCLK_DIV1)){
			pclk *= 2;
		}
	}else{
        // APB1 timers
		pclk = HAL_RCC_GetPCLK1Freq();
		if ((clkconf.APB1CLKDivider != RCC_HCLK_DIV1)) {
			pclk *= 2;
		}
	}
	
	uint32_t prescaler = (pclk /desired_tick_hz);
	uint16_t multiplier = 0;
	while(prescaler > 0xFFFF){
		// This process moves the divider from the prescaler to the period counter
		prescaler >>= 1;         // Is like dividing bz 2
		counter_depth <<= 1; // Is like multiplying through 2
		multiplier += 2;
	}
	htim->Init.Prescaler = prescaler - 1;
	htim->Init.Period = counter_depth - 1;
	return multiplier;
}
void tim4_init(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_config(&htim4, TIM4, desired_tick_hz, counter_depth);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	HAL_TIM_Base_Init(&htim4);
	HAL_TIM_Base_Start_IT(&htim4);
	
	return;
}
void tim4_restart(void){
	HAL_TIM_Base_Stop(&htim4);           // For basic mode
	// or HAL_TIM_OC_Stop(&htimX, channel);   // For output compare
	// or HAL_TIM_PWM_Stop(&htimX, channel);  // For PWM

	// Reset the counter
	__HAL_TIM_SET_COUNTER(&htim4, 0);

	// Start the timer again
	HAL_TIM_Base_Start(&htim4);          // or OC/PWM version
}


