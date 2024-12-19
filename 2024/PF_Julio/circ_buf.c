#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "circ_buf.h"
#include "lum/lum.h"

// Define el tamaño del buffer
#define BUFFER_SIZE 20

// Memoria estática para el buffer circular
static lum_sample_t buffer_circ_mem[BUFFER_SIZE];

// Puntero estático al buffer circular
static buffer_t* buffer_circ = NULL;

// Inicializa el buffer circular
buffer_t* init_Buffer(buffer_t *buf, uint16_t _capacidad) {
    buf->i_buffer_circ = buffer_circ_mem;
    assert(buf->i_buffer_circ != NULL);
    buf->capacidad = _capacidad;
    buf->elementos = 0;
    buf->ultima_entrada = 0;
    if (buf->capacidad == _capacidad && buf->elementos == 0 && buf->ultima_entrada == 0) 
        return buf;
    return NULL;
}

// Introduce un dato en el buffer circular
void introducirDatoBuffer(buffer_t* buffer, lum_sample_t *dato) {
    memcpy(&buffer->i_buffer_circ[buffer->ultima_entrada], dato, sizeof(lum_sample_t));
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
int recogerDatoBuffer(buffer_t* buffer, lum_sample_t *medida, uint8_t n_elemento) {
    if (n_elemento >= buffer->elementos) 
        return -1;
    memcpy(medida, &buffer->i_buffer_circ[n_elemento], sizeof(lum_sample_t));
    return 0;
}

// Retorna el número de elementos en el buffer circular
uint32_t num_elementos(buffer_t* buffer) {
    return buffer->elementos;
}
