#include "lcd.h"
#include "Arial12x12.h"
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include <stdio.h>

#define FLAG_CALLBACK_SPI 0x01

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef tim7;

static osThreadId_t *id_task_internal = NULL;

static void lcd_reset(void);
static void lcd_wr_cmd(unsigned char cmd);
static void lcd_wr_data(unsigned char data);


static void delay(uint32_t n_microsegundos);
static void delay(uint32_t n_microsegundos){
	__HAL_RCC_TIM7_CLK_ENABLE();
	tim7.Instance = TIM7;
	tim7.Init.Prescaler = 83;
	tim7.Init.Period = n_microsegundos - 1;

	HAL_TIM_Base_Init(&tim7);
	HAL_TIM_Base_Start(&tim7);
	
	while(!__HAL_TIM_GET_FLAG(&tim7, TIM_FLAG_UPDATE)){} //Bloqueante
	__HAL_TIM_CLEAR_FLAG(&tim7, TIM_FLAG_UPDATE);
		
	HAL_TIM_Base_Stop(&tim7);
	HAL_TIM_Base_DeInit(&tim7);
		
	__HAL_RCC_TIM7_CLK_DISABLE();
}

static void callback_spi(uint32_t event){
	osThreadFlagsSet(*id_task_internal, FLAG_CALLBACK_SPI);
}

static void lcd_reset(void){
	static GPIO_InitTypeDef GPIO_InitStruct;
	/*CS*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	
	/*A0*/
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	
	/*Reset*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	
	/*SPI*/
	SPIdrv->Initialize(callback_spi);
	SPIdrv-> PowerControl(ARM_POWER_FULL);
	SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	delay(1000);
}



static void lcd_wr_cmd(unsigned char cmd){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	SPIdrv->Send(&cmd, sizeof(cmd));
	osThreadFlagsWait(FLAG_CALLBACK_SPI, osFlagsWaitAll, osWaitForever);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}
static void lcd_wr_data(unsigned char data){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	SPIdrv->Send(&data, sizeof(data));
	osThreadFlagsWait(FLAG_CALLBACK_SPI, osFlagsWaitAll, osWaitForever);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void lcd_init(osThreadId_t *id_task){
	id_task_internal = id_task;
	lcd_reset();
	lcd_wr_cmd(0xAE);//display off
	lcd_wr_cmd(0xA2);//Fija el valor de la tensión de polarización del LCD a 1/9
	lcd_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
	lcd_wr_cmd(0xC8);//El scan en las salidas COM es el normal
	lcd_wr_cmd(0x22);//Fija la relación de resistencias interna a 2
	lcd_wr_cmd(0x2F);//Power on
	lcd_wr_cmd(0x40);//Display empieza en la línea 0
	lcd_wr_cmd(0xAF);//Display ON
	lcd_wr_cmd(0x81);//Contraste
	lcd_wr_cmd(0x17);//Valor de contraste
	lcd_wr_cmd(0xA4);//Display all points normal
	lcd_wr_cmd(0xA6);//LCD Display Normal
	
	lcd_clean();
}

static void lcd_set_line(uint8_t line){
	lcd_wr_cmd(0x00);
	lcd_wr_cmd(0x10);
	lcd_wr_cmd(0xB0 | (line & 0x3));
}

void lcd_write_drawbuf(uint8_t *drawbuf){
	const int max_column = 128;
	int line = 0, column = 0;
	for(line = 0; line < 4; line++){
		lcd_set_line(line);
		for(column = line * max_column; column < (max_column * (1 + line)); column++){
			lcd_wr_data(drawbuf[column]);
		}
	}
}
void lcd_clean(void){
	const int max_column = 128;
	int line = 0, column = 0;
	for(line = 0; line < 4; line++){
		lcd_set_line(line);
		for(column = line * max_column; column < (max_column * (1 + line)); column++){
			lcd_wr_data(0);
		}
	}
}
uint16_t lcd_poner_caracter(uint8_t * drawbuf, char caracter, uint8_t linea, uint16_t x){
	uint8_t *pos_up = &drawbuf[(linea & 0x1)*256 + x];
	uint8_t *pos_down = &drawbuf[(linea & 0x1)*256 + x + 128];
	uint16_t offset = 25 * (caracter - ' ');
	for(int i = 0; i < 12; i++){
		pos_up[i] = Arial12x12[offset + i * 2 + 1];
		pos_down[i] = Arial12x12[offset + i * 2 + 2];
	}
	return x + Arial12x12[offset];
}

	
/*
static void symbolToLocalBuffer_L1(uint8_t symbol){
	static uint8_t i, value1, value2;
	static uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for(i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i + positionL1] = value1;
		buffer[i + 128 + positionL1] = value2;
	}
	positionL1 = positionL1 + Arial12x12[offset];
}

static void symbolToLocalBuffer_L2(uint8_t symbol){
	static uint8_t i, value1, value2;
	static uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for( i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i + 256 + positionL2] = value1;
		buffer[i + 384 + positionL2] = value2;
	}
	positionL2 = positionL2 + Arial12x12[offset];
}
*/