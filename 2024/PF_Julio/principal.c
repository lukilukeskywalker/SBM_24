#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "principal.h"
#include "stm_err.h"
#include "stm_log.h"
#include "stm32f4xx_hal.h"
#include "led_control/led_control.h"
#include "com/com.h"
#include "LCD/lcd.h"
#include "hora/clock.h"
#include "pot/pot.h"
#include "joystick/joystick.h"
#include "pwm/pwm.h"
#include "circ_buf.h"



#include "lum/lum.h"
//#include "circ_buf.h"

extern program_clock_t current_time;

typedef struct{
	osMessageQueueId_t joy_msg_queue;
	joy_event_t joy_event;
	osMessageQueueId_t pot_msg_queue;
	pot_msg_t pot_msg;
	osMessageQueueId_t lum_msg_queue;
	sample_luminance_t lum_msg;
	osMessageQueueId_t rxmsg_queue; 
	osMessageQueueId_t txmsg_queue;
	com_msg_t rx_com_msg;
	uint16_t Lref;
	uint16_t PWM;
	osMessageQueueId_t pwm_msg_queue;
	osMessageQueueId_t rgb_queue_id;
	buffer_t *buf;
}app_params_t;

app_params_t app_params;



typedef enum{
	disable_edit,
	edit_hour,
	edit_min,
	edit_sec,
	edit_lref,
}prog_state_t;

void app_lcd_display_reposo(){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static program_clock_t current_time_internal;
	if(0 == memcmp(&current_time, &current_time_internal, sizeof(program_clock_t)))
		return;		// No changes
	sprintf(lcd_msg.data_L1, "     SBM 2024");
	sprintf(lcd_msg.data_L2, "      %.2u:%.2u:%.2u", current_time.hora, current_time.minuto, current_time.segundo);
	osMessageQueuePut(get_id_MsgQueue_lcd(), &lcd_msg, NULL, 0U);
}
void app_lcd_display_manual_auto(char mode, uint8_t pwm, uint8_t Lmed, uint8_t Lref){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static program_clock_t current_time_internal;
	if(0 == memcmp(&current_time, &current_time_internal, sizeof(program_clock_t)))
		return;		// No changes
	sprintf(lcd_msg.data_L1, " %c-PWM %d %%", mode, pwm);
	sprintf(lcd_msg.data_L2, "      %.2u:%.2u:%.2u", current_time.hora, current_time.minuto, current_time.segundo);
	lcd_msg.dial[0] = Lmed;
	lcd_msg.dial[1] = Lref;
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
static void com_poner_Lref(){
	char *data = (char *)app_params.rx_com_msg.data;
	uint16_t Lref = atoi(data);
	app_params.Lref = Lref;
	osMessageQueuePut(app_params.txmsg_queue, &app_params.rx_com_msg, NULL, 0U);
	app_params.rx_com_msg.operation = 0;
}



void com_devolver_medidas(){
	lum_sample_t lum_sample;
	com_msg_t msg = {0};
	uint8_t n_elementos = num_elementos(app_params.buf);
	msg.operation = 0x50;
	for(int elemento = 0; elemento < n_elementos; elemento++){
		recogerDatoBuffer(app_params.buf, &lum_sample, elemento);
		msg.size = sprintf((char *)msg.data, "%02d:%02d:%02d--Lmed:%05d--Lref:%05d--PWM:%02d%%",
			lum_sample.sample_time.hora,
			lum_sample.sample_time.minuto,
			lum_sample.sample_time.segundo,
			lum_sample.luminance.luminance,
			lum_sample.Lref,
			lum_sample.PWM
			);
		if(msg.size > 0)
			osMessageQueuePut(app_params.txmsg_queue, &msg, NULL, 1000U);
		
	}
}
void app_gestionar_com(void){
	switch(app_params.rx_com_msg.operation){
		case 0x20:
			com_poner_hora();		// poner la hora
			break;
		case 0x25:
			com_poner_Lref();		// Establecer la luminosidad de referencia
			break;
		case 0x55:				// Devolver todas las medidas realizadas
			com_devolver_medidas();
			break;	
		case 0x60:				// Borrar medidas
			borrarDatosBuffer(app_params.buf);
			break;

		default:
			break;
	}
}
void app_lcd_display_programacion(){
	static MSGQUEUE_OBJ_LCD lcd_msg;
	static program_clock_t current_time_internal;
	static uint16_t Lref_internal = 23000, step = 0;
	static prog_state_t prog_state = disable_edit;
	static uint16_t val_max = 24, *changed_val = &current_time_internal.hora; 
	sprintf(lcd_msg.data_L1, " PROGRAMACION");
	switch(prog_state){
		case disable_edit:
			memcpy(&current_time_internal, &current_time, sizeof(program_clock_t));
			if(app_params.joy_event == JOY_RIGHT_SPULSE || app_params.joy_event == JOY_CENTER_SPULSE){
				prog_state = edit_hour;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_lref;
			}
			sprintf(lcd_msg.data_L2, " H %.2u:%.2u:%.2u - Lr %d", current_time.hora, current_time.minuto, current_time.segundo, app_params.Lref);
			break;
		case edit_hour:
			sprintf(lcd_msg.data_L2, " H [%.2u]:%.2u:%.2u - Lr %d", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo, Lref_internal);
			val_max = 24;
			step = 1;
			changed_val = &current_time_internal.hora;	
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_min;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_lref;
			}
			break;
		case edit_min:
			sprintf(lcd_msg.data_L2, " H %.2u:[%.2u]:%.2u - Lr %d", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo, Lref_internal);
			val_max = 60;
			step = 1;
			changed_val = &current_time_internal.minuto;
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_sec;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_hour;
			}
			break;
		case edit_sec:
			val_max = 60;
			step = 1;
			changed_val = &current_time_internal.segundo;
			sprintf(lcd_msg.data_L2, " H %.2u:%.2u:[%.2u] - Lr %d", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo, Lref_internal);
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_lref;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_min;
			}
			break;
		case edit_lref:
			changed_val = &Lref_internal;
			val_max = 65000;
			step = 1000; 
			sprintf(lcd_msg.data_L2, " H %.2u:%.2u:%.2u - Lr [%d]", current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo, Lref_internal);
			if(app_params.joy_event == JOY_RIGHT_SPULSE){
				prog_state = edit_hour;
			}else if(app_params.joy_event == JOY_LEFT_SPULSE){
				prog_state = edit_sec;
			}
			break;
	}
	if(app_params.joy_event == JOY_CENTER_SPULSE && prog_state != disable_edit){
		prog_state = disable_edit;
		SetClock(current_time_internal.hora, current_time_internal.minuto, current_time_internal.segundo);
		app_params.Lref = Lref_internal;
	}else if(app_params.joy_event == JOY_UP_SPULSE)
		*changed_val = (*changed_val+step)%val_max;
	else if(app_params.joy_event == JOY_DOWN_SPULSE){
		*changed_val = (*changed_val-step);
		if(*changed_val > val_max)
			*changed_val = val_max - 1;
	}
	

	osMessageQueuePut(get_id_MsgQueue_lcd(), &lcd_msg, NULL, 0U);
}
void guardar_lum(){
	lum_sample_t lum_sample;
	memcpy(&lum_sample.sample_time, &current_time, sizeof(program_clock_t));
	memcpy(&lum_sample.luminance, &app_params.lum_msg.luminance, sizeof(sample_luminance_t));
	lum_sample.Lref = app_params.Lref;
	lum_sample.PWM = app_params.PWM;
	introducirDatoBuffer(app_params.buf, &lum_sample);
	return;
}

void app_stmachine(){
	static sys_state_t sys_state = sys_state_reposo;
	static osStatus_t osstatus;
	uint8_t PWM_int = 0, Lmed = 0;
	static app_params_t *app_params_p = &app_params;
	osstatus = osMessageQueueGet(app_params.joy_msg_queue, &app_params.joy_event, NULL, 0U);
	osstatus = osMessageQueueGet(app_params.pot_msg_queue, &app_params.pot_msg, NULL, 0U);
	osstatus = osMessageQueueGet(app_params.lum_msg_queue, &app_params.lum_msg, NULL, 0U);
	if(osstatus == osOK)
		guardar_lum();
	osstatus = osMessageQueueGet(app_params.rxmsg_queue, &app_params.rx_com_msg, NULL, 0U);
	MSGQUEUE_OBJ_RGB msg_rgb;
	msg_rgb.estado_led2 = ON;
	osMessageQueuePut(app_params.rgb_queue_id, &msg_rgb, NULL, 0U);
	switch(sys_state){
		case sys_state_reposo:
			app_lcd_display_reposo();
			if(app_params.joy_event == JOY_CENTER_LPULSE)
				sys_state = sys_state_manual;
			break;
		case sys_state_manual: 
			app_params.PWM = app_params.pot_msg.pot[1];
			PWM_int = (((app_params.pot_msg.pot[1]>>4) * 100)/255);
			Lmed = (((app_params.lum_msg.luminance >> 8) * 100) / 255);
			app_params.Lref = ((app_params.pot_msg.pot[0]>>4) * 100)/255;
			app_lcd_display_manual_auto('M', PWM_int , Lmed, app_params.Lref);	// PWM <-> POT2, Lmed = Lmed, Lref <-> POT1
			if(app_params.joy_event == JOY_CENTER_LPULSE)
				sys_state = sys_state_automatico;
			break;
		case sys_state_automatico:
			Lmed = (((app_params.lum_msg.luminance >> 8) * 100) / 255);
			app_params.PWM = app_params.pot_msg.pot[0] - (app_params.lum_msg.luminance >> 4);
			PWM_int = (((app_params.PWM>>4) * 100)/255);
			app_params.Lref = ((app_params.pot_msg.pot[0]>>4) * 100)/255;
			app_lcd_display_manual_auto('A', PWM_int , Lmed, app_params.Lref);	// PWM <-> POT2, Lmed = Lmed, Lref <-> POT1
			if(app_params.joy_event == JOY_CENTER_LPULSE){
				app_params.Lref = 23000;
				sys_state = sys_state_programacion;
			}
			break;
		case sys_state_programacion:
			app_gestionar_com();
			app_lcd_display_programacion();
			if(app_params.joy_event == JOY_CENTER_LPULSE)
				sys_state = sys_state_reposo;
			break;
		case sys_state_max:
		default:
			sys_state = sys_state_reposo;
			break;
	}
	osMessageQueuePut(app_params.pwm_msg_queue, &app_params.PWM, NULL, 0U);
	app_params.joy_event = 0;
}
void principal_thread(void *args){
	while(1)
		app_stmachine();
}
void init_principal_app(){
	osThreadNew(principal_thread, NULL, NULL);
}


void app_PF_Julio24(void){
	uint32_t sysclk = HAL_RCC_GetSysClockFreq();
	SWO_Init(0x1, sysclk, 64000);
	// Inicializacion de los CLKs de los puertos
	__HAL_RCC_GPIOA_CLK_ENABLE();			//	Usado por: 	MOSI; SCK; RESET; POT_1
	__HAL_RCC_GPIOB_CLK_ENABLE();			//	Usado por:  	JOY_UP; JOY_RIGHT; SCL, SDA
	__HAL_RCC_GPIOC_CLK_ENABLE();			// 	Usado por:	POT_2; 
	__HAL_RCC_GPIOD_CLK_ENABLE();			// 	Usado por: 	nCS, RGB_R; RGB_G; RGB_B
	__HAL_RCC_GPIOE_CLK_ENABLE();			//	Usado por:	JOY_DOWN, JOY_LEFT; JOY_CENTER; PWM_OUT
	__HAL_RCC_GPIOF_CLK_ENABLE();			//	Usado por:	A0
	// Nota: 	Para ahorrar energia, se puede utilizar CLK Gating, probar.
	
	Init_Th_lcd();
	InitClockThread();
	app_params.joy_msg_queue = osMessageQueueNew(5, sizeof(joy_event_t), NULL);
	init_joystick_proc(app_params.joy_msg_queue);
	app_params.pot_msg_queue =  osMessageQueueNew(5, sizeof(pot_msg_t), NULL);
	init_pot_proc(app_params.pot_msg_queue);
	app_params.lum_msg_queue =  osMessageQueueNew(5, sizeof(sample_luminance_t), NULL);
	init_lum_proc(app_params.lum_msg_queue);
	app_params.rxmsg_queue = osMessageQueueNew(5, sizeof(com_msg_t), NULL);
	app_params.txmsg_queue = osMessageQueueNew(5, sizeof(com_msg_t), NULL);
	init_com_proc(app_params.rxmsg_queue, app_params.txmsg_queue);
	app_params.pwm_msg_queue = osMessageQueueNew(5, sizeof(uint32_t), NULL);
	init_pwm_proc(app_params.pwm_msg_queue);
	init_principal_app();
	app_params.Lref = 25000;
	app_params.buf = (buffer_t *) calloc(1, sizeof(buffer_t));
	app_params.buf = init_Buffer(app_params.buf, 20);
	Init_Th_rgb();
	app_params.rgb_queue_id = led_control_msgQueue();
	//SWO_PrintString("Prueba\r\n", 0);
	//init_com(&Driver_USART3);
}
