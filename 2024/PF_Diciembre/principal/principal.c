#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include <math.h>
#include "../stm_log.h"
#include "../stm_err.h"
#include "principal.h"
#include "../LCD/lcd.h"
#include "../Clock/clock.h"
#include "../Joystick/joystick.h"
#include "../Acelerometro/accel&temp.h"
#include "../com/com.h"
#include "../circ_buf.h"
#include "../leds_nucleo/leds_nucleo.h"
#define DEBUG 1
#define N_SAMPLES_BUF 10
#define SAMPLE_SIZE   42

extern program_clock_t current_time; 

typedef enum{
	sys_state_reposo,
	sys_state_activo,
	sys_state_programacion,
	sys_state_max
}sys_state_t;
typedef enum{
	disable_edit,
	edit_hour,
	edit_min,
	edit_sec,
	edit_AX,
	edit_AY,
	edit_AZ,
}prog_state_t;
typedef struct{
	osMessageQueueId_t joy_msg_queue;
	joy_event_t joy_event;
	osMessageQueueId_t accel_temp_queue;
	accel_temp_var_t accel_temp_var;
	float xlimit, ylimit, zlimit;
	osMessageQueueId_t rxmsg_queue; 
	osMessageQueueId_t txmsg_queue;
	com_msg_t rx_com_msg;
	osThreadId_t led_thread_id;
	buffer_t *circ_buf;
}app_params_t;

app_params_t app_params;


void app_stmachine(void);

void app_lcd_display_reposo(void){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static program_clock_t current_time_internal;
	if(0 == memcmp(&current_time, &current_time_internal, sizeof(program_clock_t)))
		return;		// No changes
	sprintf(lcd_msg.data_L1, "     SBM 2024");
	sprintf(lcd_msg.data_L2, "      %.2u:%.2u:%.2u", current_time.hora, current_time.minuto, current_time.segundo);
	osMessageQueuePut(get_id_MsgQueue_lcd(), &lcd_msg, NULL, 0U);
	return;
}
void app_lcd_display_activo(void){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static double temp = 22.3;//app_params.accel_temp_var.temp;
	
	sprintf(lcd_msg.data_L1, "     ACTIVO-- T:%.1f°", temp);
	sprintf(lcd_msg.data_L2, "     X:%.1f Y:%.1f Z:%.1f", app_params.accel_temp_var.X_axis, app_params.accel_temp_var.Y_axis, app_params.accel_temp_var.Z_axis);
	osMessageQueuePut(get_id_MsgQueue_lcd(), &lcd_msg, NULL, 0U);
	return;
}
void app_store_smpl(void){
	static char msg[42];		//Incluyendo posibles valores negativos (+3*-)
	static program_clock_t current_time_internal;
	if(0 == memcmp(&current_time, &current_time_internal, sizeof(program_clock_t)))
		return;		// No changes
	memcpy(&current_time_internal, &current_time, sizeof(program_clock_t));
	sprintf(msg, "%.2u:%.2u:%.2u--Tm:%2.1f°-Ax:%1.1f-Ay:%1.1f-Az:%1.1f\n",
										current_time.hora,
										current_time.minuto,
										current_time.segundo,
										app_params.accel_temp_var.temp,
										app_params.accel_temp_var.X_axis,
										app_params.accel_temp_var.Y_axis,
										app_params.accel_temp_var.Z_axis
										);
	introducirDatoBuffer(app_params.circ_buf, msg);									
}
void app_lcd_display_programacion(){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static program_clock_t current_time_internal;
	static uint16_t step = 0;
	static prog_state_t prog_state = disable_edit;
	static uint16_t val_max = 24, *changed_val = &current_time_internal.hora;
	static uint16_t xlimit_10 = 100, ylimit_10 = 100, zlimit_10 = 100;		// SOn los float multiplicados por 10
	xlimit_10 = (app_params.xlimit * 100);
	ylimit_10 = (app_params.ylimit * 100);
	zlimit_10 = (app_params.zlimit * 100);
	sprintf(lcd_msg.data_L1, " ---P&D---");
	switch(prog_state){
		case disable_edit:
			memcpy(&current_time_internal, &current_time, sizeof(program_clock_t));
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_hour;
			/*
			if(app_params.joy_event == JOY_RIGHT_SPULSE || app_params.joy_event == JOY_CENTER_SPULSE){
				prog_state = edit_hour;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_sec;
			}*/
			sprintf(lcd_msg.data_L2, " H %.2u:%.2u:%.2u", current_time.hora, current_time.minuto, current_time.segundo);
			break;
		case edit_hour:
			sprintf(lcd_msg.data_L2, " H [%.2u]:%.2u:%.2u", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo);
			val_max = 24;
			step = 1;
			changed_val = &current_time_internal.hora;
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_min;
			/*		
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_min;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_sec;
			}
			*/
			break;
		case edit_min:
			sprintf(lcd_msg.data_L2, " H %.2u:[%.2u]:%.2u", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo);
			val_max = 60;
			step = 1;
			changed_val = &current_time_internal.minuto;
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_sec;
			/*
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_sec;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_hour;
			}
			*/
			break;
		case edit_sec:
			val_max = 60;
			step = 1;
			changed_val = &current_time_internal.segundo;
			sprintf(lcd_msg.data_L2, " H %.2u:%.2u:[%.2u]", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo);
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_AX;
			/*
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_hour;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_min;
			}
			*/
			break;
		case edit_AX:
			val_max = 200;
			step = 10;
			changed_val = &xlimit_10;
			app_params.xlimit = ((float)xlimit_10)/100;	// Aqui reduzco por 10
			sprintf(lcd_msg.data_L2," AX_r: %.2f", app_params.xlimit);
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_AY;
			break;
		case edit_AY:
			val_max = 20;
			step = 1;
			changed_val = &ylimit_10;
			app_params.ylimit = ((float)ylimit_10)/100;	// Aqui reduzco por 10
			sprintf(lcd_msg.data_L2," AY_r: %.1f", app_params.ylimit);
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_AZ;
			break;
		case edit_AZ:
			val_max = 20;
			step = 1;
			changed_val = &zlimit_10;
			app_params.zlimit = ((float)zlimit_10)/100;	// Aqui reduzco por 10
			sprintf(lcd_msg.data_L2," AZ_r: %.1f", app_params.zlimit);
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				prog_state = edit_hour;

			break;
		
	}
	if(app_params.joy_event == JOY_CENTER_LPULSE && prog_state != disable_edit){
		prog_state = disable_edit;
		SetClock(current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo);
		//app_params.Lref = Lref_internal;
	}else if(app_params.joy_event == JOY_UP_SPULSE)
		*changed_val = (*changed_val+step)%val_max;
	else if(app_params.joy_event == JOY_DOWN_SPULSE){
		*changed_val = (*changed_val-step);
		if(*changed_val > val_max)
			*changed_val = val_max - 1;
	}
	

	osMessageQueuePut(get_id_MsgQueue_lcd(), &lcd_msg, NULL, 0U);
}

static void com_poner_hora(){
	char *data = (char *)app_params.rx_com_msg.data;
	uint8_t index, index_p = 0;
	int horas, minutos, segundos;

	index = strchr(data, ':') - data;
	memcpy(data, data, index);
	horas = atoi(data);
	
	index_p = index + 1;
	data += index_p;
	index = strchr(data, ':') - data;
	memcpy(data, data, index);
	minutos = atoi(data);
	
	index_p = index + 1;
	data += index_p;
	index = strchr(data, ':') - data;
	memcpy(data, data, index);
	segundos = atoi(data);
	
	SetClock(horas, minutos, segundos);
	osMessageQueuePut(app_params.txmsg_queue, &app_params.rx_com_msg, NULL, 0U);
	app_params.rx_com_msg.operation = 0;
}
static float com_get_limit(){
	char *data = (char *)app_params.rx_com_msg.data;
	float limit = atof(data);
	
	return limit;
}
void com_devolver_medidas(){
	//char *data;
	com_msg_t msg = {0};
	uint8_t n_elementos = num_elementos(app_params.circ_buf);
	msg.operation = 0x50;
	for(int elemento = 0; elemento < n_elementos; elemento++){
		recogerDatoBuffer(app_params.circ_buf, msg.data, elemento);
		STM_LOGI(__func__, msg.data);
		msg.size = (strchr(msg.data, '\n') - msg.data);
		//memcpy(msg.data, data, msg.size);
		if(msg.size > 0)
			osMessageQueuePut(app_params.txmsg_queue, &msg, NULL, 1000U);
		
	}
}
void app_gestionar_com(void){
	const uint8_t SET_hour = 0x20, SET_Ax_r = 0x25, SET_Ay_r = 0x26, SET_Az_r = 0x27, GET_buf = 0x55, DEL_buf = 0x60;
	switch(app_params.rx_com_msg.operation){
		case SET_hour:
			com_poner_hora();		// poner la hora
			break;
		case SET_Ax_r:
			app_params.xlimit = com_get_limit();
			osMessageQueuePut(app_params.txmsg_queue, &app_params.rx_com_msg, NULL, 0U);
			break;
		case SET_Ay_r:				
			app_params.ylimit = com_get_limit();
			osMessageQueuePut(app_params.txmsg_queue, &app_params.rx_com_msg, NULL, 0U);
			break;	
		case SET_Az_r:
			app_params.zlimit = com_get_limit();
			osMessageQueuePut(app_params.txmsg_queue, &app_params.rx_com_msg, NULL, 0U);
			break;
		case GET_buf:
			com_devolver_medidas();
			break;
		case DEL_buf:				// Borrar medidas
			borrarDatosBuffer(app_params.circ_buf);
			break;

		default:
			break;
	}
	app_params.rx_com_msg.operation = 0;
}
void app_leds_nucleo_limits(void){
	if(fabs(app_params.accel_temp_var.X_axis) > app_params.xlimit)
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_1_ON));
	else
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_1_OFF));
	if(fabs(app_params.accel_temp_var.Y_axis) > app_params.ylimit)
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_2_ON));
	else
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_2_OFF));
	if(fabs(app_params.accel_temp_var.Z_axis) > app_params.zlimit)
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_3_ON));
	else
		osThreadFlagsSet(app_params.led_thread_id, (1 << LED_3_OFF));
}
void app_stmachine(){
	static sys_state_t sys_state = sys_state_reposo;
	static osStatus_t osstatus; 
	app_params.joy_event = JOY_ERR;			// RESET val
	osstatus = osMessageQueueGet(app_params.joy_msg_queue, &app_params.joy_event, NULL, 0U);
	osstatus = osMessageQueueGet(app_params.accel_temp_queue, &app_params.accel_temp_var, NULL, 0U);
	osstatus = osMessageQueueGet(app_params.rxmsg_queue, &app_params.rx_com_msg, NULL, 0U);
	switch(sys_state){
		case sys_state_reposo:
			app_lcd_display_reposo();
			if(app_params.joy_event == JOY_CENTER_LPULSE)
				sys_state = sys_state_activo;
			break;
		case sys_state_activo:
			app_store_smpl();
			app_lcd_display_activo();
			app_leds_nucleo_limits();
			if(app_params.joy_event == JOY_CENTER_SPULSE)
				sys_state = sys_state_programacion;
			break;
		case sys_state_programacion:
			app_gestionar_com();
			app_lcd_display_programacion();
			if(app_params.joy_event == JOY_CENTER_LPULSE)
				sys_state =  sys_state_reposo;
			break;
		case sys_state_max:
		default:
			sys_state = sys_state_reposo;
			break;
			
	}
	return;
}
void principal_thread(void *args){
	while(1)
		app_stmachine();
}
void init_principal_app(){
	osThreadNew(principal_thread, NULL, NULL);
}
void app_PF_Diciembre24(void){
	stm_err_t ret = STM_OK;
	uint32_t sysclk = HAL_RCC_GetSysClockFreq();
	SWO_Init(0x1, sysclk, 64000);
	// Inicializacion de los CLKs de los puertos
	__HAL_RCC_GPIOA_CLK_ENABLE();			//	Usado por: 	MOSI; SCK; RESET; POT_1
	__HAL_RCC_GPIOB_CLK_ENABLE();			//	Usado por:  	JOY_UP; JOY_RIGHT; SCL, SDA
	__HAL_RCC_GPIOC_CLK_ENABLE();			// 	Usado por:	POT_2; 
	__HAL_RCC_GPIOD_CLK_ENABLE();			// 	Usado por: 	nCS, RGB_R; RGB_G; RGB_B
	__HAL_RCC_GPIOE_CLK_ENABLE();			//	Usado por:	JOY_DOWN, JOY_LEFT; JOY_CENTER; PWM_OUT
	__HAL_RCC_GPIOF_CLK_ENABLE();			//	Usado por:	A0
	
	Init_Th_lcd();
	InitClockThread();
	app_params.joy_msg_queue = osMessageQueueNew(5, sizeof(joy_event_t), NULL);
	init_joystick_proc(app_params.joy_msg_queue);
	app_params.accel_temp_queue = osMessageQueueNew(5, sizeof(accel_temp_var_t), NULL);
	init_accel_temp_proc(app_params.accel_temp_queue);
	app_params.rxmsg_queue = osMessageQueueNew(5, sizeof(com_msg_t), NULL);
	app_params.txmsg_queue = osMessageQueueNew(5, sizeof(com_msg_t), NULL);
	init_com_proc(app_params.rxmsg_queue, app_params.txmsg_queue);
	
	leds_nucleo_init(&app_params.led_thread_id);
	app_params.circ_buf = init_Buffer(N_SAMPLES_BUF, SAMPLE_SIZE);
	STM_GOTO_ON_FALSE(app_params.circ_buf != NULL, STM_ERR_NO_MEM, cleanup,__func__, "Circular buffer could not be created");
	app_params.xlimit = 1;
	app_params.ylimit = 1;
	app_params.zlimit = 1;
	init_principal_app();
	cleanup:
	return;
	
}

