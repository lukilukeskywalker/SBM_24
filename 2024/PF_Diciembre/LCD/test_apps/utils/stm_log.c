#include "stm_log.h"
#include <stdarg.h>
#include "stm32f4xx_hal.h"
//#include 
struct __FILE {int handle;/* Add whatever you need here */};

FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
  ITM_SendChar(ch);
  return(ch);
}

void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz, uint32_t SWOSpeed){
	uint32_t SWOPrescaler = (cpuCoreFreqHz / SWOSpeed) - 1;				//SWOSpeed = 64000, 
	
	CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk; //Enable Trace Debug
	*((volatile unsigned *)(ITM_BASE + 0x400F0)) = 0x00000002;
	*((volatile unsigned *)(ITM_BASE + 0x40010)) = SWOPrescaler;
	*((volatile unsigned *)(ITM_BASE + 0x00FB0)) = 0xC5ACCE55;
	
	ITM->TCR = ITM_TCR_TraceBusID_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk,
	ITM->TPR = ITM_TPR_PRIVMASK_Msk;
	ITM->TER = portBits;
	
	*((volatile unsigned *)(ITM_BASE + 0x01000)) = 0x4000;
	*((volatile unsigned *)(ITM_BASE + 0x40304)) = 0x0000;
	
}

void stm_log_write(stm_log_level_t level, const char *tag, const char* format, ...){
	const char *level_str;
	switch(level){
		case ERROR_LVL:
			level_str = "\033[31mERROR: ";
			break;
		case WARNING_LVL:
			level_str = "\033[33mWARNING: ";
			break;
		case INFO_LVL:
			level_str = "\033[32mINFO: ";
			break;
		case DEBUG_LVL:
			level_str = "\033[34mDEBUG: ";
			break;
		case VERBOSE_LVL:
		default:
			level_str = "\033[0mVERBOSE: ";
			break;
	}
	printf("%s %s ", level_str, tag);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\033[0m\r\n");
		
}

void SWO_PrintChar(char c, uint8_t portNo){
	volatile int timeout;
	if((ITM->TCR&ITM_TCR_ITMENA_Msk) == 0)
		return;
	if((ITM->TER & (1ul << portNo)) == 0)
		return;
	timeout = 5000;
	while(ITM->PORT[0].u32 ==  0){
		timeout--;
		if(timeout == 0)
			return;
	}
	ITM->PORT[0].u16 = 0x08 | (c<<8);
	return;
}
void SWO_PrintString(const char *s, uint8_t portNumber){
	while(*s!= '\0')
		SWO_PrintChar(*s++, portNumber);
}
