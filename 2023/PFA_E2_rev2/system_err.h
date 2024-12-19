/**
* @author Lukas Gdanietz
*	@note Common error declaration list and MACRO functions
*
*/

#pragma once

typedef enum{
	sys_ok = 0,
	sys_fail = -1,
	sys_err_no_mem = 0x101,
	sys_err_invalid_arg = 0x102,
	sys_err_invalid_state = 0x103,
	sys_err_invalid_size = 0x104,
	sys_err_not_found = 0x105,
	sys_err_not_supported = 0x106,
	sys_err_timeout = 0x107
}system_err_t;

#define SYS_GOTO_ON_FALSE(condition, err_code, goto_location) \
    do { \
        if (!(condition)) { \
            ret = err_code; \
            goto goto_location; \
        } \
    } while (0)

#define SYS_RETURN_ON_FALSE(condition, err_code) \
    do { \
        if (!(condition)) { \
            return err_code; \
        } \
    } while (0)
		
		
