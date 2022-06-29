#include "Thread.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
int ciclosLed1 = 1;

osThreadId_t tid_ThLed1;
extern osThreadId_t tid_ThLed2;  

void ThLed1 (void *argument);                   // thread function
 
int Init_ThLed1 (void) {
 
  tid_ThLed1 = osThreadNew(ThLed1, NULL, NULL);
  if (tid_ThLed1 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed1 (void *argument) {	
	
  while (1) {
		
		ciclosLed1++;//cuento hasta cinco ciclos del led 1 para iniciar el led dos 
		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		osDelay(200);//similar al HAL_Delay(ms)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		osDelay(800);
		
		
		if(ciclosLed1 == 5){//cuando se cumplen los cinco ciclos de encendido, mamnda el flag al led 2 para que se encienda con la señal 0x01
			osThreadFlagsSet (tid_ThLed2, 0x01);  // el flag 0x01 = 0001 en binario (coge el hilo 1)
		}
		
    osThreadYield();                            // suspend thread
  }
}
