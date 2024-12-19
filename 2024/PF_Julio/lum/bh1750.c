#include "bh1750.h"



typedef enum{
	bh_pwr_down = 0x00,
	bh_pwr_up = 0x01,
	bh_reset = 0x07,
	bh_Hres1_mode = 0x10,
	bh_Hres2_mode = 0x11,
	bh_Lres_mode = 0x13,
	bh_oneshot_Hres1_mode = 0x20,
	bh_oneshot_Hres2_mode = 0x21,
	bh_oneshot_Lres_mode = 0x23,
	bh_MSb_Measure_time = 0x40,
	bh_LSb_Measure_time = 0x60,
	
}bh1750_opcodes_t;

bh1750_err_t init_bh1750(const bh1750_t *bh){
	static uint8_t pwr_up_cmnd = bh_pwr_up;
	return bh->write(bh->handle, &pwr_up_cmnd, 1);
}
bh1750_err_t set_mode_bh1750(const bh1750_t *bh, bh1750_mode_t mode){
	uint8_t op_mode = 0;
	switch(mode){
		case CONTINUOUS_HIGH_RES_MODE1:
			op_mode = bh_Hres1_mode;
			break;
		case CONTINUOUS_HIGH_RES_MODE2:
			op_mode = bh_Hres2_mode;
			break;
		case CONTINUOUS_LOW_RES_MODE:
			op_mode = bh_Lres_mode;
			break;
		case ONESHOT_HIGH_RES_MODE1:
			op_mode = bh_oneshot_Hres1_mode;
			break;
		case ONESHOT_HIGH_RES_MODE2:
			op_mode = bh_oneshot_Hres2_mode;
			break;
		case ONESHOT_LOW_RES_MODE:
			op_mode = bh_oneshot_Lres_mode;
		default:
			return bh1750_fail;
	}
	return bh->write(bh->handle, &op_mode, 1);
}

bh1750_err_t set_mea_tm_bh1750(const bh1750_t *bh, uint8_t measure_tm){
	uint32_t ret = 0;
	uint8_t val = 0;
	if(measure_tm > BH1750_MEA_TM_MAX || measure_tm < BH1750_MEA_TM_MIN)
		return bh1750_fail;
	val = (0x40 | (measure_tm >> 5));
	ret = bh->write(bh->handle, &val, 1);
	if(ret != 0)
		return ret;
	val = (0x60 | (measure_tm & 0x1f));
	ret = bh->write(bh->handle, &val, 1);
	return (bh1750_err_t )ret;
}

bh1750_err_t read_luminance_bh1750(const bh1750_t *bh, uint16_t *luminance){
	uint32_t ret;
	uint8_t lum_arr[2];
	ret = bh->read(bh->handle, lum_arr, 2);
	*luminance = (lum_arr[0] << 8) | lum_arr[1];
	return ret;
}

