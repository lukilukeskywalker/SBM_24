#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "com.h"
#include "stdio.h"
#include "string.h"

#define INIT_DATAGRAM 0x01
#define END_DATAGRAM 0xFE
#define BLOCK_TX_BUFFER 1<<31 

void Th_com (void *argument); 
void USART3_callback(uint32_t event);

osThreadId_t tid_pc_com_Thread;
osMessageQueueId_t mid_MsgQueue_com_rx;			//Hilo para recibir del PC al micro
osMessageQueueId_t mid_MsgQueue_com_tx;			//Hilo para transmitir del micro al pc

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART3;
void USART3_callback(uint32_t event){
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
		osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_RECEIVE_COMPLETE); //Le paso lo que ha sucedido
	}
	if(event & ARM_USART_EVENT_TX_COMPLETE) {
		//Esta seccion es para evitar que se produzca un bottleneck en las transmisiones, cuando se intenta escribir
		//el buufer de tx mientras que se esta produciendo otra tx
		//v1
		osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_TX_COMPLETE);
		//V2
		//osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_TX_COMPLETE);
		//osThreadFlagsSet(tid_pc_com_Thread, BLOCK_TX_BUFFER);
	}
}
static int writeUsart(uint8_t *msg, uint32_t size){
	//V1
	uint8_t done = false;
	uint32_t flags;
	do{
		if((BLOCK_TX_BUFFER&osThreadFlagsGet())==NULL){
			osThreadFlagsSet(tid_pc_com_Thread, BLOCK_TX_BUFFER);
			USARTdrv->Send(msg, size);
			flags=osThreadFlagsWait(ARM_USART_EVENT_TX_COMPLETE, osFlagsWaitAny, 1000U);
			osThreadFlagsClear(BLOCK_TX_BUFFER);	//Liberamos para que lo pueda volver a intentar
			if(flags==ARM_USART_EVENT_TX_COMPLETE){
				done=true;
				return 0;
			}
			
		}
	}while(!done);
//V2
	//static uint32_t flags = osThreadFlagsGet();
//	if(((BLOCK_TX_BUFFER&osThreadFlagsGet())==true) && ((ARM_USART_EVENT_TX_COMPLETE&osThreadFlagsGet())==false)){
//		//Si entra aqui, quiere decir que una transmision se inicio en otro momento, pero todavia no se completo
//		osThreadFlagsWait(ARM_USART_EVENT_TX_COMPLETE, osFlagsWaitAny, 1000U);
//	}
//	osThreadFlagsSet(tid_pc_com_Thread, BLOCK_TX_BUFFER);
//	USARTdrv->Send(msg, size);
}
void writeUsartln(uint8_t *msg, uint32_t size){
	writeUsart(msg, size);
	writeUsart("\n\r", 2);
}


static int conf_USART3(void){
	USARTdrv->Initialize(USART3_callback);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
										ARM_USART_DATA_BITS_8				|
										ARM_USART_PARITY_NONE				|
										ARM_USART_STOP_BITS_1				|
										ARM_USART_FLOW_CONTROL_NONE,
										19200);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	//writeUsart("\n\rHola mundo\n\r",14); //No podemos usar flags si el OS no a arrancado
	USARTdrv->Send("\n\rHola mundo\n\r", 14);
	return 0;
}

int Init_MsgQueue_com(void){
	mid_MsgQueue_com_rx = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_COM), NULL);
	mid_MsgQueue_com_tx = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_COM), NULL);
	if((mid_MsgQueue_com_rx == NULL) && (mid_MsgQueue_com_tx == NULL)){
		return -1;
	}
	return 0;
}
int Init_Th_com (void) {
	conf_USART3();
  tid_pc_com_Thread = osThreadNew(Th_com, NULL, NULL);
	osThreadSetPriority(tid_pc_com_Thread, osPriorityRealtime7);
  if (tid_pc_com_Thread == NULL) {
    return(-1);
  }
  return(Init_MsgQueue_com());
}
 
void Th_com (void *argument) {
	uint8_t cmd[64];
	uint8_t operation;
	uint8_t size;
	static MSGQUEUE_OBJ_COM msg_rx;
	static MSGQUEUE_OBJ_COM msg_tx;
	static osStatus_t queue_status;
	
	while(1){
		USARTdrv->Receive(&cmd, 3);	
		//Recibimos 3 bytes.
			//1º Indica inicio de trama 0x01
			//2º Indica tipo de operacion , opciones: 0x20, 0x25, 0x35, 0x70, 0x40, 0x50, 0x55, 0x60
			//3º Indica tamaño del resto de la trama
		osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);
		if(cmd[0] == INIT_DATAGRAM){	//&&((proc_flags & ARM_USART_EVENT_RECEIVE_COMPLETE)!= 0)
			//Solo entramos si el evento de recepcion se ha producido correctamente. 
			//Esto evita que si se produzca otra condicion se quede bloqueado
			operation = cmd[1];
			size = (cmd[2]-4);
			USARTdrv->Receive(&cmd, size+1);
			osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, 500U);//osWaitForever);	
			//Esperamos la condicion de haber llenado el buffer. 
			if(cmd[size] == END_DATAGRAM){
				//writeUsartln("Hemos entrado en 1", 18);
				
				msg_rx.op = operation;
				memcpy(msg_rx.data, cmd, size);
				msg_rx.size = size;	//Guardamos el tamaño del mensaje
				queue_status=osMessageQueuePut(mid_MsgQueue_com_rx, &msg_rx, 0U, 0U);
				//Esperamos respuesta de principal
				cmd[0]=INIT_DATAGRAM;
				do{
					//writeUsartln("2-Hemos entrado en 2", 20);
					queue_status=osMessageQueueGet(mid_MsgQueue_com_tx, &msg_tx, NULL, 1000U);	//
					
					//Puede volver por: osOK, osError, osErrorTimeout, y otros que no importan tanto
					if(queue_status == osOK){
						cmd[1] =0xFF-msg_tx.op;
						cmd[2] = msg_tx.size+4;
						memcpy(cmd+3, msg_tx.data, msg_tx.size);
						cmd[msg_tx.size+3] = END_DATAGRAM;
						writeUsart(cmd, msg_tx.size+4);//USARTdrv->Send(cmd, msg_tx.size+4);
#ifdef USART_NEW_LINE
						writeUsart("\n\r", 2);
#endif
					}
					if(queue_status == osErrorTimeout){
						writeUsartln("ERROR, principal no ha respondido en el tiempo preestablecido", 61);
					}
					if(queue_status == osError){
						writeUsartln("ERROR en cola de mensajes", 25);
					}
					
				}while((queue_status == osOK) && (msg_tx.last_msg != true)); ///&& osMessageQueueGetCount(mid_MsgQueue_com_tx)!=0));	
				//Problema aqui, debemos esperar a que el sistema haya enviado todos los paq, pero que la cola este vacia,
				//No quiere decir que el principal haya finalizado.
				//Asi que el principal debe dar una señal a com, para indiar de que ha finalizado
			}
		}
	}
}



