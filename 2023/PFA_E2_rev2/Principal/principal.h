/**
* @author Lukas Gdanietz
* @note El modulo principal es el encargado de recibir las conexiones principales con los otros modulos.
*	Primero se encarga de generar las colas de coms, las cuales organizare en un struct para facil gestion. Estas colas se las pasaremos a los hilos
*
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../system_err.h"
#include "cmsis_os2.h"

// PROJECT INCLUDES
#include "../hora/hora.h"
#include "../joystick/joystick.h"
#include "../temperatura/temperatura.h"

#ifndef QUEUE_SIZE
#define QUEUE_SIZE 10
#endif

typedef enum{
	module_joystick = 0,
	module_temperatura = 1,
	module_max
}module_list;

const size_t module_size[module_max] = {
	sizeof(MSGQUEUE_OBJ_JOY), 
	sizeof(temp_message_t)
	};
	

typedef struct{
	osMessageQueueId_t module_queues[module_max];
}principal_handle_t;

