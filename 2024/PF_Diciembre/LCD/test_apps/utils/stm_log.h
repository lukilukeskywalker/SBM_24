#ifndef __STM_LOG_H__
#define __STM_LOG_H__
#include "stdio.h"
#include "stdint.h"


typedef enum{
	ERROR_LVL = 0,
	WARNING_LVL = 1,
	INFO_LVL = 2,
	DEBUG_LVL = 3,
	VERBOSE_LVL = 4,
}stm_log_level_t;

//#define ERROR_LVL 0
//#define WARNING_LVL 1
//#define INFO_LVL 2
//#define DEBUG_LVL 3
//#define VERBOSE_LVL 4

#ifndef _LOG_LEVEL
#define _LOG_LEVEL ERROR_LVL
#endif
	
	
void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz, uint32_t SWOSpeed);
void stm_log_write(stm_log_level_t level, const char *tag, const char* format, ...);

#if _LOG_LEVEL >= ERROR_LVL
#define STM_LOGE(tag, format, ...) do { \
	stm_log_write(ERROR_LVL, tag, format, ##__VA_ARGS__); \
}while(0)
#else
#define STM_LOGE(tag, format, ...)
#endif

#if _LOG_LEVEL >= WARNING_LVL
#define STM_LOGW(tag, format, ...) do { \
	stm_log_write(WARNING_LVL, tag, format, ##__VA_ARGS__); \
}while(0)
#else
#define STM_LOGW(tag, format, ...)
#endif

#if _LOG_LEVEL >= INFO_LVL
#define STM_LOGI(tag, format, ...) do { \
	stm_log_write(INFO_LVL, tag, format, ##__VA_ARGS__); \
}while(0)
#else
#define STM_LOGI(tag, format, ...)
#endif

#if _LOG_LEVEL >= DEBUG_LVL
#define STM_LOGD(tag, format, ...) do { \
	stm_log_write(DEBUG_LVL, tag, format, ##__VA_ARGS__); \
}while(0)
#else
#define STM_LOGD(tag, format, ...)
#endif

#if _LOG_LEVEL >= VERBOSE_LVL
#define STM_LOGV(tag, format, ...) do { \
	stm_log_write(VERBOSE_LVL, tag, format, ##__VA_ARGS__); \
}while(0)
#else
#define STM_LOGV(tag, format, ...)
#endif

#endif

