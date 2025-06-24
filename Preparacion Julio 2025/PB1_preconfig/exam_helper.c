#include "stm32f4xx_hal.h"
#include "exam_helper.h"
#include "math.h"

TIM_HandleTypeDef htim3,htim4, htim7, htim12;

typedef struct {
    uint32_t pattern;         // The bit pattern (e.g., 0b0101...)
    uint8_t period_size;      // Number of bits in the pattern (max 32)
    uint8_t counter;          // Internal counter (state)
} BlinkPattern;

uint8_t next_blink_state(BlinkPattern *blink) {
    blink->counter = (blink->counter + 1) % blink->period_size;
    return (blink->pattern >> blink->counter) & 0x1;
}

BlinkPattern ld2_blink = {
	.pattern = 0x1,
	.period_size = 10,
	.counter = -1,
};

/** INTERRUPCIONES */
void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); // deshabilita la interrupción del pin del pulsador azul B1
}
void TIM4_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim4);
}
void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);
}
/** HAL Handlers */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){ 
	static int lock = 0; 
	if(GPIO_Pin == GPIO_PIN_13 && lock == 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);		// Turn LED1 ON
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);		// Turn LED2 ON
		//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);		// Turn LED3 ON
		//TIM4
		//tim4_init(1000, 1500);					// 1000 Hz (1 ms) and the counter to 1500
		//tim4_chn2_init(1000, 1500);
		//MX_TIM4_Init();
		//TIM7
		tim7_init(1000, 150);
		tim3_chn3_init(1000, 500);
		tim12_chn1_init(1000, 5000);
		lock = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4){
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);			// Blink led 2
	}
	if(htim->Instance == TIM7){
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, next_blink_state(&ld2_blink));
	}
	
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
	
	gpio_conf.Pin = GPIO_PIN_0 | GPIO_PIN_14 | GPIO_PIN_7;
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
void tim3_chn3_init(uint32_t desired_tick_hz, uint16_t counter_depth) {
    tim_config(&htim3, TIM3, desired_tick_hz, counter_depth);
    
    GPIO_InitTypeDef gpio_conf = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure PB0 as TIM3_CH3 (AF2)
    gpio_conf.Pin = GPIO_PIN_0;
    gpio_conf.Mode = GPIO_MODE_AF_PP;
    gpio_conf.Pull = GPIO_NOPULL;
    gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_conf.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &gpio_conf);

    // Configure Output Compare for TIM3 Channel 3
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);

    HAL_TIM_OC_Init(&htim3);
    HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_3);
}
void tim4_chn2_init(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_config(&htim4, TIM4, desired_tick_hz, counter_depth);
	GPIO_InitTypeDef gpio_conf = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	gpio_conf.Pin = GPIO_PIN_7;
	gpio_conf.Mode = GPIO_MODE_AF_PP;
	gpio_conf.Pull = GPIO_NOPULL;
	gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_conf.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOB, &gpio_conf);
	
	
	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = 0;  // Initial compare value
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2);
	
	HAL_TIM_OC_Init(&htim4);
	HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_2);
	
	return;
}
void tim12_chn1_init(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_config(&htim12, TIM12, desired_tick_hz, counter_depth);
	GPIO_InitTypeDef gpio_conf = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_TIM12_CLK_ENABLE();
	
	gpio_conf.Pin = GPIO_PIN_14;
	gpio_conf.Mode = GPIO_MODE_AF_PP;
	gpio_conf.Pull = GPIO_NOPULL;
	gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_conf.Alternate = GPIO_AF9_TIM12;
	HAL_GPIO_Init(GPIOB, &gpio_conf);
	
	
	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = 0;  // Initial compare value
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1);
	
	HAL_TIM_OC_Init(&htim12);
	HAL_TIM_OC_Start(&htim12, TIM_CHANNEL_1);
	
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

void tim7_init(uint32_t desired_tick_hz, uint16_t counter_depth){
	tim_config(&htim7, TIM7, desired_tick_hz, counter_depth);
	HAL_NVIC_EnableIRQ(TIM7_IRQn);
	__HAL_RCC_TIM7_CLK_ENABLE();
	
	HAL_TIM_Base_Init(&htim7);
	HAL_TIM_Base_Start_IT(&htim7);
	
	return;
}
