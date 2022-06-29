#include "clock.h"

int hours = 0;
int minutes = 0;
int seconds = 0;

void Th_clk (void *argument);
static void Timer_clk_Callback (void*arg);

osTimerId_t tim_clk;
osThreadId_t tid_Th_clk;
 
int Init_Th_clk (void) {
  tid_Th_clk = osThreadNew(Th_clk, NULL, NULL);
  if (tid_Th_clk == NULL)
    return(-1);
  return(0);
}
 
void Th_clk (void *argument) {
  tim_clk = osTimerNew(Timer_clk_Callback, osTimerPeriodic, NULL, NULL);
	osTimerStart(tim_clk, 1000U);
  while(1)	
		osThreadYield();
}

static void Timer_clk_Callback (void*arg) {
	seconds++;
	if(seconds == 60){
		seconds = 0;
		minutes++;
		if(minutes == 60){
			minutes = 0;
			hours++;
			if(hours == 24){
				hours = 0;
			}
		}
	}
}

void set_clock (int hour, int min, int sec) {
	hours = (hour < 24) ? hour : 0;
	minutes = (min < 60) ? min : 0;
	seconds = (sec < 60) ? sec : 0;
}
