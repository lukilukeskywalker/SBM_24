#include "stm_log.h"
#include <stdio.h>
#include <stdarg.h>
#include "stm32f4xx_hal.h"
//#include 

static volatile uint32_t stm_log_flags = 0;

struct __FILE {int handle;/* Add whatever you need here */};

FILE __stdout;
FILE __stdin;


int fputc(int ch, FILE *f){
     if(stm_log_flags == 0)
          return (ch);
     ITM_SendChar(ch);
     return(ch);
}
int _write(int le, char *ptr, int len)
    {
    int DataIdx;
    for(DataIdx = 0; DataIdx < len; DataIdx++)
        {
        ITM_SendChar(*ptr++);
        }
    return len;
    }
void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz, uint32_t SWOSpeed){
     volatile uint32_t *dhcsr = (uint32_t*)0xE000EDF0;
     if ((*dhcsr & 0x1) == 0) 
          return;                                                               // System not being debugged, SWO can't work
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

	stm_log_flags = 1;                                                         // SWO debugging enabled.
}

void stm_log_write(stm_log_level_t level, const char *tag, const char* format, ...){
	const char *level_str;
     if(stm_log_flags == 0)
          return;
	switch(level){
		case ERROR_LVL:
			level_str = "\033[31mE:";
			break;
		case WARNING_LVL:
			level_str = "\033[33mW:";
			break;
		case INFO_LVL:
			level_str = "\033[32mI:";
			break;
		case DEBUG_LVL:
			level_str = "\033[34mD:";
			break;
		case VERBOSE_LVL:
		default:
			level_str = "\033[0mV:";
			break;
	}
	printf("%s %s: ", level_str, tag);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\033[0m\r\n");
		
}

void SWO_PrintChar(char c, uint8_t portNo){
	volatile int timeout;
     if(stm_log_flags == 0)
          return;
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
     if(stm_log_flags == 0)
          return;
	while(*s!= '\0')
		SWO_PrintChar(*s++, portNumber);
}
