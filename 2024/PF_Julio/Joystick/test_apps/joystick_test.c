#include "joystick_test.h"
#include "../../stm_err.h"
#include "cmsis_os2.h"
#include "../joystick.h"

#define MSG_QUEUE_SIZE 5

static osThreadId_t *test_app_th_id = NULL;

const char* joy_pulse_strings[] = {
    "JOY_UP_SPULSE",
    "JOY_RIGHT_SPULSE",
    "JOY_DOWN_SPULSE",
    "JOY_LEFT_SPULSE",
    "JOY_CENTER_SPULSE",
    "JOY_UP_LPULSE",
    "JOY_RIGHT_LPULSE",
    "JOY_DOWN_LPULSE",
    "JOY_LEFT_LPULSE",
    "JOY_CENTER_LPULSE",
    "JOY_ERR"
};

void test_thread(void *args){
	osMessageQueueId_t *tb_msg_queue = (osMessageQueueId_t *)args;
	joy_pulse_t joy_msg = {0};
	osStatus_t status;
	while(1){
		status = osMessageQueueGet(tb_msg_queue, &joy_msg, NULL, osWaitForever);
		if(status != osOK){
			STM_LOGE("joy_test", "Se ha producido un error. osMessageQueueGet ha devuelto: %d", status);
			osDelay(1000);
			//osThreadExit();
		}
		STM_LOGI("joy_test", "Valor devuelto: %d %s", joy_msg, joy_pulse_strings[joy_msg - 1]);
		//STM_LOGI("joy_test", "Valor devuelto: %d", joy_msg);
		
	}
		
}
stm_err_t init_test_app(void){
	stm_err_t ret = STM_OK;

	osMessageQueueId_t joy_queue_id = osMessageQueueNew(MSG_QUEUE_SIZE, sizeof(joy_event_t), NULL);
	STM_RETURN_ON_FALSE(init_joystick_proc(joy_queue_id)== STM_OK, STM_FAIL, __func__, "Init Joystick failed");

	test_app_th_id = osThreadNew(test_thread, joy_queue_id, NULL);
	STM_RETURN_ON_FALSE(test_app_th_id != NULL, STM_ERR_NOT_FINISHED, __func__, "Thread creation failed");
	return ret;
}