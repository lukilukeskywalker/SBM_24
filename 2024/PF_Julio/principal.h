#ifndef __PRINCIPAL_H__
#define __PRINCIPAL_H__

typedef enum{
	sys_state_reposo,				//!< Estado inicial, RGB_LEDG en PWM [10, 90]% T: 4s (2s en crecimiento, 2s en decrecimiento)   
	sys_state_manual,				//!< POT1->Lref[1000, 65000], POT2->PWM_LED[5, 95]%, RGB_LEDR = (Lmed - Lref >= 5001)? 1 : 0    
	sys_state_automatico,		//!< Lref - Lmed -+> PWM_LED (PWM_LED debe poder acercarse a Lref, Lmed indicara Lum actual de PWM_LED), Buf_circ RGB_LEDB = (Lmed -Lref >= 5001) ?  1 : 0, Buf_circ[20]
	sys_state_programacion,	//!< Modificar Hora y Lref con pulsaciones cortas del Joystick. Se efectuaran los cambios con una pulsacion corta en JOY_CENTER, Lref pasos de 1000: Aceptara comandos por RS232
	sys_state_max						//!< Maximo estado, Estado no valido
}sys_state_t;							//!< Todos los estados pasan al siguiente nivel con: JOY_CENTER pulsacion larga

void app_PF_Julio24(void);


#endif
