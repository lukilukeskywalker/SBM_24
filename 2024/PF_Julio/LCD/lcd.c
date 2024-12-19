#include "lcd.h"
#include "Arial12x12.h"
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include <stdio.h>

#define FLAG_CALLBACK_SPI 0x01

static osThreadId_t id_Th_Test_Th_lcd;
static osThreadId_t id_Th_lcd;
static osMessageQueueId_t id_MsgQueue_lcd;
static MSGQUEUE_OBJ_LCD msg;

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef tim7;

unsigned char buffer[512] = {0x00};
static uint16_t positionL1, positionL2 = 0;

static void Th_lcd(void *argument);
static void Test_Th_lcd(void *arguments);

static void update_data(MSGQUEUE_OBJ_LCD msg);
static void LCD_init(void);
static void LCD_reset(void);
static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);
static void LCD_update(void);
static void delay(uint32_t n_microsegundos);
static void symbolToLocalBuffer_L1(uint8_t symbol);
static void symbolToLocalBuffer_L2(uint8_t symbol);

osMessageQueueId_t get_id_MsgQueue_lcd(void){
	return id_MsgQueue_lcd;
}

static int Init_MsgQueue_lcd(void){
  id_MsgQueue_lcd = osMessageQueueNew(MSGQUEUE_OBJECTS_LCD, sizeof(MSGQUEUE_OBJ_LCD), NULL);
  if(id_MsgQueue_lcd == NULL)
    return (-1); 
  return(0);
}

int Init_Th_lcd(void){
  id_Th_lcd = osThreadNew(Th_lcd, NULL, NULL);
  if(id_Th_lcd == NULL)
    return(-1);
  return(Init_MsgQueue_lcd());
}

static void Th_lcd(void *argument) {
	LCD_init();
	while(1){
		if(osMessageQueueGet(id_MsgQueue_lcd, &msg, NULL, osWaitForever) == osOK){
			update_data(msg);
		}
	}
}

uint8_t shift_to_msb(uint8_t val){
	// Esta funcion es un poco hack, lo unico que tengo que hacer es comprobar si 0x80 esta puesto
	if(val == 0)
		return 0;
	while(0x80!=(0x80&val))
		val = val << 1;
	return val;
}

void update_data(MSGQUEUE_OBJ_LCD msg){
	int i, j;
	positionL1 = positionL2 = 0;
	
	for(i = 0; i < strlen(msg.data_L1); i++){
		symbolToLocalBuffer_L1(msg.data_L1[i]);
	}
	for(i = positionL1; i < 128; i++){
		buffer[i] = 0x00;
		buffer[i+128] = 0x00;
	}
	
	for(j = 0; j < strlen(msg.data_L2); j++){
		symbolToLocalBuffer_L2(msg.data_L2[j]);
	}
	for(j = positionL2; j < 128; j++){
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
	}
	if(msg.dial[0] != 0){
		uint8_t bar_shift = (msg.dial[0] * 32) / 100;
		uint32_t bar = ((1 << bar_shift) - 1);	// This way I set the bits
		for(uint8_t width = 0; width < 8; width++){
			buffer[110 + width] = shift_to_msb((bar >> 24) & 0xff);					// Linea 0
			buffer[110 + 128 + width] = shift_to_msb((bar >> 16) & 0xff);					// Linea 1
			buffer[110 + 256 + width] = shift_to_msb((bar >> 8) & 0xff);					// Linea 0
			buffer[110 + 384 + width] = shift_to_msb((bar) & 0xff);					// Linea 1
		}
	}
	if(msg.dial[1] != 0){
		uint8_t bar_shift = (msg.dial[1] * 32) / 100;
		uint32_t bar = ((1 << bar_shift) - 1);	// This way I set the bits
		for(uint8_t width = 0; width < 8; width++){
			buffer[120 + width] = shift_to_msb((bar >> 24) & 0xff);					// Linea 0
			buffer[120 + 128 + width] = shift_to_msb((bar >> 16) & 0xff);					// Linea 1
			buffer[120 + 256 + width] = shift_to_msb((bar >> 8) & 0xff);					// Linea 0
			buffer[120 + 384 + width] = shift_to_msb((bar) & 0xff);					// Linea 1
		}
	}
	LCD_update();
}



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
	osThreadFlagsSet(id_Th_lcd, FLAG_CALLBACK_SPI);
}

static void LCD_reset(void){
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

static void LCD_wr_data(unsigned char data){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	SPIdrv->Send(&data, sizeof(data));
	osThreadFlagsWait(FLAG_CALLBACK_SPI, osFlagsWaitAll, osWaitForever);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_wr_cmd(unsigned char cmd){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	SPIdrv->Send(&cmd, sizeof(cmd));
	osThreadFlagsWait(FLAG_CALLBACK_SPI, osFlagsWaitAll, osWaitForever);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_init(void){
	LCD_reset();
	LCD_wr_cmd(0xAE);//display off
	LCD_wr_cmd(0xA2);//Fija el valor de la tensión de polarización del LCD a 1/9
	LCD_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8);//El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22);//Fija la relación de resistencias interna a 2
	LCD_wr_cmd(0x2F);//Power on
	LCD_wr_cmd(0x40);//Display empieza en la línea 0
	LCD_wr_cmd(0xAF);//Display ON
	LCD_wr_cmd(0x81);//Contraste
	LCD_wr_cmd(0x17);//Valor de contraste
	LCD_wr_cmd(0xA4);//Display all points normal
	LCD_wr_cmd(0xA6);//LCD Display Normal
	
	memset(buffer, 0x00, sizeof(buffer));
	LCD_update();
}

static void LCD_update(void){
	int i;	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB0);
	for(i = 0; i < 128; i++){LCD_wr_data(buffer[i]);}		// Setear primera linea

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB1);
	for(i = 128; i < 256; i++){LCD_wr_data(buffer[i]);}		// Setear segunda linea 

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2);
	for(i = 256; i < 384; i++){LCD_wr_data(buffer[i]);}		// Setear tercera linea
	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3);
	for(i = 384; i < 512; i++){LCD_wr_data(buffer[i]);}		// Setear cuarta linea
}

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

/*
int Init_Th_lcd_test(void){
	id_Th_Test_Th_lcd = osThreadNew(Test_Th_lcd, NULL, NULL);
	if(id_Th_Test_Th_lcd== NULL)
		return(-1);
	return(0);
}

static void Test_Th_lcd(void *arguments){	
	static MSGQUEUE_OBJ_LCD msg_test;
	
	static uint8_t hora = 12;
	static uint8_t min = 27;
	static uint8_t seg = 35;
	
	Init_Th_lcd();

	sprintf(msg_test.data_L1, "     SBM 2024", valor);
	while(1){
		seg++;
		if(seg == 60){
			seg = 0;
			min++;
			if(min == 60){
				min = 0;
				hora++;
				if(hora == 24){
					hora = 0;
				}
			}
		}
		sprintf(msg_test.data_L2, "      %.2u:%.2u:%.2u", hora, min, seg);
		osMessageQueuePut(get_id_MsgQueue_lcd(), &msg_test, NULL, 0U);
		osDelay(1000);
	}
}
*/