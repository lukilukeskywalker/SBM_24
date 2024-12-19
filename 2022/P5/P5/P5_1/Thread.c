#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_ThLed1;                        //variable de identificación del thread 
 
void ThLed1 (void *argument);                   // funcion del thread 
 
int Init_ThLed1 (void) {//funcion de inicialización del thread
 
  tid_ThLed1 = osThreadNew(ThLed1, NULL, NULL);//genera el nuevo thread
  if (tid_ThLed1 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLed1 (void *argument) {	// realiza la acción que quiero conseguir con el thread gernerado que en este caso es encender y apagar el led
  while (1) {
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		osDelay(200);//similar al HAL_Delay(ms)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		osDelay(800);
    osThreadYield();                            // suspend thread
  }
}
