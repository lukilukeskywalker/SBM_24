#ifndef __CIRC_BUF__
#define __CIRC_BUF__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lum/lum.h"

typedef struct{
	program_clock_t sample_time;
	sample_luminance_t luminance;
	uint16_t Lref;
	uint16_t PWM;
}lum_sample_t;

typedef struct{
	uint32_t capacidad;
	uint32_t elementos;
	uint32_t ultima_entrada;
	lum_sample_t* i_buffer_circ;
}buffer_t;

buffer_t* init_Buffer(buffer_t *buf, uint16_t _capacidad);
void introducirDatoBuffer(buffer_t* buffer, lum_sample_t *dato);
int borrarDatosBuffer(buffer_t* buffer);
int recogerDatoBuffer(buffer_t* buffer, lum_sample_t *medida, uint8_t n_elemento);
uint32_t num_elementos(buffer_t* buffer);

#endif
