#ifndef __THJOY_H
#define __THJOY_H

#define MSGQUEUE_OBJECTS 16                     // number of Message Queue Objects


/*Includes*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"

/*Funciones*/
int Init_MsgQueue (void);
void Thread2 (void *argument);
int Init_Thread2 (void);
static void Timer1_Callback (void const *arg);
int Init_Timers (void);
void Thread (void *argument);
int Init_Thread (void) ;
void Init_Joystick(void);




#endif 
