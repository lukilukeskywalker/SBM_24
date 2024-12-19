#ifndef __POT_H__
#define __POT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../stm_err.h"
#include "cmsis_os2.h"

typedef struct{
	uint16_t pot[2];
}pot_msg_t;

stm_err_t init_pot_proc(osMessageQueueId_t *queueId);

#endif