#ifndef __CIRC_BUF__
#define __CIRC_BUF__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>




typedef struct{
	uint16_t capacidad;
	uint16_t data_size;
	uint32_t elementos;
	uint32_t ultima_entrada;
	void* i_buffer_circ;
}buffer_t;

buffer_t* init_Buffer(uint16_t _capacidad, uint16_t data_size);
void introducirDatoBuffer(buffer_t* buffer, void *dato);
int borrarDatosBuffer(buffer_t* buffer);
int recogerDatoBuffer(buffer_t* buffer, void *dato, uint8_t n_elemento);
uint32_t num_elementos(buffer_t* buffer);

#endif
