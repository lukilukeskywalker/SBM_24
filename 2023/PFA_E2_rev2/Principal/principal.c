/**
* @author Lukas Gdanietz
* @note El modulo principal es el encargado de recibir las conexiones principales con los otros modulos.
*	Primero se encarga de generar las colas de coms, las cuales organizare en un struct para facil gestion. Estas colas se las pasaremos a los hilos
*
*/

#include "principal.h"
time_t time = {0, 2, 5}; 


system_err_t principal_deinit(principal_handle_t *handle){
	system_err_t ret = sys_ok;
	if(handle==NULL)
		return sys_err_invalid_arg;
	for(int modulo = 0; modulo < module_max; modulo++){
		if(handle->module_queues[modulo] != NULL)
			osMessageQueueDelete(handle->module_queues[modulo]);
	}
	return ret;
}
system_err_t principal_init(void){
	system_err_t ret = sys_ok;
	principal_handle_t *pri_hndl = (principal_handle_t *)calloc(1, sizeof(principal_handle_t));
	//SYS_GOTO_ON_FALSE(pri_hndl != NULL, sys_err_no_mem, cleanup);
	SYS_RETURN_ON_FALSE(pri_hndl != NULL, sys_err_no_mem);
	for(int modulo = 0; modulo < module_max; modulo++){
		pri_hndl->module_queues[modulo] = osMessageQueueNew(QUEUE_SIZE, module_size[modulo], NULL);
		SYS_GOTO_ON_FALSE(pri_hndl->module_queues[modulo] != NULL, sys_err_no_mem, cleanup);			
	}
	init_time(&time);
	temp_init(pri_hndl->module_queues[module_temperatura], NULL);
	
	return ret;
	cleanup:
	principal_deinit(pri_hndl);
	return ret;	
}

