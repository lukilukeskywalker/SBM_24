/**
 * @author Lukas Gdanietz
 * @note M�dulo encargado de la lectura de la temperatura proporcionada por el sensor LM75B conectado al bus I2C
 * M�todo de sincronizaci�n: Message Queue Env�a un mensaje con la temperatura medida cada segundo
 * La forma mas optima es que opere dentro del stackup del programa principal
 * 
*/

#include "temperatura.h"
osThreadId_t temperatura_task_id;
extern ARM_DRIVER_I2C	Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
/*
typedef struct{
	ARM_DRIVER_I2C I2Cdrv;
	uint8_t device_addr;
}i2c_handle_t;
*/
void i2c_SignalEvent(uint32_t event){
	osThreadFlagsSet(temperatura_task_id, event);
}
static int32_t i2c_init(void){
  I2Cdrv->Initialize(i2c_SignalEvent);
	I2Cdrv->PowerControl(ARM_POWER_FULL);
	I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);      //Sets the speed of the bus
	I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);                           //Clears the bus by sending 9 pulses
	return 0;
}
static int32_t i2c_read_op(const void *handle, const void *out_data, size_t out_size, void *in_data, size_t in_size){
	uint32_t os_flags = 0;
	//i2c_handle_t *i2c_handle = (i2c_handle_t *)handle;
	//ARM_DRIVER_I2C I2Cdrv = i2c_handle->I2Cdrv;
	//int32_t 	ARM_I2C_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending)
	//I2Cdrv.MasterTransmit(i2c_handle->device_addr, out_data, out_size, true);
	//Aqui tenemos que esperar a que la transmision se haya completado! @todo
	//I2Cdrv.MasterReceive(i2c_handle->device_addr, in_data, in_size, false);
	I2Cdrv->MasterTransmit(LM75_BASE_I2C_ADDR, out_data, out_size, true);
	os_flags = osThreadFlagsWait(0x1FF, osFlagsWaitAny, 100);
	if((os_flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (os_flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	I2Cdrv->MasterReceive(LM75_BASE_I2C_ADDR, in_data, in_size, false);
	if((os_flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (os_flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	return 0;
	
}
static int32_t i2c_write_op(const void *handle, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size){
	uint32_t os_flags = 0;
	//i2c_handle_t *i2c_handle = (i2c_handle_t *)handle;
	//ARM_DRIVER_I2C I2Cdrv = i2c_handle->I2Cdrv;
	//I2Cdrv.MasterTransmit(i2c_handle->device_addr, out_reg, out_reg_size, true);
	I2Cdrv->MasterTransmit(LM75_BASE_I2C_ADDR, out_reg, out_reg_size, true);
	if((os_flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (os_flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	I2Cdrv->MasterTransmit(LM75_BASE_I2C_ADDR, out_data, out_size, false);
	if((os_flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (os_flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	return 0;	
}
/**
* @brief Callback for making a I2C transfer and capture the temperature
*/
static void temp_capture_cb(void *arg){
	temp_handle_t *temp_handle = (temp_handle_t *)arg;
	float temperature = lm75_get_temp(temp_handle->lm75);
	osMessageQueuePut(temp_handle->temp_messagequeue, &temperature, 0U, 0U);	//Prio 0, Return Delay 0, we dont want to delay other stuff
	return;	
}

void temp_task(void *args){
	while(1)
		osThreadYield();		//Si no fuera porque el per I2C no tiene buffer... o porque la com tarda tanto, que nopodemos dejar que el hilo central se detenga...
}
temp_err_t temp_init(osMessageQueueId_t temp_messagequeue, temp_handle_t **handle){
	if(temp_messagequeue == NULL)
		return temp_err_invalid_params;					//err_invalid_params
	temp_handle_t *temp_handle = (temp_handle_t *)calloc(1, sizeof(temp_handle_t));
	if(temp_handle == NULL)
		return temp_err_mem_alloc;
	temp_handle->temp_messagequeue = temp_messagequeue;
	temp_handle->lm75 = calloc(1, sizeof(lm75_t));
	//(temp_handle->lm75)->handle = calloc(1, sizeof(i2c_handle_t));	//Esto debido a que i2c_driver es const, me lo puedo ahorrar
	//((i2c_handle_t *)((temp_handle->lm75)->handle))->I2Cdrv = i2c_driver;		//El i2cdriver es const, por tanto no puedo meterlo en una variable con constante, por tanto solo queda traerlo con extern
	//((i2c_handle_t *)((temp_handle->lm75)->handle))->device_addr = LM75_BASE_I2C_ADDR;
	(temp_handle->lm75)->i2c_read_op = &i2c_read_op;
	(temp_handle->lm75)->i2c_write_op = &i2c_write_op;
	
	temperatura_task_id = osThreadNew(temp_task, NULL, NULL);
  if (temperatura_task_id == NULL)
    goto cleanup;
	temp_handle->temp_timer = osTimerNew(temp_capture_cb, osTimerPeriodic, temp_handle, NULL);
	//TEMP_RETURN_ERR_ON_FALSE(temp_handle->temp_timer != NULL, temp_err_create_timer);
	if(temp_handle->temp_timer == NULL)
		goto cleanup;
	temp_handle->temp_task_id = temperatura_task_id;
	if(handle != NULL)	//Solo devolvemos si hay espacio donde guardarlo
		*handle = temp_handle;	//If everything is fine, lets set the handle
	return temp_ok;
	cleanup:
		if(temp_handle->temp_timer != NULL){
			osTimerStop(temp_handle->temp_timer);
			osTimerDelete(temp_handle->temp_timer);
		}
		if(temperatura_task_id != NULL) 
			osThreadTerminate(temperatura_task_id);			//Kill this process
		if(temp_handle != NULL)
			free(temp_handle);
		
		return temp_err;				//Lo apropiado es establecer para cada condicion que falla, un codigo de retorno, por ejemplo err_timer_init o err_mem_alloc, etc. Los errores se deben tratar y corregir
}

//Falta hacer un stop y delete



