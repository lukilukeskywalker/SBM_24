#include "com_pc.h"

#define COM_PC_STARTUP (1 << 16)
#define COM_PC_TIMEOUT (1 << 17)

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART3;

static com_pc_handle_t com_pc_handle = {0};

typedef enum{
     COM_UNINITIALIZED = 0,
     COM_RECEIVE_HEADER = 1,
     COM_RECEIVE_PAYLOAD = 2,

}com_state_t;

static void com_pc_cb(uint32_t event){
     static com_segment_t com_segment = {0};
     static com_state_t com_state = COM_RECEIVE_HEADER;
     if(event & ARM_USART_EVENT_TX_COMPLETE) {
		osThreadFlagsSet(com_pc_handle.com_task_id, ARM_USART_EVENT_TX_COMPLETE);
	}
     if(!((event & ARM_USART_EVENT_RECEIVE_COMPLETE) | (event & 0xffff0000)))
          return;
     // At this point we have received a complete transmision
     switch(com_state){
          case COM_RECEIVE_HEADER:
               if(com_segment.soh != START_OF_HEADER)
                    USARTdrv->Receive(&com_segment, 3);
               else{
                    com_segment.data = calloc(com_segment.len - 3, sizeof(uint8_t)); //El comienzo de la trama ya la tenemos
                    USARTdrv->Receive(com_segment.data, com_segment.len - 3);
                    com_state = COM_RECEIVE_PAYLOAD;
                    //  @todo Add timeout timer
               }
               break;
          case COM_RECEIVE_PAYLOAD:
               if(com_segment.data[com_segment.len - 4] == END_OF_TRANSMISSION){     // In this case timeout didnt happen, there is a valid message
                    com_segment.len--;  //The rest of the code doesnt need to know that we added the end of transmision byte to the data.
                    osMessageQueuePut(com_pc_handle.com_ingoing_messagequeue, &com_segment.cmd, 0U, 0U);
               }else if(com_segment.data != NULL){     // In this case or a timeout did happen or the data is not valid, free data segment
                    free(com_segment.data);  
               }
               com_segment.soh = 0;
               com_state = COM_RECEIVE_HEADER;
               USARTdrv->Receive(&com_segment, 3);
               break;
					default:					//<-Puedo pulirlo mas. Luego @todo
						USARTdrv->Receive(&com_segment, 3);
						com_state = COM_RECEIVE_HEADER;
						break;
								 
     }
     return;   // If the upper thing works (It should... in freertos it does... cmsis has less funtionality to be honest) I will do a toast (?) for myself
          // The compiler should Optimize this out:

     if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
		osThreadFlagsSet(com_pc_handle.com_task_id, ARM_USART_EVENT_RECEIVE_COMPLETE); //Le paso lo que ha sucedido
	}
	if(event & ARM_USART_EVENT_TX_COMPLETE) {
		//Esta seccion es para evitar que se produzca un bottleneck en las transmisiones, cuando se intenta escribir
		//el buufer de tx mientras que se esta produciendo otra tx
		//v1
		osThreadFlagsSet(com_pc_handle.com_task_id, ARM_USART_EVENT_TX_COMPLETE);
		//V2
		//osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_TX_COMPLETE);
		//osThreadFlagsSet(tid_pc_com_Thread, BLOCK_TX_BUFFER);
	}
}
static int writeUsart(uint8_t *msg, uint32_t size) {
    if (osSemaphoreAcquire(com_pc_handle.usartSemaphore, osWaitForever) == osOK) {
        USARTdrv->Send(msg, size);
        uint32_t flags = osThreadFlagsWait(ARM_USART_EVENT_TX_COMPLETE, osFlagsWaitAny, 1000U);
        osSemaphoreRelease(com_pc_handle.usartSemaphore);
    } else {
        return -1;
    }
}
static system_err_t conf_USART3(void){
	USARTdrv->Initialize(com_pc_cb);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
										ARM_USART_DATA_BITS_8				|
										ARM_USART_PARITY_NONE				|
										ARM_USART_STOP_BITS_1				|
										ARM_USART_FLOW_CONTROL_NONE,
										115200);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	return 0;
}
static void com_pc_task(void *arg){
     uint8_t *buf;
     com_message_t com_message_outgoing;
     static osStatus_t osstatus;
     /*
     while(1){
          while(1){
               USARTdrv->Receive(&sbm_segment, 3);
               osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);
               if(sbm_segment.soh != START_OF_HEADER)
                    break;    //Only breaks first loop... Can we get it without a indentation more (without using goto)? Posiible Restructure?
          }
     }
     WIth the receive functionality handle solely by the usart callback, i can now use this task to perform asyncronous transmit functionality
     */
     while(1){
          osstatus = osMessageQueueGet(com_pc_handle.com_outgoing_messagequeue, &com_message_outgoing, NULL, 1000U);
          if(osstatus == osOK){
               buf = calloc(com_message_outgoing.len + 4, sizeof(uint8_t));
               buf[0] = START_OF_HEADER;
               buf[1] = 0xFF - com_message_outgoing.cmd;
               buf[2] = com_message_outgoing.len + 4;
               memcpy(buf+3, com_message_outgoing.data, com_message_outgoing.len);
               buf[com_message_outgoing.len + 3] = END_OF_TRANSMISSION;
               writeUsart(buf, com_message_outgoing.len + 4);
          }
     }
    
}
system_err_t init_com_pc(osMessageQueueId_t com_ingoing_messagequeue, osMessageQueueId_t com_outgoing_messagequeue){
     if(com_pc_handle.com_task_id != NULL)
          return sys_err_invalid_state;
     if((com_ingoing_messagequeue == NULL) | (com_outgoing_messagequeue == NULL))
          return sys_err_invalid_arg;
     com_pc_handle.com_ingoing_messagequeue = com_ingoing_messagequeue;
     com_pc_handle.com_outgoing_messagequeue = com_outgoing_messagequeue;
     conf_USART3();      // Init USART3
     com_pc_handle.com_task_id = osThreadNew(com_pc_task, NULL, NULL);
     com_pc_handle.usartSemaphore = osSemaphoreNew(1, 1, NULL);
     if(com_pc_handle.com_task_id == NULL)
          return sys_fail;
     osThreadSetPriority(com_pc_handle.com_task_id, osPriorityRealtime7);
     return sys_ok;
}