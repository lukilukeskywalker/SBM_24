#include "Thjoy.h"

/*----------------------------------------------------------------------------
 *      Message Queue creation & usage
 *---------------------------------------------------------------------------*/
 
 
typedef struct {                                // object data type
  uint8_t Buf[32];
  uint8_t Idx;
} MSGQUEUE_OBJ_t;
MSGQUEUE_OBJ_t msg_enviado;

osMessageQueueId_t mid_MsgQueue;                // message queue id
 
int Init_MsgQueue (void) {
 
  mid_MsgQueue = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_t), NULL);
  if (mid_MsgQueue == NULL) {
    ; // Message Queue object not created, handle failure
  }
 
  return(0);
}



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
		msg_enviado.Buf[0] = 1;
		osMessageQueuePut(mid_MsgQueue, &msg_enviado, 0U, 0U);//pongo el mensaje a la cola
		
	}
	else if(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_11) == GPIO_PIN_SET ){//Pulsado el derecho
		msg_enviado.Buf[0] = 2;
		osMessageQueuePut(mid_MsgQueue, &msg_enviado, 0U, 0U);//pongo el mensaje a la cola
	}
	else if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_12) == GPIO_PIN_SET ){//Pulsado abajo
		msg_enviado.Buf[0] = 4;
		osMessageQueuePut(mid_MsgQueue, &msg_enviado, 0U, 0U);//pongo el mensaje a la cola
	}
	else if( HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){//Pulsado izquierda
		msg_enviado.Buf[0] = 8;
		osMessageQueuePut(mid_MsgQueue, &msg_enviado, 0U, 0U);//pongo el mensaje a la cola
	}
	else if(HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){//Pulsado centro
		msg_enviado.Buf[0] = 16;
		osMessageQueuePut(mid_MsgQueue, &msg_enviado, 0U, 0U);//pongo el mensaje a la cola
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

