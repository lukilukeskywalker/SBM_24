/**
 * @author Lukas Gdanietz
 * @note Se ocupa de la com con el pc a traves de datagramas.
*/
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"
#include "Driver_USART.h"
#include "../system_err.h"

#define START_OF_HEADER            0x01
#define END_OF_TRANSMISSION        0xFE



typedef __attribute__((__packed__)) struct{
     uint8_t soh;        //In packed mode: 1b
     uint8_t cmd;                        //2b
     uint8_t len;                       // 3b
     uint8_t *data;                     // 7b     
     uint8_t eot;                       // 8b = 2 mem loc
}com_segment_t;

typedef struct{
     uint8_t cmd;
     uint8_t len;
     uint8_t *data;
}com_message_t;


typedef struct{
     osThreadId_t   com_task_id;
     osMessageQueueId_t com_ingoing_messagequeue;
     osMessageQueueId_t com_outgoing_messagequeue;
     osSemaphoreId_t usartSemaphore;
}com_pc_handle_t;
// Lo cierto es que hay muchas maneras de hacr las cosas mal. Habra que pensar como mantener el sistema asincrono y unir las señales 
// de los message queue 
system_err_t init_com_pc(osMessageQueueId_t com_ingoing_messagequeue, osMessageQueueId_t com_outgoing_messagequeue);









