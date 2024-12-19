#ifndef __PWM_H__
#define __PWM_H__
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "../stm_err.h"
#include "../stm_log.h"

stm_err_t init_pwm_proc(osMessageQueueId_t *queueId);

#endif