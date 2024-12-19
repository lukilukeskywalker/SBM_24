#ifndef __ACCEL_TEMP_H__
#define __ACCEL_TEMP_H__
#include "../stm_err.h"
#include "cmsis_os2.h"

typedef struct{
	float X_axis;
	float Y_axis;
	float Z_axis;
	float temp;
}accel_temp_var_t;
	
stm_err_t init_accel_temp_proc(osMessageQueueId_t *queueId);

#endif


