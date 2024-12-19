#include "led_control.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include "../PF_conf.h"

//#define RGB_TEST 1


TIM_HandleTypeDef htim4;
TIM_OC_InitTypeDef htim4OC;

osThreadId_t tid_Th_rgb;
osMessageQueueId_t mid_MsgQueue_rgb;

static uint32_t i;
static int8_t polarity = 1;

static osTimerId_t Tmr_350ms;
static osTimerId_t Tmr_25ms;
static osTimerId_t Tmr_1s;

void Th_rgb(void *argument);

static int conf_pinout(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	htim4.Instance = TIM4;
	htim4.Init.Period = 100;
	htim4.Init.Prescaler = 20;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	__HAL_RCC_TIM4_CLK_ENABLE();
	HAL_TIM_PWM_Init(&htim4);
	htim4OC.OCMode = TIM_OCMODE_PWM1;
	htim4OC.OCPolarity = TIM_OCPOLARITY_LOW;
	htim4OC.Pulse = 50;
	
	if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim4, &htim4OC, TIM_CHANNEL_1)) 
		return (-1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
		
	return(0);
}

static void tmr_350ms_Callback(void* argument){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

static void tmr_1s_Callback(void* argument) {
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
}

static void tmr_25ms_Callback(void* argument) {
	polarity = ((10 == i) || (i == 90)) ? -polarity : polarity;
	i += polarity;
	htim4OC.Pulse = i;
	HAL_TIM_PWM_ConfigChannel(&htim4, &htim4OC, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}

int Init_MsgQueue_rgb(void) {
  mid_MsgQueue_rgb = osMessageQueueNew(MSGQUEUE_OBJECTS_RGB, sizeof(MSGQUEUE_OBJ_RGB), NULL);
  if(mid_MsgQueue_rgb == NULL)
    return (-1); 
  return(0);
}
osMessageQueueId_t led_control_msgQueue(void){
	return mid_MsgQueue_rgb;
}

int Init_Th_rgb(void){
	tid_Th_rgb = osThreadNew(Th_rgb, NULL, NULL);
	if(tid_Th_rgb == NULL)
		return -1;
	conf_pinout();
	Init_MsgQueue_rgb();
	return 0;
}

void Th_rgb(void *argument){
	Tmr_350ms = osTimerNew(tmr_350ms_Callback, osTimerOnce, (void *)0, NULL);
	Tmr_25ms = osTimerNew(tmr_25ms_Callback, osTimerPeriodic, (void *)0, NULL);
	Tmr_1s = osTimerNew(tmr_1s_Callback, osTimerPeriodic, (void *)0, NULL);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	static MSGQUEUE_OBJ_RGB msg;
	static osStatus_t status;
	i = 11;
	while(1){
		status = osMessageQueueGet(mid_MsgQueue_rgb, &msg, NULL, osWaitForever);
		if (status == osOK){
			if((msg.estado_rgb == ON) && (!osTimerIsRunning(Tmr_25ms))){
				osTimerStart(Tmr_25ms, 25U);
			}else if((msg.estado_rgb == OFF) && (osTimerIsRunning(Tmr_25ms))){
				osTimerStop(Tmr_25ms);
			}
			
			if((msg.estado_led2 == ON) && (!osTimerIsRunning(Tmr_1s))){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				osTimerStart(Tmr_1s, 1000U);
			}else if((msg.estado_led2 == OFF) && (osTimerIsRunning(Tmr_1s))) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				osTimerStop(Tmr_1s);
			}
			
			if(msg.estado_led3 == ON){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				osTimerStart(Tmr_350ms, 350U);
			}
		}
	}
}

