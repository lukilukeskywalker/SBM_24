#include "TSL2591.h"

 /* 
 Vamos a usar los flags del propio driver de CMSIS,
 Cuidado de no borrar nuestros flags:
 
								ARM_I2C_EVENT_.			
								TRANSFER_INCOMPLETE - (1UL << 1)
								SLAVE_TRANSMIT ------ (1UL << 2)
								SLAVE_RECEIVE ------- (1UL << 3)
								ADDRESS_NACK -------- (1UL << 4)
								GENERAL_CALL -------- (1UL << 5)
								ARBITRATION_LOST ---- (1UL << 6)
								BUS_ERROR ----------- (1UL << 7)
								BUS_CLEAR ----------- (1UL << 8)
								
 LSB son los usados en I2C. Estos se borraran cuando se haga una operacion I2C
*/

#define TSL2591_ADDR 0x29
#define TSL_ID_REG 0x12
#define TSL_CMD_BIT_REG 0x80
#define TSL_TRANSACTION 0x20

//Direciones Registro
#define ENA_REG 0x00
#define CONT_REG 0x01
#define STATUS_REG 0x13 //+TSL_TRANSACTION + TSL_CMD_CMD_BIT_REG

#define STATUS_AVALID_MASK 0x01	//Mascara para ccomprobar si ya hay una medida valida

#define POWER_ON 0x01
#define ALS_ON 0x02
//Ganancias del sensor //Registro de control!!!
#define LOW_GAIN 0x00
#define MEDIUM_GAIN 0x10
#define HIGH_GAIN 0x20
#define MAX_GAIN 0x30
//Registro de control!!!
#define SYS_RST 0x80

#define TSL_VISB_REG 0x14
#define TSL_INF_REG 0x16
								
extern ARM_DRIVER_I2C	Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
 
static MSGQUEUE_OBJ_I2C msg;
osThreadId_t tid_Th_I2C;
osMessageQueueId_t mid_MsgQueue_I2C;
static osTimerId_t Tmr_id;
 
void I2C_SignalEvent(uint32_t event){
	osThreadFlagsSet(tid_Th_I2C, event);
}
void tmr_callback(void *arguments){
	osThreadFlagsSet(tid_Th_I2C, TMR_RESP_FLAG);
}

int conf_pinout(void){
	I2Cdrv->Initialize(I2C_SignalEvent);
	I2Cdrv->PowerControl(ARM_POWER_FULL);
	I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
	I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
	return 0;
}
 
int read_TSL2591(uint16_t addr, uint8_t *buf, uint32_t len){
	uint8_t a[2];
	uint32_t flags;
  a[0] = (uint8_t)(addr >> 8);
  a[1] = (uint8_t)(addr & 0xFF);
	I2Cdrv->MasterTransmit(TSL2591_ADDR, a+1, 1, true); //3arg: xfer_pending=true, controla la condicion de stop, en true, puedes continuar haciendo operaciones en el bus.
	flags = osThreadFlagsWait(0x1FF, osFlagsWaitAny,  osWaitForever);//Might not be the best idea...
	if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	I2Cdrv->MasterReceive(TSL2591_ADDR, buf, len, false);
	flags = osThreadFlagsWait(0x1FF, osFlagsWaitAny, osWaitForever);
	//Esperamos a todos los flags. Si no es ARM_I2C_EVENT_TRANSFER_DONE, cancelamos.
	if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	return 0;
}
int capture_lum(int ciclo){
		static uint8_t lum_buf[4];
		static uint32_t measured_data;
		const uint32_t _max_lum = MAX_VALUE_VIS - MIN_VALUE_VIS;
		read_TSL2591(TSL_CMD_BIT_REG|TSL_TRANSACTION|STATUS_REG, lum_buf, 1);
		if((lum_buf[0] & STATUS_AVALID_MASK) == true){
			read_TSL2591(TSL_CMD_BIT_REG|TSL_TRANSACTION|TSL_VISB_REG, lum_buf, 4);			
			measured_data = (((lum_buf[1]<<8)|lum_buf[0]) - MIN_VALUE_VIS)*1000; // (MAX_VALUE_VIS - MIN_VALUE_VIS) * 1000;
			msg.light_cen = ((measured_data/_max_lum) / 100);
			msg.light_dec = ((measured_data/_max_lum) %100)/10;//(measured_data - msg.light_cen) % 10;
			msg.light_uni = ((measured_data/_max_lum) %1000)>>8;//measured_data - msg.light_cen - msg.light_dec;
			msg.ciclo=ciclo;
			osMessageQueuePut(mid_MsgQueue_I2C, &msg, 0U, 0U);	
			return 0;//osThreadFlagsClear(INIT_LIGHT_MEASURE);
		}
		return -1;
}


int write_TSL2591(uint8_t *data, uint8_t len){
	return I2Cdrv->MasterTransmit(TSL2591_ADDR, data, len, false);
}

void Th_I2C (void *argument);

int Init_I2C_message_queue(void){
	mid_MsgQueue_I2C = osMessageQueueNew(MSGQUEUE_OBJECTS_I2C, sizeof(MSGQUEUE_OBJ_I2C), NULL);
	if(mid_MsgQueue_I2C ==NULL)return -1;
	return 0;
}
int Init_I2C_Tmr(void){
	Tmr_id = osTimerNew(tmr_callback, osTimerOnce, (void *)0, NULL);
	if(Tmr_id == NULL) return -1;
	return 0;
}

int Init_Th_I2C (void) {
	conf_pinout();
  tid_Th_I2C = osThreadNew(Th_I2C, NULL, NULL);
  if (tid_Th_I2C == NULL) {
    return(-1);
  }
  return(Init_I2C_message_queue()|Init_I2C_Tmr());
}
 
void Th_I2C (void *argument) {
	int i=0;
	int t=0;
	uint32_t flags;
	//Escribir en A0 03 
	uint8_t data[]={TSL_CMD_BIT_REG|TSL_TRANSACTION|ENA_REG, ALS_ON|POWER_ON, MEDIUM_GAIN};	//ALS_ENABLE & PowerON, MEDIOUM GAIN
	write_TSL2591(data, 3);
	//free(data);	//Crashea el hilo de ejeccucion, una vez espera un
	//read_TSL2591(TSL_CMD_BIT_REG|TSL_TRANSACTION|TSL_VISB_REG, lum_buf, 4);
	while (1) {
		osThreadFlagsWait(INIT_LIGHT_MEASURE|INIT_LIGHT_CYCLE, osFlagsWaitAny | osFlagsNoClear, osWaitForever);	//Se queda esperando hasta que recibe un flag del OS
		flags = osThreadFlagsGet();
 		if((flags&INIT_LIGHT_MEASURE)>>28){
			if(capture_lum(false)==0)osThreadFlagsClear(INIT_LIGHT_MEASURE);
		}else if(((flags&INIT_LIGHT_CYCLE)>>29)){
			i=((flags&N_CYCLE_MASK)>>N_CYCLE_SHIFT)+1;		//No la mejor implementacion
			t=((flags&N_TIME_MASK)>>N_TIME_SHIFT);
			do{
				if(!capture_lum(true))i--;		//99 bottle beer, -1 makes 98 bottle of beer...
				osTimerStart(Tmr_id, t*1000);
				osThreadFlagsWait(TMR_RESP_FLAG, osFlagsWaitAny, 99001U);
			}while(i!=0);
			osThreadFlagsClear(INIT_LIGHT_CYCLE|N_CYCLE_MASK|N_TIME_MASK);
		}
  }
}

