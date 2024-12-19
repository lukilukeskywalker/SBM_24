#ifndef __COM_H__
#define __COM_H__

#include <stdio.h>

#include "../stm_err.h"
#include "cmsis_os2.h"
#include "Driver_USART.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	uint8_t operation;
	uint8_t size;
	uint8_t data[64];
}com_msg_t;

stm_err_t init_com_proc(osMessageQueueId_t *rxqueueId, osMessageQueueId_t *txqueueId);


#ifdef __cplusplus
}
#endif
#endif
