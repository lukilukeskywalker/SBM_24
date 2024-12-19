#include "accel&temp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Driver_I2C.h"
#include "libs/adxl34x/adxl34x.h"

#define TAG		"accel_temp"
#define TEST		1
#define TMR_ELAPSED_FLAG 0x200
#define INT_FLAG_0 	 0x400
#define INT_FLAG_1	 0x800

const float accel_resolution = 2.0; 

const char* error_codes[] = {
	"TRANSFER_DONE---------0x01",
	"TRANSFER_INCOMPLETE - 0x02",  // (1UL << 1) = 0x02
	"SLAVE_TRANSMIT ------ 0x04",  // (1UL << 2) = 0x04
	"SLAVE_RECEIVE ------- 0x08",  // (1UL << 3) = 0x08
	"ADDRESS_NACK -------- 0x10",  // (1UL << 4) = 0x10
	"GENERAL_CALL -------- 0x20",  // (1UL << 5) = 0x20
	"ARBITRATION_LOST ---- 0x40",  // (1UL << 6) = 0x40
	"BUS_ERROR ----------- 0x80",  // (1UL << 7) = 0x80
	"BUS_CLEAR ----------- 0x100"  // (1UL << 8) = 0x100
};

const uint8_t adxl34x_addr = ADXL34x_I2C_ADDR1;		// Hay 2 ADXL_34x_I2C_ADDR0 y ADDR1. El mio es ADDR1, se puede configurar con un pin, pero no se cual es (en mi breakout-brd)

typedef struct{
	adxl34x_t *adxl34x_hndl;
}accel_temp_t;

static accel_temp_t accel_temp_hndl;
static osMessageQueueId_t *private_queueId = NULL;

extern ARM_DRIVER_I2C Driver_I2C1;
static osThreadId_t *task_id = NULL;
void i2c_cb(uint32_t event){
	osThreadFlagsSet(task_id, event);
}
int32_t i2c_read(void *handle, uint8_t reg, void *in_data, size_t in_size){
	uint32_t thread_flags = 0;
	uint8_t slave_addr = *(uint8_t *)handle;
	Driver_I2C1.MasterTransmit(slave_addr, &reg, 1, true);
	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if(thread_flags != ARM_I2C_EVENT_TRANSFER_DONE)
		return thread_flags;
	Driver_I2C1.MasterReceive(slave_addr, in_data, in_size, false);
	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if(thread_flags == ARM_I2C_EVENT_TRANSFER_DONE)
		return 0;
	return thread_flags;
}
/*
int32_t i2c_write (void *handle, uint8_t reg, const void *out_data, size_t out_size){
	uint32_t thread_flags = 0;
	uint8_t slave_addr = *(uint8_t *)handle;
	Driver_I2C1.MasterTransmit(slave_addr, &reg, 1, true);
	if(thread_flags != ARM_I2C_EVENT_TRANSFER_DONE)
		return thread_flags;
	Driver_I2C1.MasterTransmit(slave_addr, out_data, out_size, false);
	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if(thread_flags == ARM_I2C_EVENT_TRANSFER_DONE)
		return 0;
	return thread_flags;
}*/
int32_t i2c_write(void *handle, uint8_t reg, const void *out_data, size_t out_size) {
	uint32_t thread_flags = 0;
	uint8_t slave_addr = *(uint8_t *)handle;

	uint8_t buffer[out_size + 1];
	buffer[0] = reg; // First byte is the register address
	memcpy(&buffer[1], out_data, out_size); // Copy the data to the rest of the buffer

	Driver_I2C1.MasterTransmit(slave_addr, buffer, sizeof(buffer), false);

	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if (thread_flags == ARM_I2C_EVENT_TRANSFER_DONE)
		return 0;

	return thread_flags;
}
int32_t waitInterrupt(uint32_t IntSrcToWait, bool WaitAll){
	return 0;	//STUB
}
void tmr_cb(void *arg){
	osThreadId_t *thread_id = (osThreadId_t *)arg;
	osThreadFlagsSet(*thread_id, TMR_ELAPSED_FLAG);
	return;
}
static void accel_temp_th(void *args){
	stm_err_t ret = 0;
	static osStatus_t osstatus;
	uint32_t thread_flags = 0;
	task_id = osThreadGetId();		// Para que los flags vayan a nuestro hilo
	// Iniciamos un timer por segundo:
	osTimerId_t tmr_id = osTimerNew(tmr_cb, osTimerPeriodic, &task_id, NULL);
	osTimerStart(tmr_id, 1000);
	const adxl34x_t adxl34x_hndl = {
		.read = (void *)i2c_read,
		.write = (void *)i2c_write,
		.handle = (void *)&adxl34x_addr,
		.waitInterrupt = waitInterrupt,
		.IntSrc_0 = INT_FLAG_0,
		.IntSrc_1 = INT_FLAG_1
	};
	static accel_temp_var_t accel_temp;
	adxl_setRange(&adxl34x_hndl, _2G);
	adxl_SamplingRate(&adxl34x_hndl, _100Hz);
	adxl_FIFO_Mode(&adxl34x_hndl, BYPASS);
	adxl_ena_Measurement(&adxl34x_hndl, 1);
	// Perform calibration
	//adxl_calibrate(&adxl34x_hndl, 32);		// No tengo puestos los interrupts, asi que esto por ahora no puede funcionar...
	while(1){
		thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 1100);
		//STM_GOTO_ON_FALSE(thread_flags & TMR_ELAPSED_FLAG, STM_ERR_NOT_ALLOWED, thread_fail_cleanup, __func__, "Thread Starvation");
		//adxl_axis_t axis = adxl_get_axis(&adxl34x_hndl);
		accel_temp.X_axis = (accel_resolution * adxl_getX(&adxl34x_hndl))/512;
		accel_temp.Y_axis = (accel_resolution * adxl_getY(&adxl34x_hndl))/512;
		accel_temp.Z_axis = (accel_resolution * adxl_getZ(&adxl34x_hndl))/512;
		osstatus = osMessageQueuePut(private_queueId, &accel_temp, NULL, 250);
		//STM_GOTO_ON_FALSE(osstatus != 0, STM_FAIL, thread_fail_cleanup,__func__, "osstatus returned: &d", osstatus);
		//STM_LOGI(__func__, "Se recibe la siguiente acceleracion: x: %.2f, y: %.2f z: %.2f", accel_temp.X_axis, accel_temp.Y_axis, accel_temp.Z_axis);

	}
	thread_fail_cleanup:
	osThreadExit();		// Pues algo ha fallado y cerramos el hilo
}
stm_err_t init_accel_temp_proc(osMessageQueueId_t *queueId){
	stm_err_t ret = STM_OK;
	uint8_t adxl_device_id = 0;
	//STM_RETURN_ON_FALSE(accel_temp_hndl != NULL, STM_ERR_INVALID_STATE, __func__, "Already initialized");
	STM_RETURN_ON_FALSE(queueId != NULL, STM_ERR_INVALID_ARG, __func__, "queueId can not be NULL");
	// Inicializamos el periferico i2c
	Driver_I2C1.Initialize(i2c_cb);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	Driver_I2C1.Control(ARM_I2C_BUS_CLEAR, 0);
	
	// Inicializamos hndl de adxl34x
	const adxl34x_t adxl34x_hndl = {
		.read = (void *)i2c_read,
		.write = (void *)i2c_write,
		.handle = (void *)&adxl34x_addr,
		.waitInterrupt = waitInterrupt,
		.IntSrc_0 = INT_FLAG_0,
		.IntSrc_1 = INT_FLAG_1
	};
	// Probamos a comunicarnos con el LM35 y con el ADXL34x.
#if TEST
	task_id = osThreadGetId();
#endif
	adxl_device_id = adxl_getDeviceID(&adxl34x_hndl);				//  Segun datasheet tiene que devolver b"11100110"
	if(adxl_device_id == 0xE6)
		STM_LOGI(TAG, "Adxl responde con ID esperado");
	else
		STM_LOGE(TAG, "Adxl responde con ID inesperado. ID obtenido: 0x02%", adxl_device_id);
	accel_temp_hndl.adxl34x_hndl = &adxl34x_hndl;
	private_queueId = queueId;
	osThreadNew(accel_temp_th, NULL, NULL);
	return ret;
}


