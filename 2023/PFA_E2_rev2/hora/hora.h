#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../system_err.h"
#include "cmsis_os2.h"


typedef struct{
	uint8_t hora;
	uint8_t min;
	uint8_t sec;
}time_t;



system_err_t init_time(time_t * time);
void set_time(uint8_t hora, uint8_t min, uint8_t sec);

