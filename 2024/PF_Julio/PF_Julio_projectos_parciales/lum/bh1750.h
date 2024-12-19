#ifndef __BH1750_H__
#define __BH1750_H__
#include <stdio.h>
#include <stdint.h>

#define BH1750_ADDR0 0x5b
#define BH1750_ADDR1 0x23

#define BH1750_MEA_TM_MIN 31
#define BH1750_MEA_TM_MAX 254

typedef struct{
    int32_t (* read)(void *handle, uint8_t *in_data, size_t in_size);
    int32_t (* write)(void *handle, const uint8_t *out_data, size_t out_size);
    void *handle;
}bh1750_t;

typedef enum{
	CONTINUOUS_HIGH_RES_MODE1,
	CONTINUOUS_HIGH_RES_MODE2,
	CONTINUOUS_LOW_RES_MODE,
	ONESHOT_HIGH_RES_MODE1,
	ONESHOT_HIGH_RES_MODE2,
	ONESHOT_LOW_RES_MODE,
	
}bh1750_mode_t;

typedef enum{
	bh1750_ok = 0,
	bh1750_fail = -1,
}bh1750_err_t;

bh1750_err_t init_bh1750(const bh1750_t *bh);
bh1750_err_t set_mode_bh1750(const bh1750_t *bh, bh1750_mode_t mode);
bh1750_err_t set_mea_tm_bh1750(const bh1750_t *bh, uint8_t measure_tm);
bh1750_err_t read_luminance_bh1750(const bh1750_t *bh, uint16_t *luminance);

#endif
