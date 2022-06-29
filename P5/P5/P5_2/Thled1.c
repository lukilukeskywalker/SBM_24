#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

osThreadId_t tid_ThLed1;                        // thread id
 
void ThLed1 (void *argument);                   // thread function
 
int Init_ThLed1 (void) {
 
  tid_ThLed1 = osThreadNew(ThLed1, NULL, NULL);
  if (tid_ThLed1 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed1 (void *argument) {
	
	// esta repetido en el main??? Init_ThLed1();
	
  while (1) {
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		osDelay(200);//similar al HAL_Delay(ms)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		osDelay(800);
    osThreadYield();                            // suspend thread
  }
}
