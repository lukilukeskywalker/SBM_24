
#include "modLeds.h"                         
//These variables are only accessible from this module 
osThreadId_t tid_Th_Leds;
osMessageQueueId_t mid_MsgQueueLeds;

osMessageQueueId_t getModLedQueueID(void);
int Init_Thread_Leds (void);
int Init_MsgQueue_Leds (void);
void Thread_Leds(void *argument);

void initModLeds(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  
  __HAL_RCC_GPIOB_CLK_ENABLE();

  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_RESET);
	
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	Init_Thread_Leds();
	Init_MsgQueue_Leds();

} 
int Init_Thread_Leds (void) {
 
  tid_Th_Leds = osThreadNew(Thread_Leds, NULL, NULL);
  if (tid_Th_Leds == NULL) {
    return(-1);
  }
 
  return(0);
}


	

int Init_MsgQueue_Leds (void) {
 
  mid_MsgQueueLeds = osMessageQueueNew(SIZE_MSGQUEUE_LEDS , sizeof(MSGQUEUE_LEDS_t), NULL);
  if (mid_MsgQueueLeds == NULL) {
    return (-1);
  }
  return(0);
}
//Gets the queue ID
osMessageQueueId_t getModLedQueueID(void){
	return mid_MsgQueueLeds;
}
 
void Thread_Leds (void *argument) {
	
	MSGQUEUE_LEDS_t localObject;
 
  while (1) {
		
		osMessageQueueGet(mid_MsgQueueLeds, &localObject, NULL, osWaitForever);
		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (localObject.LD1 ? GPIO_PIN_SET : GPIO_PIN_RESET));
		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (localObject.LD2 ? GPIO_PIN_SET : GPIO_PIN_RESET));
		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (localObject.LD3 ? GPIO_PIN_SET : GPIO_PIN_RESET));
		
	}
}

//Código para testear el módulo
void Th_Leds_Test(void *argument); 
 
osThreadId_t tid_Th_Leds_Test;

int Init_Th_Leds_Test(void) {
 
  tid_Th_Leds_Test = osThreadNew(Th_Leds_Test, NULL, NULL);
  if (tid_Th_Leds_Test == NULL) {
    return(-1);
  }	
  return(0);
}
 
void Th_Leds_Test (void *argument) {

	MSGQUEUE_LEDS_t localObject;
	uint8_t contador = 0;
	
	
	
  while (1) {
		
		osDelay(500);
		
		contador++;
		
		localObject.LD1 = contador & 0x01;
		localObject.LD2 = contador & 0x02;
		localObject.LD3 = contador & 0x04;
		
		
		osMessageQueuePut(mid_MsgQueueLeds, &localObject, 0U, 0U);
    		
  }
}
