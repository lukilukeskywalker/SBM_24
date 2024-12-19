#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "circ_buf.h"
#include "stm_log.h"


// Inicializa el buffer circular
buffer_t* init_Buffer(uint16_t _capacidad, uint16_t data_size){
	buffer_t *buf = calloc(1, sizeof(buffer_t));
	if(buf == NULL)
		return NULL;
	buf->i_buffer_circ = calloc(_capacidad, data_size);
	if(buf->i_buffer_circ == NULL)
		goto clean_buf;
	buf->capacidad = _capacidad;
	buf->data_size = data_size;
        return buf;
	clean_buf:
	free(buf);
	return NULL;
}

// Introduce un dato en el buffer circular
void introducirDatoBuffer(buffer_t* buffer, void *dato){
	STM_LOGI(__func__, "Se introduce dato en %d", buffer->ultima_entrada);
	STM_LOGI(__func__, "Se introduce lo siguiente: %s", dato);
	memcpy(&buffer->i_buffer_circ[buffer->ultima_entrada*buffer->data_size], dato, buffer->data_size);		//Shifteamos el tamano del dato
	STM_LOGI(__func__, "En el buf tenemos: %s", (char *)&buffer->i_buffer_circ[buffer->ultima_entrada*buffer->data_size]);
	buffer->ultima_entrada = (buffer->ultima_entrada + 1) % buffer->capacidad;
	if (buffer->elementos < buffer->capacidad)
		buffer->elementos++;
	return;
}

// Borra los datos del buffer circular
int borrarDatosBuffer(buffer_t* buffer) {
	buffer->elementos = 0;
	buffer->ultima_entrada = 0;
	return 0;
}

// Recoge un dato del buffer circular
int recogerDatoBuffer(buffer_t* buffer, void *dato, uint8_t n_elemento){
	
	if (n_elemento >= buffer->elementos) 
		return -1;
	STM_LOGI(__func__, "Se lee dato en %d", n_elemento);
	STM_LOGI(__func__, &buffer->i_buffer_circ[n_elemento]);
	memcpy(dato, &buffer->i_buffer_circ[n_elemento*buffer->data_size], buffer->data_size);
	return 0;
}

// Deuelve el número de elementos en el buffer circular
uint32_t num_elementos(buffer_t* buffer) {
    return buffer->elementos;
}
