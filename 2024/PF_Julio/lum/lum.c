#include "lum.h"
#include "bh1750.h"

extern ARM_DRIVER_I2C	Driver_I2C1;

#define TMR_ELAPSED_FLAG 0x200

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

uint32_t set_flag_index(uint32_t flags, uint32_t max){
	for(int i = 0; i < max; i++){
		if(flags & (1ULL << i))
			return i;
	}
	return max;
}



extern program_clock_t program_clock;

static osThreadId_t *lum_th_id = NULL;


typedef struct{
	ARM_DRIVER_I2C *I2Cdrv;
	uint8_t slave_addr;
	osMessageQueueId_t *queueId;
}lum_handle_t;

int32_t i2c_read(void *handle, uint8_t *in_data, size_t in_size){
	uint32_t thread_flags = 0;
	lum_handle_t *lum_hndl = (lum_handle_t *) handle;
	lum_hndl->I2Cdrv->MasterReceive(lum_hndl->slave_addr, in_data, in_size, false);
	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if(thread_flags == ARM_I2C_EVENT_TRANSFER_DONE)
		return 0;
	return thread_flags;
}
int32_t i2c_write(void *handle, const uint8_t *out_data, size_t out_size){
	uint32_t thread_flags = 0;
	lum_handle_t *lum_hndl = (lum_handle_t *) handle;
	lum_hndl->I2Cdrv->MasterTransmit(lum_hndl->slave_addr, out_data, out_size, false);
	thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 100);
	if(thread_flags == ARM_I2C_EVENT_TRANSFER_DONE)
		return 0;
	return thread_flags;
}

void tmr_cb(void *arg){
	osThreadId_t *thread_id = (osThreadId_t *)arg;
	osThreadFlagsSet(*thread_id, TMR_ELAPSED_FLAG);
	return;
}

void lum_thread(void *args){
	stm_err_t ret = STM_OK;
	uint16_t luminance = 0;
	sample_luminance_t sample_lum = {0};
	uint32_t thread_flags = 0;
	bh1750_t *bh = (bh1750_t *)args;
	lum_handle_t *lum_hndl = (lum_handle_t *)bh->handle;
	init_bh1750(bh);
	set_mode_bh1750(bh, CONTINUOUS_HIGH_RES_MODE1);
	set_mea_tm_bh1750(bh, 69);
	osThreadId_t thread_id = osThreadGetId();
	osTimerId_t tmr_id = osTimerNew(tmr_cb, osTimerPeriodic, &thread_id, NULL);
	osTimerStart(tmr_id, 1000);
	while(1){
		thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 1100);
		STM_GOTO_ON_FALSE(thread_flags & TMR_ELAPSED_FLAG, STM_ERR_NOT_ALLOWED, thread_fail_cleanup, __func__, "Thread Starvation");
		STM_LOGD(__func__, "Tomando medida de luminosidad");
		thread_flags = read_luminance_bh1750(bh, &sample_lum.luminance); //&luminance);
		////STM_GOTO_ON_FALSE(thread_flags, STM_ERR_NOT_ALLOWED, thread_fail_cleanup, __func__, "Thread Starvation"); Perdona pero no me quiero disparar en los huevos. Aqui no
		if(thread_flags)
			STM_LOGW(__func__, "Thread_flags a devuelto un codigo distinto al esperado : %s", error_codes[set_flag_index(thread_flags, 8)]);
		STM_LOGD(__func__, "Se ha medido: %d", sample_lum.luminance);
		
		osMessageQueuePut(lum_hndl->queueId, &sample_lum, NULL, 250);
		
		//Driver_I2C1.MasterReceive(BH1750_ADDR1, in_data, 2, false);
		//thread_flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, 1100);
		//STM_LOGI(__func__, "Valor de thread_flags: 0x%x : %s", thread_flags, error_codes[set_flag_index(thread_flags, 8)]);
		//STM_GOTO_ON_FALSE(0 == (thread_flags & osFlagsError), thread_flags, thread_fail_cleanup, __func__, "Thread flags returned an error: %s", error_codes[set_flag_index(thread_flags, 8)]); // An error ocurred
	}
	return;
	thread_fail_cleanup:
	osThreadExit();
}

void i2c_cb(uint32_t event){
	osThreadFlagsSet(lum_th_id, event);
}

stm_err_t init_lum_proc(osMessageQueueId_t *queueId){
	stm_err_t ret = STM_OK;
	static lum_handle_t *lum_hndl = NULL;
	static bh1750_t *bh = NULL;
	STM_RETURN_ON_FALSE(lum_hndl == NULL || bh == NULL, STM_ERR_INVALID_STATE, __func__, "Lum already initialized");
	
	// Inicilaizamos handles sistema
	lum_hndl = (lum_handle_t *)calloc(1, sizeof(lum_handle_t));
	bh = (bh1750_t *)calloc(1, sizeof(bh1750_t));
	
	STM_RETURN_ON_FALSE(lum_hndl != NULL || bh != NULL, STM_ERR_NO_MEM, __func__, "No se ha podido alocar memoria para handles");
		
	//Rellenamos handle del "programa"
	lum_hndl->I2Cdrv = &Driver_I2C1;
	lum_hndl->queueId = queueId;
	lum_hndl->slave_addr = BH1750_ADDR1;
	bh->handle = lum_hndl;
	bh->read = i2c_read;
	bh->write = i2c_write;
	//Inicializamos periferico I2C
	lum_hndl->I2Cdrv->Initialize(i2c_cb);
	lum_hndl->I2Cdrv->PowerControl(ARM_POWER_FULL);
	lum_hndl->I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	lum_hndl->I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
	lum_handle_t *lum_hndl_calloc = (lum_handle_t *)calloc(1, sizeof(lum_handle_t));
	// Creamos thread del sistema
	lum_th_id = osThreadNew(lum_thread, bh, NULL);
	if(lum_th_id == NULL)
		return STM_FAIL;
	return ret;
}

