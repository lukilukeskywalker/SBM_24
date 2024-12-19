#include "joystick.h"

typedef struct{
	osThreadId_t thread_id;
	osMessageQueueId_t joy_messagequeue;
}joystick_handle_t;

joystick_handle_t joy_hndl;
uint32_t block_mask = 0;

void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(JOY_UP);
	HAL_GPIO_EXTI_IRQHandler(JOY_RIGHT);
	HAL_GPIO_EXTI_IRQHandler(JOY_DOWN);
	HAL_GPIO_EXTI_IRQHandler(JOY_LEFT);
	HAL_GPIO_EXTI_IRQHandler(JOY_CENTER);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(!(block_mask & GPIO_Pin))		//Si no esta en la lista de exclusion, se avisa al hilo principal
		osThreadFlagsSet(joy_hndl.thread_id, GPIO_Pin);
}
static system_err_t joystick_pinout_config(void){
	// Enable clk to gpio ports
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	// Configure GPIOs
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// PORT_B
	GPIO_InitStruct.Pin = JOY_UP | JOY_RIGHT;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// PORT_E
	GPIO_InitStruct.Pin = JOY_DOWN | JOY_CENTER | JOY_LEFT;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Enable Interrupts in Pins 10-15
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	return sys_ok;  
}
void duracion_cb(void *args){
	uint32_t flags = *(uint32_t *)args;
	joy_message_t joy_message;
	if((flags >> JOY_UP)&1ULL){
		joy_message.pos = up_pos;
		if(HAL_GPIO_ReadPin (GPIOB, JOY_UP) == GPIO_PIN_SET){
			joy_message.dur = long_dur;
		}else
			joy_message.dur = short_dur;
		osMessageQueuePut(joy_hndl.joy_messagequeue, &joy_message, 0, 0);
	}
	else if((flags >> JOY_DOWN)&1ULL){
		joy_message.pos = down_pos;
		if(HAL_GPIO_ReadPin (GPIOE, JOY_DOWN) == GPIO_PIN_SET){
			joy_message.dur = long_dur;
		}else
			joy_message.dur = short_dur;
		osMessageQueuePut(joy_hndl.joy_messagequeue, &joy_message, 0, 0);
	}
	else if((flags >> JOY_RIGHT)&1ULL){
		joy_message.pos = down_pos;
		if(HAL_GPIO_ReadPin (GPIOB, JOY_RIGHT) == GPIO_PIN_SET){
			joy_message.dur = long_dur;
		}else
			joy_message.dur = short_dur;
		osMessageQueuePut(joy_hndl.joy_messagequeue, &joy_message, 0, 0);
	}
	else if((flags >> JOY_CENTER)&1ULL){
		joy_message.pos = center_pos;
		if(HAL_GPIO_ReadPin (GPIOE, JOY_CENTER) == GPIO_PIN_SET){
			joy_message.dur = long_dur;
		}else
			joy_message.dur = short_dur;
		osMessageQueuePut(joy_hndl.joy_messagequeue, &joy_message, 0, 0);
	}
	else if((flags >> JOY_LEFT)&1ULL){
		joy_message.pos = left_pos;
		if(HAL_GPIO_ReadPin (GPIOE, JOY_LEFT) == GPIO_PIN_SET){
			joy_message.dur = long_dur;
		}else
			joy_message.dur = short_dur;
		osMessageQueuePut(joy_hndl.joy_messagequeue, &joy_message, 0, 0);
	}
}
void antirebote_cb(void *args){
	block_mask ^= *(uint32_t *)args;	//Esto hace un flip de los bits de la mascara
	osTimerId_t timer_duracion = osTimerNew(duracion_cb, osTimerOnce, args, NULL);
	osTimerStart(timer_duracion, 50U);
}

void joystick_task(void *args){
	static uint32_t osFlags;
	while(1){
		osFlags = osThreadFlagsWait(0xFFFFFFFF, osFlagsWaitAny, osWaitForever);
		if(osFlags & ((1ULL << JOY_UP) | (1 << JOY_RIGHT) | (1 << JOY_DOWN) | (1 << JOY_CENTER) | (1 << JOY_LEFT))){
			block_mask |= osFlags;
			osTimerNew(antirebote_cb, osTimerOnce, &osFlags, NULL);
		}		
	}
}

system_err_t init_joystick(osMessageQueueId_t joy_messagequeue){
	joystick_pinout_config();
	joy_hndl.thread_id = osThreadNew(joystick_task, NULL, NULL);
	if(joy_hndl.thread_id == NULL)
		return sys_fail;
	return sys_ok;
	joy_hndl.joy_messagequeue = joy_messagequeue;
	
}

