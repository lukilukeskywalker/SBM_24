#include "lcd.h"
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

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