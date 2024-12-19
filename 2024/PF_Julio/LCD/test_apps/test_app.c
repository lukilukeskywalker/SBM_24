#include "test_app.h"
#include "../lcd.h"
#include "../../stm_err.h"
#include "cmsis_os2.h"

#define MSQ_QUEUE_SIZE 20

static osThreadId_t *test_app_th_id = NULL;
/*
typedef struct{
	
}test_thread_args_t;
*/


stm_err_t init_test_app(void){
	stm_err_t ret = STM_OK;
	//osMessageQueueId_t tb_msg_queue = osMessageQueueNew(MSQ_QUEUE_SIZE, sizeof(sample_luminance_t), &tb_msg_queue_attr);
	Init_Th_lcd();
	Init_Th_lcd_test();
	//ret = init_lum_proc(tb_msg_queue);
	//test_app_th_id = osThreadNew(test_thread, tb_msg_queue, NULL);
	//STM_RETURN_ON_FALSE(test_app_th_id != NULL, STM_ERR_NOT_FINISHED, __func__, "Thread creation failed");
	return ret;
}

// LCD Test Code
static osThreadId_t id_Th_Test_Th_lcd;


static void Test_Th_lcd(void *arguments); // Forward declaration

int Init_Th_lcd_test(void) {
	id_Th_Test_Th_lcd = osThreadNew(Test_Th_lcd, NULL, NULL);
	if (id_Th_Test_Th_lcd == NULL)
		return -1;
	return 0;
}

static void Test_Th_lcd(void *arguments) {
	static MSGQUEUE_OBJ_LCD msg_test;

	static uint8_t hora = 12;
	static uint8_t min = 27;
	static uint8_t seg = 35;

	Init_Th_lcd();

	sprintf(msg_test.data_L1, "     SBM 2024");
	while (1) {
		seg++;
		if (seg == 60) {
			seg = 0;
			min++;
			if (min == 60) {
				min = 0;
				hora++;
				if (hora == 24) {
					hora = 0;
				}
			}
		}
		sprintf(msg_test.data_L2, "      %.2u:%.2u:%.2u", hora, min, seg);
		osMessageQueuePut(get_id_MsgQueue_lcd(), &msg_test, NULL, 0U);
		osDelay(1000);
	}
}
