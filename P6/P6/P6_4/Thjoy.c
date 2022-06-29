#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
/*----------------------------------------------
 *                    Timer 
 *----------------------------------------------*/ 
 
 int cnt = 0;//cuenta las veces de pulsacion
 
/*----- One-Shoot Timer -----*/
osTimerId_t tim_id1;                            // timer id
static uint32_t exec1;                          // argument for the timer call back function

// One-Shoot Timer Function
static void Timer1_Callback (void const *arg) {
  // add user code here
	if( HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_10) == GPIO_PIN_SET ){//Pulsado arriba
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
		cnt++;
	}
	else if(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_11) == GPIO_PIN_SET ){//Pulsado el derecho
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
		cnt++;
	}
	else if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_12) == GPIO_PIN_SET ){//Pulsado abajo
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
		cnt++;
	}
	else if( HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){//Pulsado izquierda
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
		cnt++;
	}
	else if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){//Pulsado centro
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
		cnt++;
	}
}

 
// Create timers
int Init_Timers (void) {
  // Create one-shoot timer
  exec1 = 1U;
  tim_id1 = osTimerNew((osTimerFunc_t)&Timer1_Callback, osTimerOnce, &exec1, NULL);
  return NULL;
}

 
/*----------------------------------------------------------------------------
 *                                Thread 1
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thread;                        // thread id
 
void Thread (void *argument);                   // thread function
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
	
  while (1) {
    osThreadFlagsWait (0x04, osFlagsWaitAny , osWaitForever);
		osTimerStart(tim_id1, 50U); //cuenta de 50us para los rebotes
    osThreadYield();                            // suspend thread
  }
}

/*----------------------------------------------------------------------------
 *                       Definición Joystick
 *---------------------------------------------------------------------------*/
extern GPIO_InitTypeDef GPIO_InitStruct;
void Init_Joystick(void){
	//JOYSTICK
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;	//Detecta los flancos de subida
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;//Derecho
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_10;//Arriba
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;//Abajo
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_14;//Izquierda
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_15;//Centro
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);		//Habilitar tratado e interrupciones de la HAL
}
