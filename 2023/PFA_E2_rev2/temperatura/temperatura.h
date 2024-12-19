/**
 * @author Lukas Gdanietz
 * @note Módulo encargado de la lectura de la temperatura proporcionada por el sensor LM75B conectado al bus I2C
 * Método de sincronización: Message Queue Envía un mensaje con la temperatura medida cada segundo
 * 
 * 
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Driver_I2C.h"
#include "LM75/lm75.h"
#include "cmsis_os2.h"


typedef enum {
     temp_ok = 0,
     temp_err,
	   temp_err_invalid_params, 
	   temp_err_mem_alloc, 
		 temp_err_create_timer,
}temp_err_t;

typedef struct{
     lm75_t *lm75;
     osMessageQueueId_t temp_messagequeue;
		 osThreadId_t temp_task_id;
		 osTimerId_t temp_timer;
}temp_handle_t;

typedef float temp_message_t;

temp_err_t temp_init(osMessageQueueId_t temp_messagequeue, temp_handle_t **handle);
