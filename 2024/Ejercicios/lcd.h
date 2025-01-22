#ifndef __LCD_H__
#define __LCD_H__
#include "Driver_SPI.h"  
#include "stm32f4xx_hal.h"

typedef struct{
	uint8_t x_pos, y_pos;
	uint8_t x_size, y_size;
	uint8_t *buf;
}custom_symbol_t;
	

void LCD_IO_Init(void);
void LCD_reset(void);
void LCD_init(void);
void LCD_update(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToPosition(custom_symbol_t *symb);
void clearbuffer(void);
uint8_t* getSymbolPointer(uint8_t symbol);
void copySymbolToBuffer(uint8_t symbol, custom_symbol_t *dest);


#endif
