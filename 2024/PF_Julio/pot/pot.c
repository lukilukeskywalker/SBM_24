#include "pot.h"
#include "../stm_log.h"
#include "stm32f4xx_hal.h"
#include <string.h>


static ADC_ChannelConfTypeDef sadc_ch3 = {
	.Channel = 3,
	.Rank = 1,                   /*!< Specifies the rank in the regular group sequencer. This parameter must be a number between Min_Data = 1 and Max_Data = 16 */
	.SamplingTime = ADC_SAMPLETIME_3CYCLES,           /*!< Sampling time value to be set for the selected channel.
                                       Unit: ADC clock cycles
                                       Conversion time is the addition of sampling time and processing time (12 ADC clock cycles at ADC resolution 12 bits, 11 cycles at 10 bits, 9 cycles at 8 bits, 7 cycles at 6 bits).
                                       This parameter can be a value of @ref ADC_sampling_times
                                       Caution: This parameter updates the parameter property of the channel, that can be used into regular and/or injected groups.
                                                If this same channel has been previously configured in the other group (regular/injected), it will be updated to last setting.
                                       Note: In case of usage of internal measurement channels (VrefInt/Vbat/TempSensor),
                                             sampling time constraints must be respected (sampling time can be adjusted in function of ADC clock frequency and sampling time setting)
                                             Refer to device datasheet for timings values, parameters TS_vrefint, TS_temp (values rough order: 4us min). */
	.Offset = 0,
};
static ADC_ChannelConfTypeDef sadc_ch10 = {
	.Channel = 10,
	.Rank = 1,                   /*!< Specifies the rank in the regular group sequencer. This parameter must be a number between Min_Data = 1 and Max_Data = 16 */
	.SamplingTime = ADC_SAMPLETIME_3CYCLES,           /*!< Sampling time value to be set for the selected channel.
                                       Unit: ADC clock cycles
                                       Conversion time is the addition of sampling time and processing time (12 ADC clock cycles at ADC resolution 12 bits, 11 cycles at 10 bits, 9 cycles at 8 bits, 7 cycles at 6 bits).
                                       This parameter can be a value of @ref ADC_sampling_times
                                       Caution: This parameter updates the parameter property of the channel, that can be used into regular and/or injected groups.
                                                If this same channel has been previously configured in the other group (regular/injected), it will be updated to last setting.
                                       Note: In case of usage of internal measurement channels (VrefInt/Vbat/TempSensor),
                                             sampling time constraints must be respected (sampling time can be adjusted in function of ADC clock frequency and sampling time setting)
                                             Refer to device datasheet for timings values, parameters TS_vrefint, TS_temp (values rough order: 4us min). */
	.Offset = 0,
};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc_hndl){
	return;
}

stm_err_t conf_pot_gpi(ADC_HandleTypeDef *adc_hndl){
	HAL_StatusTypeDef ret = HAL_OK;
	static const ADC_InitTypeDef adc_init_params = {
		.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2,
		.Resolution = ADC_RESOLUTION_12B,
		.DataAlign = ADC_DATAALIGN_RIGHT,
		.ScanConvMode = DISABLE,
		.EOCSelection = ADC_EOC_SINGLE_CONV, 
		.ContinuousConvMode = DISABLE,
		.NbrOfConversion = 1,
		.DiscontinuousConvMode = DISABLE,
		.NbrOfDiscConversion = 1,		// If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
		.ExternalTrigConv = ADC_SOFTWARE_START, 
		.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE,
		.DMAContinuousRequests = DISABLE,
	};

	// Configuring IOs and ADC channels
	static GPIO_InitTypeDef adc_gpi_conf = {
		.Pin = GPIO_PIN_0,
		.Mode = GPIO_MODE_ANALOG,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_MEDIUM,
	};
	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_Init(GPIOC, &adc_gpi_conf);		// Conf Port and Pin for GPIO PC0
	adc_gpi_conf.Pin = GPIO_PIN_3;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &adc_gpi_conf);		// Conf Port and Pin for GPIO PA3
	// The ADC Channels for PA3 is ADC1/3 and PC3 is ADC1/10
	__HAL_RCC_ADC1_CLK_ENABLE();
	// Fill handle params
	adc_hndl->Instance =  ADC1;
	memcpy(&adc_hndl->Init, &adc_init_params, sizeof(ADC_InitTypeDef));		//Copiamos byte a byte la configuracion standard
	ret = HAL_ADC_Init(adc_hndl);
	STM_RETURN_ON_ERROR(ret, __func__, "Inicializacion de ADC ha fallado");

	// Conf GPI PA3 Ch3
	ret = HAL_ADC_ConfigChannel(adc_hndl, &sadc_ch3);
	STM_RETURN_ON_ERROR(ret, __func__, "Inicializacion de ADC Channel 3 ha fallado");
	// Conf GPI PC0	Ch10
	//ret = HAL_ADC_ConfigChannel(adc_hndl, &sadc_ch10);
	//STM_RETURN_ON_ERROR(ret, __func__, "Inicializacion de ADC Channel 10 ha fallado");
	//if(ret == HAL_OK)
	//	return STM_OK;
	return (stm_err_t )ret;
}
void pot_read_ADC_and_conf_next_chnl(ADC_HandleTypeDef *adc_hndl, uint16_t *value){
	const uint8_t max_adc_chnl = 2;
	static uint8_t selected_chnl = 0;
	//selected_chnl = selected_chnl % max_adc_chnl;
	HAL_StatusTypeDef hal_status = HAL_OK;
	ADC_ChannelConfTypeDef *sadc_ch[2] = {&sadc_ch3, &sadc_ch10};
	*(value + selected_chnl) = HAL_ADC_GetValue(adc_hndl);
	selected_chnl = (selected_chnl + 1) % max_adc_chnl;
	ADC_ChannelConfTypeDef *sadc_to_smpl = sadc_ch[selected_chnl];
	hal_status = HAL_ADC_ConfigChannel(adc_hndl, sadc_to_smpl);
	if(hal_status != HAL_OK)
		return;
	//selected_chnl++;
}
void pot_thread(void *args){
	stm_err_t ret = STM_OK;
	osMessageQueueId_t *queueId = (osMessageQueueId_t *)args;
	static ADC_HandleTypeDef *adc_hndl = NULL;
	pot_msg_t pot_msg = {0};
	STM_GOTO_ON_FALSE (adc_hndl == NULL, STM_ERR_INVALID_STATE, thread_cleanup, __func__, "Thread already started?");
	adc_hndl = (ADC_HandleTypeDef *)calloc(1, sizeof(ADC_HandleTypeDef));
	STM_GOTO_ON_FALSE (adc_hndl != NULL, STM_ERR_NO_MEM, thread_cleanup, __func__, "Mem allocation failed");
	ret = conf_pot_gpi(adc_hndl);
	STM_GOTO_ON_ERROR(ret, thread_cleanup, __func__, "GPI & ADC init failed"); 
	while(1){
		for(uint8_t active_chnl = 0; active_chnl < 2; active_chnl++){
			HAL_ADC_Start(adc_hndl);
			while(HAL_ADC_PollForConversion(adc_hndl, 100) != HAL_OK);
			pot_read_ADC_and_conf_next_chnl(adc_hndl, (uint16_t *)&pot_msg.pot);
		}
		osMessageQueuePut(queueId, &pot_msg, NULL, 250);
		osDelay(500);
	}
	thread_cleanup:
	if(adc_hndl != NULL)
		free(adc_hndl);
	osThreadExit();
}

stm_err_t init_pot_proc(osMessageQueueId_t *queueId){
	STM_RETURN_ON_FALSE(queueId != NULL, STM_ERR_INVALID_ARG, __func__, "queueID can't be NULL");
	osThreadId_t pot_th_id = osThreadNew(pot_thread, queueId, NULL);
	if(pot_th_id == NULL)
		return STM_FAIL;
	return STM_OK;
	
		
}
