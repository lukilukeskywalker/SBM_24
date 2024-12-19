#pragma once

/**
 * @brief Hardware Agnostic library for lm75 sensor
 * @author Lukas Gdanietz
*/

#include <stdio.h>
#include <stdint.h>



#define BASE_I2C_ADDR    72

typedef enum{
     I2C_ADDR_0 = 0,
     I2C_ADDR_1 = 1,
     I2C_ADDR_2 = 2,
     I2C_ADDR_3 = 3,
     I2C_ADDR_4 = 4,
     I2C_ADDR_5 = 5,
     I2C_ADDR_6 = 6,
     I2C_ADDR_7 = 7,
     I2C_ADDR_MAX
}lm75_addr_enum_t;

typedef enum{
     lm75_temp_reg = 0,
     lm75_conf_reg = 1,
     lm75_thys_reg = 2,
     lm75_tos_reg = 3,
     lm75_reg_max
}lm75_reg_enum_t;

typedef union{
     uint8_t configuration;
     struct{
          uint8_t PAD: 3;
          uint8_t fault_queue: 2;
          uint8_t os_pol: 1;
          uint8_t com_int_mode: 1;
          uint8_t shutdown: 1;
     };
}lm75_config_reg_t;

/**
 * @brief Structure with hardware abstraction functions for the operation of the LM75 library
*/
typedef struct{
     int32_t (* i2c_read_op)(const void *handle, const void *out_data, size_t out_size, void *in_data, size_t in_size);
     int32_t (* i2c_write_op)(const void *handle, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size);
     void *handle;                                   /*!< Handle that other hardware abstraction functions may need*/    
}lm75_t;
/**
 * @brief Sets the configuration of lm75
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
void lm75_set_conf(lm75_t lm75_handle, lm75_config_reg_t lm75_config);
/**
 * @brief Gets die temperature of lm75
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
int16_t lm75_get_temp(lm75_t lm75_handle);
/** 
 * @brief Sets the temperature hysteresis for opendrain 
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
void lm75_set_thys(lm75_t lm75_handle, float thys);
/** 
 * @brief Gets the temperature hysteresis for opendrain 
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
float lm75_get_thys(lm75_t lm75_handle);
/** 
 * @brief Sets the max temperature trigger for opendrain 
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
void lm75_set_tos(lm75_t lm75_handle, float tos);
/** 
 * @brief Gets the max temperature trigger for opendrain 
 * @param lm75_handle Handle that contains the list of params required to handle the sensor in a hardware agnostic code arquitecture
*/
float lm75_get_tos(lm75_t lm75_handle);


