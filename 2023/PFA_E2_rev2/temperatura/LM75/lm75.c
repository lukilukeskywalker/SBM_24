#include "lm75.h"

static inline float lm75_type_to_float(uint16_t value){
     return ((float)value)/256;
}

float lm75_get_temp(lm75_t *lm75_handle){
     static uint16_t temperature = 0;
     lm75_handle->i2c_read_op(lm75_handle->handle, lm75_temp_reg, 1, &temperature, sizeof(uint16_t));
     return lm75_type_to_float(temperature);
}
