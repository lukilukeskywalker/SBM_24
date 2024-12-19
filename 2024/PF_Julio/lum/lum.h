#ifndef __LUM_H__
#define __LUM_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../stm_err.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h"

#ifndef __PROGRAM_CLOCK_H__
typedef struct{
	uint8_t hora;
	uint8_t minuto;
	uint8_t segundo;
}program_clock_t;
#endif

typedef struct{
	//program_clock_t sample_timestamp;
	uint16_t luminance;
}sample_luminance_t;

stm_err_t init_lum_proc(osMessageQueueId_t *queueId);

#endif
