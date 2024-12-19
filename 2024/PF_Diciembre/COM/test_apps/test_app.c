#include "test_app.h"
#include "../com.h"
#include "utils/stm_err.h"
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"

#define MSQ_QUEUE_SIZE 20

static osThreadId_t *test_app_th_id = NULL;

typedef struct{
	osMessageQueueId_t *rxqueueId;
	osMessageQueueId_t *txqueueId;
}test_thread_args_t;


static const osMessageQueueAttr_t tb_msg_queue_attr = {
	.name = "tb_msg_queue",
	.attr_bits = 0,
	.cb_mem = NULL, 	//!< memory for control block. Pointer to a memory for the message queue control block object
	.cb_size = 0, 		//!< sizeof of provided memory for control block
	.mq_mem = NULL, 	//!< memory for data storage. Pointer to a memory loc for the message queue data
	.mq_size = 0		//!< size of provided memory for data storage
};

void test_thread(void *args){
	test_thread_args_t *test_thread_args = (test_thread_args_t *)args;
	com_msg_t com_msg = {
		.operation = 0x42,
		.size = 1,
		.data[0] = 0x2a
	};
	osStatus_t status;
	
	osMessageQueuePut(test_thread_args->txqueueId, &com_msg, NULL, NULL);
	while(1){
		
		status = osMessageQueueGet(test_thread_args->rxqueueId, &com_msg, NULL, osWaitForever);
		if(status != osOK){
			STM_LOGE("com_test", "Se ha producido un error. osMessageQueueGet ha devuelto: %d", status);
			osDelay(1000);
			osThreadExit();
		}
		STM_LOGI("com_test", "Comando recibido: 0x%x, Tamano: 0x%x, Primer byte: 0x%x", com_msg.operation, com_msg.size, com_msg.data);
		osMessageQueuePut(test_thread_args->txqueueId, &com_msg, NULL, NULL);
		
	}
}

stm_err_t init_test_app(void){

	stm_err_t ret = STM_OK;
	osMessageQueueId_t tb_rxmsg_queue = osMessageQueueNew(MSQ_QUEUE_SIZE, sizeof(com_msg_t), &tb_msg_queue_attr);
	osMessageQueueId_t tb_txmsg_queue = osMessageQueueNew(MSQ_QUEUE_SIZE, sizeof(com_msg_t), &tb_msg_queue_attr);
	ret = init_com_proc(tb_rxmsg_queue, tb_txmsg_queue);
	test_thread_args_t *test_thread_args = calloc(1, sizeof(test_thread_args_t));
	test_thread_args->rxqueueId = tb_rxmsg_queue;
	test_thread_args->txqueueId = tb_txmsg_queue;
	test_app_th_id = osThreadNew(test_thread, test_thread_args, NULL);
	STM_RETURN_ON_FALSE(test_app_th_id != NULL, STM_ERR_NOT_FINISHED, __func__, "Thread creation failed");
	return ret;
}
