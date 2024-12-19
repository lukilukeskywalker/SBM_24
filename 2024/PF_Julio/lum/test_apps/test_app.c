#include "test_app.h"
#include "../lum.h"
#include "../../stm_err.h"
#include "cmsis_os2.h"

#define MSQ_QUEUE_SIZE 20

static osThreadId_t *test_app_th_id = NULL;
/*
typedef struct{
	
}test_thread_args_t;
*/

static const osMessageQueueAttr_t tb_msg_queue_attr = {
	.name = "tb_msg_queue",
	.attr_bits = 0,
	.cb_mem = NULL, 	//!< memory for control block. Pointer to a memory for the message queue control block object
	.cb_size = 0, 		//!< sizeof of provided memory for control block
	.mq_mem = NULL, 	//!< memory for data storage. Pointer to a memory loc for the message queue data
	.mq_size = 0		//!< size of provided memory for data storage
};

void test_thread(void *args){
	osMessageQueueId_t *tb_msg_queue = (osMessageQueueId_t *)args;
	sample_luminance_t sample_lum = {0};
	osStatus_t status;
	while(1){
		
		status = osMessageQueueGet(tb_msg_queue, &sample_lum, NULL, 2000);
		if(status != osOK){
			STM_LOGE("lum_test", "Se ha producido un error. osMessageQueueGet ha devuelto: %d", status);
			osDelay(1000);
			osThreadExit();
		}
		STM_LOGI("lum_test", "Luminancia medida: %d", sample_lum.luminance);
		
	}
}

stm_err_t init_test_app(void){

	stm_err_t ret = STM_OK;
	osMessageQueueId_t tb_msg_queue = osMessageQueueNew(MSQ_QUEUE_SIZE, sizeof(sample_luminance_t), &tb_msg_queue_attr);
	ret = init_lum_proc(tb_msg_queue);
	test_app_th_id = osThreadNew(test_thread, tb_msg_queue, NULL);
	STM_RETURN_ON_FALSE(test_app_th_id != NULL, STM_ERR_NOT_FINISHED, __func__, "Thread creation failed");
	return ret;
}
