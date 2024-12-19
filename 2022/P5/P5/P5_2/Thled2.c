#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
 
/*----------------------------------------------------------------------------
 *      Thread 2 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

osThreadId_t tid_ThLed2;                        // thread id
 
void ThLed2 (void *argument);                   // thread function
 
int Init_ThLed2 (void) {
 
  tid_ThLed2 = osThreadNew(ThLed2, NULL, NULL);
  if (tid_ThLed2 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed2 (void *argument) {
	
  while (1) {
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		osDelay(137);//similar al HAL_Delay(ms)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		osDelay(137);
    osThreadYield();                            // suspend thread
  }
}
