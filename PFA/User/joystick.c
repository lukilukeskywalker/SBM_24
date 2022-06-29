#include "joystick.h"

#define REBOTE_FLAG 0X01
#define CICLOS_FLAG 0X02
#define IRQ_FLAG    0x04

osThreadId_t tid_Th_joy;
osMessageQueueId_t mid_MsgQueue_joy;
static MSGQUEUE_OBJ_JOY msg;

static osTimerId_t Tmr_rebote_50ms;
static osTimerId_t Tmr_ciclos_50ms;
static uint32_t flags;
uint8_t valida = 0;
uint8_t ciclos = 0;

void Th_joy(void *argument);

static void conf_pinout (void) {
	GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void tmr_rebote_Callback (void* argument) {
	osThreadFlagsSet(tid_Th_joy, REBOTE_FLAG);
}

static void tmr_ciclos_Callback (void* argument) {
  osThreadFlagsSet(tid_Th_joy, CICLOS_FLAG);
}

int Init_MsgQueue_joy (void) {
  mid_MsgQueue_joy = osMessageQueueNew(MSGQUEUE_OBJECTS_JOY, sizeof(MSGQUEUE_OBJ_JOY), NULL);
  if (mid_MsgQueue_joy == NULL)
    return (-1); 
  return(0);
}

int Init_Th_joy(void) {
  tid_Th_joy = osThreadNew(Th_joy, NULL, NULL);
  if (tid_Th_joy == NULL) {
    return(-1);
  }
	conf_pinout();
	Init_MsgQueue_joy();
  return(0);
}

void Th_joy (void *argument) {
	Tmr_rebote_50ms = osTimerNew(tmr_rebote_Callback, osTimerOnce, (void *)0, NULL);
	Tmr_ciclos_50ms = osTimerNew(tmr_ciclos_Callback, osTimerPeriodic, (void *)0, NULL);
	while(1) {
		flags = osThreadFlagsWait((CICLOS_FLAG | REBOTE_FLAG | IRQ_FLAG), osFlagsWaitAny, osWaitForever);
		if(flags & IRQ_FLAG){
			osTimerStart(Tmr_rebote_50ms, 50U);
		}
		if(flags & REBOTE_FLAG){
			switch (msg.tecla){
				case Arriba:
					if(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
						valida = 1;
						ciclos = 0;
						osTimerStart(Tmr_ciclos_50ms, 50U);
					} 
				break;
				
				case Abajo:
					if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
						valida = 1;
						ciclos = 0;
						osTimerStart(Tmr_ciclos_50ms, 50U);
					} 
				break;
			}
		}
		if(flags & CICLOS_FLAG){
			switch (msg.tecla){
				case Arriba:
					if(ciclos == 29){
						ciclos = 0;
						msg.duracion = Larga;
						osTimerStop(Tmr_ciclos_50ms);
						osMessageQueuePut(mid_MsgQueue_joy, &msg, 0U, 0U);
					}else if(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
						ciclos ++;
					} 
					else if(valida == 1){
						msg.duracion = Corta;
						osTimerStop(Tmr_ciclos_50ms);
						osMessageQueuePut(mid_MsgQueue_joy, &msg, 0U, 0U);
					}
				break;
				
				case Abajo:
					if(ciclos == 29){
						ciclos = 0;
						msg.duracion = Larga;
						osTimerStop(Tmr_ciclos_50ms);
						osMessageQueuePut(mid_MsgQueue_joy, &msg, 0U, 0U);
					}else if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
						ciclos ++;
					} 
					else if(valida == 1){
						msg.duracion = Corta;
						osTimerStop(Tmr_ciclos_50ms);
						osMessageQueuePut(mid_MsgQueue_joy, &msg, 0U, 0U);
					}
				break;
			}
		}
    osThreadYield();                            
  }
}

void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); //Arriba
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); //Abajo
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	valida = 0;
	if(GPIO_Pin == GPIO_PIN_10){
		msg.tecla = Arriba;
	}
	if(GPIO_Pin == GPIO_PIN_12){
		msg.tecla = Abajo;		
	}
	osThreadFlagsSet(tid_Th_joy, IRQ_FLAG);
}

