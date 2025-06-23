#ifndef __EXAM_HELPER_H__
#define __EXAM_HELPER_H__ 

#include <stdint.h>

void BTN1_init(void);
void LEDs_init(void);

void tim4_init(uint32_t desired_tick_hz, uint16_t counter_depth);

#endif