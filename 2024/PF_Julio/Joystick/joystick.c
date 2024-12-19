#include "joystick.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

#define JOY_DLC_PORT	GPIOE
#define JOY_UR_PORT	GPIOB
#define JOY_DOWN	GPIO_PIN_12		//0x1000
#define JOY_LEFT	GPIO_PIN_14		//0x4000
#define JOY_CENTER	GPIO_PIN_15		//0x8000
#define JOY_UP		GPIO_PIN_10		//0x0400
#define JOY_RIGHT	GPIO_PIN_11		//0x0800

#define SHORT_PULSE_MASK		0x38	// b'11100
#define LONG_PULSE_MASK			0x3f	// b'11111
#define ARE_ALL_BITS_SET(x, mask) (((x) & (mask)) == (mask))
#define JOY_DOWN_LPULSE_FILTER(x)    ARE_ALL_BITS_SET((x), LONG_PULSE_MASK)
#define JOY_LEFT_LPULSE_FILTER(x)    ARE_ALL_BITS_SET((x), (LONG_PULSE_MASK << 6))
#define JOY_CENTER_LPULSE_FILTER(x)  ARE_ALL_BITS_SET((x), (LONG_PULSE_MASK << 12))
#define JOY_UP_LPULSE_FILTER(x)      ARE_ALL_BITS_SET((x), (LONG_PULSE_MASK << 18))
#define JOY_RIGHT_LPULSE_FILTER(x)   ARE_ALL_BITS_SET((x), (LONG_PULSE_MASK << 24))

#define JOY_DOWN_SPULSE_FILTER(x)    (x & SHORT_PULSE_MASK)         //ARE_ALL_BITS_SET((x), SHORT_PULSE_MASK)
#define JOY_LEFT_SPULSE_FILTER(x)    (x & (SHORT_PULSE_MASK << 6))  //ARE_ALL_BITS_SET((x), (SHORT_PULSE_MASK << 6))
#define JOY_CENTER_SPULSE_FILTER(x)  (x & (SHORT_PULSE_MASK << 12)) //ARE_ALL_BITS_SET((x), (SHORT_PULSE_MASK << 12))
#define JOY_UP_SPULSE_FILTER(x)      (x & (SHORT_PULSE_MASK << 18)) //ARE_ALL_BITS_SET((x), (SHORT_PULSE_MASK << 18))
#define JOY_RIGHT_SPULSE_FILTER(x)   (x & (SHORT_PULSE_MASK << 24)) //ARE_ALL_BITS_SET((x), (SHORT_PULSE_MASK << 24))

#define DOWN_FLAG			1ULL
#define LEFT_FLAG			(1ULL << 6)
#define CENTER_FLAG			(1ULL << 12)
#define UP_FLAG				(1ULL << 18)
#define RIGHT_FLAG			(1ULL << 24)
#define ISR_FLAG			(1ULL << 30)la 


#define TAG "JOYSTICK"
#define _LOG_LEVEL 3

typedef struct{
	osMessageQueueId_t *queueId;
}joystick_handle_t;

static osThreadId_t *joystick_th_id = NULL;		// Stores the thread's id
static const uint32_t io_mask = JOY_DOWN | JOY_LEFT | JOY_CENTER | JOY_UP | JOY_RIGHT;		//Mascara de los IOs habilitados

void joystick_isr(void);

void EXTI15_10_IRQHandler(void){
	joystick_isr();
	return;
}

void joystick_isr(void){
	//const uint32_t io_mask = JOY_DOWN | JOY_LEFT | JOY_CENTER | JOY_UP | JOY_RIGHT;		//Mascara de los IOs habilitados
	osStatus_t status = 0;
	uint32_t io_interrupt = __HAL_GPIO_EXTI_GET_IT(io_mask), flag = 0; //EXTI->PR & io_mask;		//Filtramos los pines que han producido la interrupcion con la mascara
	__HAL_GPIO_EXTI_CLEAR_IT(io_interrupt);							//Borramos los bits de registro de interrupcion que hemos recibido previamente
	if(joystick_th_id == NULL)
		return;										//Volver, no podemos procesar la interrupcion si el hilo todavia no existe
	if(io_interrupt & JOY_DOWN)
		flag = DOWN_FLAG;
	if(io_interrupt & JOY_LEFT)
		flag |= LEFT_FLAG;
	if(io_interrupt & JOY_CENTER)
		flag |= CENTER_FLAG;
	if(io_interrupt & JOY_UP)
		flag |= UP_FLAG;
	if(io_interrupt & JOY_RIGHT)
		flag |= RIGHT_FLAG;
	flag |= ISR_FLAG;
	status = osThreadFlagsSet(joystick_th_id, flag);						//Notificamos al hilo del proceso de la interrupcion
	return;
}

void joystick_tmr_cb(void *args){
	uint32_t flags = *(uint32_t *)args, new_flags = 0;
	new_flags = ((GPIOE->IDR & JOY_CENTER))? CENTER_FLAG : 0;
	new_flags |= ((GPIOE->IDR & JOY_LEFT))? LEFT_FLAG : 0;
	new_flags |= ((GPIOE->IDR & JOY_DOWN))? DOWN_FLAG : 0;
	new_flags |= ((GPIOB->IDR & JOY_UP))? UP_FLAG : 0;
	new_flags |= ((GPIOB->IDR & JOY_RIGHT))? RIGHT_FLAG : 0;
	flags = (flags << 1) | new_flags;
	flags = flags & (~ISR_FLAG);
	osThreadFlagsSet(joystick_th_id, flags);
	return;
}

static void init_joystick_gpi(void){
	//HAL GPIO Structure definition and Init
	GPIO_InitTypeDef GPIOB_conf = {
		.Pin = JOY_UP | JOY_RIGHT,
		.Mode = GPIO_MODE_IT_RISING,
		.Pull = GPIO_PULLDOWN,
	},
	GPIOE_conf = {
		.Pin = JOY_DOWN | JOY_LEFT | JOY_CENTER,
		.Mode = GPIO_MODE_IT_RISING,
		.Pull = GPIO_PULLDOWN,
	};
	//Init Peripheral CLKs
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	//Configure IOs
	HAL_GPIO_Init(GPIOB, &GPIOB_conf);
	HAL_GPIO_Init(GPIOE, &GPIOE_conf);
	// Enable SRC IRQ
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	return;
}
void joystick_thread(void* args){
	uint32_t task_flags = 0, gpi_mem = 0, timer_counter = 0;
	osTimerId_t tmr_50ms = osTimerNew(joystick_tmr_cb, osTimerOnce, &gpi_mem, NULL);
	osStatus_t osstatus = 0;
	//joystick_handle_t *joy_hndl = (joystick_handle_t *) args;
	osMessageQueueId_t *queueId = (osMessageQueueId_t *)args;
	joy_event_t joy_event = 0;
	
	while(1){
		task_flags = osThreadFlagsWait(0x7fffffff, osFlagsWaitAny, osWaitForever);
		timer_counter++;
		gpi_mem = task_flags & ~ISR_FLAG;
		osTimerStart(tmr_50ms, 75);
		if(task_flags & ISR_FLAG){
			timer_counter = 0;
		}else if(timer_counter == 6){
			osTimerStop(tmr_50ms);
			if(JOY_CENTER_LPULSE_FILTER(gpi_mem))
				joy_event = JOY_CENTER_LPULSE;
			else if(JOY_CENTER_SPULSE_FILTER(gpi_mem))
				joy_event = JOY_CENTER_SPULSE;
			else if (JOY_LEFT_LPULSE_FILTER(gpi_mem))
				joy_event = JOY_LEFT_LPULSE;
			else if (JOY_LEFT_SPULSE_FILTER(gpi_mem))
				joy_event = JOY_LEFT_SPULSE;
			else if (JOY_DOWN_LPULSE_FILTER(gpi_mem))
				joy_event = JOY_DOWN_LPULSE;
			else if (JOY_DOWN_SPULSE_FILTER(gpi_mem))
				joy_event = JOY_DOWN_SPULSE;
			else if (JOY_UP_LPULSE_FILTER(gpi_mem))
				joy_event = JOY_UP_LPULSE;
			else if (JOY_UP_SPULSE_FILTER(gpi_mem))
				joy_event = JOY_UP_SPULSE;
			else if (JOY_RIGHT_LPULSE_FILTER(gpi_mem))
				joy_event = JOY_RIGHT_LPULSE;
			else if (JOY_RIGHT_SPULSE_FILTER(gpi_mem))
				joy_event = JOY_RIGHT_SPULSE;
			if(joy_event != 0)
				osstatus = osMessageQueuePut(queueId, &joy_event, NULL, 1000);
			gpi_mem = 0;
			joy_event = 0;

		}
		
	}
	return;
}
stm_err_t init_joystick_proc(osMessageQueueId_t *queueId){
	STM_RETURN_ON_FALSE(queueId != NULL, STM_ERR_INVALID_ARG, __func__, "queueId can't be null");
	STM_RETURN_ON_FALSE(joystick_th_id == NULL, STM_ERR_INVALID_STATE, __func__, "Joystick task exists already");
	/*joystick_handle_t joy_hndl = {
		.queueId = queueId,
	};
	*/
	joystick_th_id = osThreadNew(joystick_thread, queueId, NULL);
	STM_RETURN_ON_FALSE(joystick_th_id != NULL, STM_FAIL, __func__, "Joystick task gen failed");
	init_joystick_gpi();
	STM_LOGD(__func__, "Joystick init completed");
	return STM_OK;
}
