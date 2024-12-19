#ifndef __STM_ERR_H__
#define __STM_ERR_H__
#include <stdio.h>
#include "stm_log.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef int stm_err_t;

//#define STM_OK          0       /*!< STM_err_t value indicating success (no error) */
//#define STM_FAIL        -1      /*!< Generic STM_err_t code indicating failure */

//#define STM_ERR_NO_MEM              0x101   /*!< Out of memory */
//#define STM_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
//#define STM_ERR_INVALID_STATE       0x103   /*!< Invalid state */
//#define STM_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
//#define STM_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
//#define STM_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
//#define STM_ERR_TIMEOUT             0x107   /*!< Operation timed out */
//#define STM_ERR_INVALID_RSTMONSE    0x108   /*!< Received rSTMonse was invalid */
//#define STM_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
//#define STM_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
//#define STM_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
//#define STM_ERR_NOT_FINISHED        0x10C   /*!< Operation has not fully completed */
//#define STM_ERR_NOT_ALLOWED         0x10D   /*!< Operation is not allowed */

typedef enum{
	STM_OK = 0,
	STM_FAIL = -1,
	STM_ERR_NO_MEM = 0x101,
	STM_ERR_INVALID_ARG = 0x102,
	STM_ERR_INVALID_STATE = 0x103,
	STM_ERR_INVALID_SIZE = 0x104,
	STM_ERR_NOT_FOUND = 0x105,
	STM_ERR_NOT_SUPPORTED = 0x106,
	STM_ERR_TIMEOUT = 0x107,
	STM_ERR_INVALID_RSTMONSE = 0x108,
	STM_ERR_INVALID_CRC  = 0x109,
	STM_ERR_INVALID_VERSION = 0x10A,
	STM_ERR_INVALID_MAC = 0x10B,
	STM_ERR_NOT_FINISHED = 0x10C,
	STM_ERR_NOT_ALLOWED = 0x10D
	
	
}stm_err_t;


// STM_RETURN_ON_ERROR: If (expr) evaluates to non-zero, print an error message and return its value
#define STM_RETURN_ON_ERROR(expr, TAG, msg, ...) do {                \
    int _err = (expr);                                          \
    if (_err) {                                                 \
        STM_LOGE(TAG, msg, ##__VA_ARGS__);                \
        return _err;                                            \
    }                                                           \
} while (0)

// STM_GOTO_ON_ERROR: If (expr) evaluates to non-zero, print an error message, set ret to _err, and goto the label
#define STM_GOTO_ON_ERROR(expr, label, TAG, msg, ...) do {           \
    int _err = (expr);                                          \
    if (_err) {                                                 \
        STM_LOGE(TAG, msg, ##__VA_ARGS__);                \
        ret = _err;                                             \
        goto label;                                             \
    }                                                           \
} while (0)

// STM_RETURN_ON_FALSE: If (expr) evaluates to zero (false), print an error message and return the err_code
#define STM_RETURN_ON_FALSE(expr, err_code, TAG, msg, ...) do {		\
	if (!(expr)) {							\
		STM_LOGE(TAG, msg, ##__VA_ARGS__);			\
		return err_code;                                        \
    }                                                           	\
} while (0)

// STM_GOTO_ON_FALSE: If (expr) evaluates to zero (false), print an error message, set ret to err_code, and goto the label
#define STM_GOTO_ON_FALSE(expr, err_code, label, TAG, msg, ...) do { 	\
	if (!(expr)) {                                              	\
		STM_LOGE(TAG, msg, ##__VA_ARGS__);		        \
		ret = err_code;                                         \
		goto label;                                             \
	}                                                           	\
}while(0)





#ifdef __cplusplus
}
#endif

#endif
