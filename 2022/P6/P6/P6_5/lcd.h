#ifndef __LCD_H
#define __LCD_H

/*Includes*/
#include "Driver_SPI.h"  
#include "main.h"
#include "stdio.h"
#include "string.h"

/*Funciones*/
void delay (uint32_t n_microsegundos);
void PIN_LCD(void);
void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init(void);
void LCD_update(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void LCD_Clean(void);
void symnolToLocalBuffer(uint8_t line,uint8_t symbol);
void Pintar_Arriba(void);
void Pintar_Abajo(void);
void Pintar_Centro(void);
void Pintar_Derecha(void);
void Pintar_Izquierda(void);

#endif 
