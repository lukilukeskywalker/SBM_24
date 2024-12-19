#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "cmsis_os2.h"
#define __PROGRAM_CLOCK_H__
typedef struct {
    uint16_t hora;
    uint16_t minuto;
    uint16_t segundo;
} program_clock_t;


int InitClockThread(void);

// Función para establecer la hora manualmente
void SetClock(uint16_t hour, uint16_t minutes, uint16_t seconds);

#endif 
