#include "test_app.h"
#include "../../stm_err.h"
#include "cmsis_os2.h"
#include "../pot.h"

#define MSG_QUEUE_SIZE 5

static osThreadId_t *test_app_th_id = NULL;

void test_thread(void *args){
	osMessageQueueId_t *tb_msg_queue = (osMessageQueueId_t *)args;
	pot_msg_t pot_msg = {0};
	osStatus_t status;
	while(1){
		status = osMessageQueueGet(tb_msg_queue, &pot_msg, NULL, 2000);
		if(status != osOK){
			STM_LOGE("pot_test", "Se ha producido un error. osMessageQueueGet ha devuelto: %d", status);
			osDelay(1000);
			osThreadExit();
		}
		STM_LOGI("pot_test", "Valores potenciometros medidos: POT0: %d, POT1: %d", pot_msg.pot[0], pot_msg.pot[1]);
		
	}
		
}
stm_err_t init_test_app(void){
	stm_err_t ret = STM_OK;
	osMessageQueueId_t tb_msg_queue =  osMessageQueueNew(MSG_QUEUE_SIZE, sizeof(pot_msg_t), NULL);
	ret = init_pot_proc(tb_msg_queue);
	test_app_th_id = osThreadNew(test_thread, tb_msg_queue, NULL);
	STM_RETURN_ON_FALSE(test_app_th_id != NULL, STM_ERR_NOT_FINISHED, __func__, "Thread creation failed");
	return ret;
}
