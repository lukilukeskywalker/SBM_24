#include "hora.h"

typedef struct{
	osTimerId_t timer_ID;
	time_t *time;
}hora_handle_t;

static hora_handle_t hora_hndl = {0};

static void time_correction(time_t *time){
	//Teoricamente puede suceder que el cambio de contexto suceda entre medias de esta funcion y quede como resultado una hora imposible hasta que continue la funcion
	//Se solucionaria con una variable intermedia, pero es bastante improbable, asi que lo dejare asi, por ahora.
	time->min += (time->sec/60);
	time->hora += (time->min/60);
	time->sec = time->sec%60;
	time->min = time->min%60;
	time->hora = time->hora%24;
}

static void timer_time_cb(void *arg){
	time_t *time = (time_t *)arg;
	time->sec++;
	time_correction(time);
}

system_err_t init_time(time_t * time){
	system_err_t ret = sys_ok;
	if(time == NULL)
		return sys_err_invalid_arg;
	if(hora_hndl.timer_ID != NULL)
		return sys_err_invalid_state;
	hora_hndl.timer_ID = osTimerNew(timer_time_cb, osTimerPeriodic, time, NULL);
	SYS_RETURN_ON_FALSE(hora_hndl.timer_ID != NULL, sys_err_no_mem);
	hora_hndl.time = time;
	SYS_GOTO_ON_FALSE(osOK !=osTimerStart(timer_time_cb, 1000U), sys_fail, cleanup);
	
	return ret;
	cleanup:
	osTimerDelete(hora_hndl.timer_ID);
	hora_hndl.timer_ID = NULL;
	return ret;		//Should delete
}
void set_time(uint8_t hora, uint8_t min, uint8_t sec){
	hora_hndl.time->hora = hora;
	hora_hndl.time->min = min;
	hora_hndl.time->sec = sec;
	time_correction(hora_hndl.time);
}
