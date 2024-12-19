#include "Thread.h"
 
/*----------------------------------------------------------------------------
 *      Thread 3 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
int ciclosLed3 = 1; 

extern osThreadId_t tid_ThLed2;   
osThreadId_t tid_ThLed3;                        
 
void ThLed3 (void *argument);                   // thread function
 
int Init_ThLed3 (void) {
 
  tid_ThLed3 = osThreadNew(ThLed3, NULL, NULL);
  if (tid_ThLed3 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed3 (void *argument) {
	osThreadFlagsWait (0x02, osFlagsWaitAny , osWaitForever);
  while (1) {
		
		ciclosLed3++;

		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		osDelay(287);//similar al HAL_Delay(ms)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		osDelay(287);
		
		if(ciclosLed3 == 25){
			osThreadFlagsSet (tid_ThLed2, 0x04); 
		}
		
		if ( ciclosLed3 ==30){
			osThreadFlagsWait (0x02, osFlagsWaitAny , osWaitForever);// 0x02 = 0010 en código binario, es decir, el hilo 1
			ciclosLed3 = 1;
		}
		
    osThreadYield();                            // suspend thread
  }
}
