#ifndef __EXAM_HELPER_H__
#define __EXAM_HELPER_H__ 

#include <stdint.h>

void BTN1_init(void);
void LEDs_init(void);


void tim3_chn3_init(uint32_t desired_tick_hz, uint16_t counter_depth);		// OC LD1
void tim4_chn2_init(uint32_t desired_tick_hz, uint16_t counter_depth);		// OC LD2
void tim12_chn1_init(uint32_t desired_tick_hz, uint16_t counter_depth);		// OC LD3

void tim4_init(uint32_t desired_tick_hz, uint16_t counter_depth);
void tim7_init(uint32_t desired_tick_hz, uint16_t counter_depth);


#endif
