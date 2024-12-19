#include <stdio.h>
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "accel_temp_tb.h"
#include "../accel&temp.h"



void accel_temp_tb_th(void *args){
	static accel_temp_var_t accel_temp;
	osMessageQueueId_t *test_queueId = osMessageQueueNew(10, sizeof(accel_temp_var_t), NULL);
	init_accel_temp_proc(test_queueId);
	while(1){
		osMessageQueueGet(test_queueId, &accel_temp, NULL,  osWaitForever);
		STM_LOGI(__func__, "Se recibe la siguiente acceleracion: x: %d, y: %d z: %d", accel_temp.X_axis, accel_temp.Y_axis, accel_temp.Z_axis);
		osDelay(100);
		
	}
}
