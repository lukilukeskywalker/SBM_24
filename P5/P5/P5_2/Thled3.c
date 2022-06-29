#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
 
/*----------------------------------------------------------------------------
 *      Thread 3 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

osThreadId_t tid_ThLed3;                        // thread id
 
void ThLed3 (void *argument);                   // thread function
 
int Init_ThLed3 (void) {
 
  tid_ThLed3 = osThreadNew(ThLed3, NULL, NULL);
  if (tid_ThLed3 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed3 (void *argument) {
	
  while (1) {
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		osDelay(287);//similar al HAL_Delay(ms)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		osDelay(287);
    osThreadYield();                            // suspend thread
  }
}
