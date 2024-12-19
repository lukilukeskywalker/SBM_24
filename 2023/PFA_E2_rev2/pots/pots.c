#include "pots.h"
//pots_handle_t pots_hndl;


static int conf_pinout(ADC_HandleTypeDef *adc_handle){
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOA_CLK_ENABLE(); 
	__HAL_RCC_GPIOC_CLK_ENABLE(); 
	
	//First portA
	GPIO_InitStruct.Pin = GPIO_PIN_3;	
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//Second portC
	GPIO_InitStruct.Pin = GPIO_PIN_0;	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// Usaremos ADC 1
	__HAL_RCC_ADC1_CLK_ENABLE();
	adc_handle->Instance = ADC1;
	adc_handle->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	adc_handle->Init.Resolution = ADC_RESOLUTION_12B;
	adc_handle->Init.ContinuousConvMode = DISABLE;
	adc_handle->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adc_handle->Init.DiscontinuousConvMode = DISABLE;
	adc_handle->Init.DMAContinuousRequests = DISABLE;
	adc_handle->Init.EOCSelection = ADC_EOC_SINGLE_CONV; //
	adc_handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
	adc_handle->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	adc_handle->Init.NbrOfConversion = 1;
	adc_handle->Init.ScanConvMode = DISABLE;
	HAL_ADC_Init(adc_handle);
	return 0;
}
static uint16_t ADC_Poll(ADC_HandleTypeDef *adc_handle, ADC_ChannelConfTypeDef *adc_channel_conf){
	static uint16_t value;
	HAL_ADC_ConfigChannel(adc_handle, adc_channel_conf);
	HAL_ADC_Start(adc_handle);
	HAL_ADC_PollForConversion(adc_handle, 100);
	value = HAL_ADC_GetValue(adc_handle);
	HAL_ADC_Stop(adc_handle);
	return value;
}

void pots_task(void *args){
	pots_message_t pots_message;
	pots_handle_t *pots_hndl = (pots_handle_t *)args; 
	static ADC_ChannelConfTypeDef adc_CH3_conf = {
		.Channel = ADC_CHANNEL_3,
		.Rank = 1,
		.SamplingTime = ADC_SAMPLETIME_3CYCLES,
	};
	static ADC_ChannelConfTypeDef adc_CH10_conf = {
		.Channel = ADC_CHANNEL_10,
		.Rank = 1,
		.SamplingTime = ADC_SAMPLETIME_3CYCLES,
	};
	while(1){
		pots_message.pot_1 = ADC_Poll(pots_hndl->adc_handle, &adc_CH3_conf);
		pots_message.pot_2 = ADC_Poll(pots_hndl->adc_handle, &adc_CH10_conf);	//Esto es imposible que sea la forma correcta de hacerlo...
		osMessageQueuePut(pots_hndl->pots_messagequeue, &pots_message, 0, 0);
		osDelay(1000);
	}
}

system_err_t init_pots(osMessageQueueId_t pots_messagequeue){
	static pots_handle_t *pots_hndl;
	if(pots_hndl != NULL)
		return sys_err_invalid_state;
	if(pots_messagequeue == NULL)
		return sys_err_invalid_arg;
	pots_hndl = calloc(1, sizeof(pots_handle_t));
	if(pots_hndl == NULL)
		return sys_err_no_mem;
	conf_pinout(pots_hndl->adc_handle);
	pots_hndl->pots_messagequeue = pots_messagequeue;
	
	pots_hndl->pots_task_id = osThreadNew(pots_task, &pots_hndl, NULL);
	if(pots_hndl->pots_task_id == NULL)
		return sys_fail;
	return sys_ok;
}