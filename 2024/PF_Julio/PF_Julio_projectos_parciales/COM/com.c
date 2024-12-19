#include "com.h"

 // Private Includes
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define DEBUG 0
#define _LOG_LEVEL 0

#define SOH 0x01
#define EOT 0xFE

#define UART_EVENT_MASK ( \
				ARM_USART_EVENT_SEND_COMPLETE         | /* 0x001 */ \
				ARM_USART_EVENT_RECEIVE_COMPLETE      | /* 0x002 */ \
				ARM_USART_EVENT_TRANSFER_COMPLETE     | /* 0x004 */ \
				ARM_USART_EVENT_TX_COMPLETE           | /* 0x008 */ \
				ARM_USART_EVENT_TX_UNDERFLOW          | /* 0x010 */ \
				ARM_USART_EVENT_RX_OVERFLOW           | /* 0x020 */ \
				ARM_USART_EVENT_RX_TIMEOUT            | /* 0x040 */ \
				ARM_USART_EVENT_RX_BREAK              | /* 0x080 */ \
				ARM_USART_EVENT_RX_FRAMING_ERROR      | /* 0x100 */ \
				ARM_USART_EVENT_RX_PARITY_ERROR         /* 0x200 */ \
			)

extern ARM_DRIVER_USART Driver_USART3;
static osThreadId_t com_thread_id = 0;
osSemaphoreId_t com_send_semaphore_id = 0; 

typedef struct{
	ARM_DRIVER_USART *usart_drv;
	osMessageQueueId_t *rxqueueId;
	osMessageQueueId_t *txqueueId;
}com_handle_t;
typedef enum{
	COM_DECODER_INIT,
	COM_DECODER_WAIT_HEADER,
	COM_DECODER_WAIT_DATA,
	COM_DECODER_MAX
}com_decoder_t;
static void usart_cb(uint32_t event){
	if(event & (ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE))
		osSemaphoreRelease(com_send_semaphore_id);
	if(event & UART_EVENT_MASK)
		osThreadFlagsSet(com_thread_id, event);
	return;
}
void com_decoder_stmachine(com_handle_t *com_hndl, osStatus_t flags){
	static com_decoder_t com_dc_st = COM_DECODER_MAX;					// At startup decoder state machine goes to default where init usart conf is set
	static uint8_t cmd[64] = {0}, datagram_len = 0;
	static com_msg_t com_msg = {0};
	if((flags & osFlagsError))
		return;			// Es el timeout de los flags
	switch(com_dc_st){
		case COM_DECODER_INIT:
			if(cmd[0] == SOH && (flags & ARM_USART_EVENT_RECEIVE_COMPLETE)){
				com_dc_st = COM_DECODER_WAIT_HEADER;
				com_hndl->usart_drv->Receive(&cmd[1], 2);
			}
			com_hndl->usart_drv->Receive(&cmd, 1);
			break;		// We can omit ARM_USART_EVENT_RX_TIMEOUT, as this is to be expected
		case COM_DECODER_WAIT_HEADER:
			if(!(flags & osFlagsError) && (flags & ARM_USART_EVENT_RECEIVE_COMPLETE)){	
				com_dc_st = COM_DECODER_WAIT_DATA;
				datagram_len = cmd[2];
				com_hndl->usart_drv->Receive(&cmd[3], (datagram_len - 3));
				break;
			}
			//if instead a ARM_USART_EVENT_RX_TIMEOUT gets produced, it should fall to default
		case COM_DECODER_WAIT_DATA:
			if(!(flags & osFlagsError) && (cmd[datagram_len - 1] == EOT)){
				com_msg.operation = cmd[1];
				com_msg.size = datagram_len - 4;
				memcpy(com_msg.data, &cmd[3], (datagram_len - 4));
				osMessageQueuePut(com_hndl->rxqueueId, &com_msg, 0U, 0U);
				STM_LOGI(__func__, "Se ha recibido todo el datagrama");
			}	
			//break; //if instead a ARM_USART_EVENT_RX_TIMEOUT gets produced, it should fall to default
		default:
			com_dc_st = COM_DECODER_INIT;		//Reinit state machine
			datagram_len = 0;
			cmd[0] = 0xFF;
			com_hndl->usart_drv->Receive(&cmd, 1);
			break;
	}

	return;
}
stm_err_t com_send_sbm_msg(com_handle_t *com_hndl){
	static uint8_t cmd[32] = {SOH};
	static com_msg_t com_msg;
	osStatus_t osstatus = osMessageQueueGet(com_hndl->txqueueId, &com_msg, 0U, 0U);
	if(osstatus == osOK){
		osstatus = osSemaphoreAcquire(com_send_semaphore_id, osWaitForever);
		cmd[1] = 0xFF - com_msg.operation;
		cmd[2] = com_msg.size + 4;
		memcpy(cmd+3, &com_msg.data, com_msg.size);
		cmd[com_msg.size + 3] = EOT;
		com_hndl->usart_drv->Send(cmd, cmd[2]);
	}
	
}
void com_thread(void *args){
	com_handle_t *com_hndl = (com_handle_t *)args;
	com_send_semaphore_id = osSemaphoreNew(1U, 1U, NULL);
	static osStatus_t flags = 0;
	while(1){
		com_decoder_stmachine(com_hndl, flags);
		com_send_sbm_msg(com_hndl);
		flags = osThreadFlagsWait(UART_EVENT_MASK, osFlagsWaitAny, 100);//1000);		// ttl set to 1 s

#if DEBUG
		if(flags & (ARM_USART_EVENT_TX_UNDERFLOW | /* 0x010 */ \
				ARM_USART_EVENT_RX_OVERFLOW           | /* 0x020 */ \
				ARM_USART_EVENT_RX_TIMEOUT            | /* 0x040 */ \
				ARM_USART_EVENT_RX_BREAK              | /* 0x080 */ \
				ARM_USART_EVENT_RX_FRAMING_ERROR      | /* 0x100 */ \
				ARM_USART_EVENT_RX_PARITY_ERROR         /* 0x200 */
				) && !(flags & osFlagsError))
			STM_LOGW(__func__, "Se ha producido el siguiente error en usart 0x%x", flags);
#endif
		
	}
}

/** @brief 
*		@note velocidad de 9600 Baudios, 8 bits de datos, 1bit de stop y sin paridad
*/
static stm_err_t conf_usart(ARM_DRIVER_USART *usart_drv){
	__HAL_RCC_GPIOB_CLK_ENABLE();
	usart_drv->Initialize(usart_cb);
	usart_drv->PowerControl(ARM_POWER_FULL);
	usart_drv->Control(ARM_USART_MODE_ASYNCHRONOUS |
				ARM_USART_DATA_BITS_8 |
				ARM_USART_PARITY_NONE |
				ARM_USART_STOP_BITS_1 |
				ARM_USART_FLOW_CONTROL_NONE,
				9800
				);
	usart_drv->Control(ARM_USART_CONTROL_RX, 1);
	usart_drv->Control(ARM_USART_CONTROL_TX, 1);

	return STM_OK;
											
}

stm_err_t init_com_proc(osMessageQueueId_t *rxqueueId, osMessageQueueId_t *txqueueId){
	//static ARM_DRIVER_USART *usart_drv = NULL;
	stm_err_t ret = STM_OK;
	static com_handle_t *com_hndl = NULL;
	STM_RETURN_ON_FALSE(com_hndl == NULL | com_thread_id == NULL, STM_ERR_INVALID_STATE, __func__, "COM already initialized");
	STM_RETURN_ON_FALSE(rxqueueId != NULL | txqueueId != NULL, STM_ERR_INVALID_ARG, __func__, "rx/tx QueueId cant be NULL");
	com_hndl = (com_handle_t *)calloc(1, sizeof(com_handle_t));
	STM_RETURN_ON_FALSE(com_hndl != NULL, STM_ERR_NO_MEM, __func__, "COM handle allocation failed");
	com_hndl->usart_drv = &Driver_USART3;
	com_hndl->rxqueueId = rxqueueId;
	com_hndl->txqueueId = txqueueId;
	conf_usart(com_hndl->usart_drv);
	com_thread_id = osThreadNew(com_thread, com_hndl, NULL);
	
	STM_GOTO_ON_FALSE(com_thread_id != NULL, STM_ERR_NOT_FINISHED, cleanup, __func__, "com thread was not created successfully");
	osThreadSetPriority(com_thread_id, osPriorityRealtime7);
	return STM_OK;
	
	cleanup:
	if(com_hndl != NULL)
		free(com_hndl);
	return ret;
}

