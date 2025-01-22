#ifndef __SBMINIT_H__
#define __SBMINIT_H__
#include <stdlib.h>
#include <stdint.h>


void leds_nucleo_gpo_init(void);
void tim2_init(void);
void tim11_oc_interrupt(uint16_t toggleperiod_us);

#endif