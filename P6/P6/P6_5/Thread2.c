#include "Thjoy.h"
/*----------------------------------------------------------------------------
*              Thread 2: escribe por pantalla en el LCD
 *---------------------------------------------------------------------------*/
extern osMessageQueueId_t mid_MsgQueue;  

osThreadId_t tid_Thread2;                        // thread id
 
int Init_Thread2 (void) {
 
  tid_Thread2 = osThreadNew(Thread2, NULL, NULL);
  if (tid_Thread2 == NULL) {
    return(-1);
  }
 
  return(0);
}

typedef struct {                                // object data type
  uint8_t Buf[32];
  uint8_t Idx;
} MSGQUEUE_OBJ_t;
MSGQUEUE_OBJ_t msg_recibido;

osStatus_t status;

void Thread2 (void *argument) {
	
  while (1) {
		status=osMessageQueueGet(mid_MsgQueue, &msg_recibido, NULL, osWaitForever);
		if(status == osOK){
			if(msg_recibido.Buf[0] == 1){//Arriba
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				LCD_Clean();
				Pintar_Arriba();
				LCD_update();
			}
			if(msg_recibido.Buf[0] == 2){//Derecha
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				LCD_Clean();
				Pintar_Derecha();
				LCD_update();
			}
			if(msg_recibido.Buf[0] == 4){//Abajo
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				LCD_Clean();
				Pintar_Abajo();
				LCD_update();
			}
			if(msg_recibido.Buf[0] == 8){//Izquierda
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				LCD_Clean();
				Pintar_Izquierda();
				LCD_update();
			}
			if(msg_recibido.Buf[0] == 16){//Centro
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				LCD_Clean();
				Pintar_Centro();
				LCD_update();
			}
		}
		osThreadYield();                            // suspend thread
  }
}
