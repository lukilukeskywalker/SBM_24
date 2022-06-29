#include "Thread.h"

/*----------------------------------------------------------------------------
 *      Thread 2 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
int ciclosLed2=1;

osThreadId_t tid_ThLed2; 
extern osThreadId_t tid_ThLed3;                        

void ThLed2 (void *argument);                   // thread function
 
int Init_ThLed2 (void) {
 
  tid_ThLed2 = osThreadNew(ThLed2, NULL, NULL);
  if (tid_ThLed2 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed2 (void *argument) {
	
	osThreadFlagsWait (0x01, osFlagsWaitAny , osWaitForever);//recibe el flag del led2 e indico que se debe encender
	
  while (1) {
		
		ciclosLed2++;
		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		osDelay(137);//similar al HAL_Delay(ms)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		osDelay(137);
		
		if (ciclosLed2 == 15){//cuando pasan 15 ciclos del led2 manda el flag 0x02 al thread del led 3 para que se active y se encienda
			osThreadFlagsSet (tid_ThLed3, 0x02); 
		}
		
		if( ciclosLed2 == 20){
			osThreadFlagsWait (0x04, osFlagsWaitAny , osWaitForever);// a los 25 ciclos del led se volverá a reactivar porque le llegará el flag 0x04 ( 0100 en codigo binagio, es decir, coge el hilo 2)
			ciclosLed2 = 1;
		}
		
    osThreadYield();                            // suspend thread
  }
	


}
