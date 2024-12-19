#pragma once
/**
 * @author Lukas Gdanietz
 * @note Módulo encargado de la lectura de la temperatura proporcionada por el sensor LM75B conectado al bus I2C
 * Método de sincronización: Message Queue Envía un mensaje con la temperatura medida cada segundo
 * 
 * 
*/
#include "LM75/lm75.h"

typedef enum {
     temp_ok = 0,
     temp_err,
}temp_err_t;


temp_err_t init_temp_th(temp_t);

