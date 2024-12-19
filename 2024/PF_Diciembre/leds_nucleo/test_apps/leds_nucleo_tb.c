#include <stdio.h>
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "../leds_nucleo.h"

// Flag positions (as per your thread's logic)




// Testbench thread function
void leds_nucleo_tb_th(void *args) {
	osThreadId_t led_thread_id;
	leds_nucleo_init(&led_thread_id);
	while (1) {
		// Turn LED 1 ON
		osThreadFlagsSet(led_thread_id, (1 << LED_1_ON));
		osDelay(1000);  // 1-second delay
		
		// Turn LED 1 OFF
		osThreadFlagsSet(led_thread_id, (1 << LED_1_OFF));
		osDelay(1000);
		
		// Turn LED 2 ON
		osThreadFlagsSet(led_thread_id, (1 << LED_2_ON));
		osDelay(1000);
		
		// Turn LED 2 OFF
		osThreadFlagsSet(led_thread_id, (1 << LED_2_OFF));
		osDelay(1000);
		
		// Turn LED 3 ON
		osThreadFlagsSet(led_thread_id, (1 << LED_3_ON));
		osDelay(1000);

		// Turn LED 3 OFF
		osThreadFlagsSet(led_thread_id, (1 << LED_3_OFF));
		osDelay(1000);
    }
}

