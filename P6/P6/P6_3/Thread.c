#include "cmsis_os2.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

extern int fin_timer;
extern osTimerId_t tim_id1;

osThreadId_t tid_Thread;                        // thread id
 
void Thread (void *argument);                   // thread function
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
 
  while (1) {
		osThreadFlagsWait (0x04, osFlagsWaitAny , osWaitForever);
    if( fin_timer != 1){
			osTimerStart(tim_id1, 3000U); 
		}
    osThreadYield();                            // suspend thread
  }
}
