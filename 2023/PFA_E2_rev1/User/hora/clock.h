#ifndef CLOCK_H
#define CLOCK_H

#include "cmsis_os2.h" 

int Init_Th_clk (void);
void set_clock (int hour, int minutes, int seconds);

#endif
