#include "lcd.h"
#include "Arial12x12.h"

osThreadId_t tid_Th_lcd;
osMessageQueueId_t mid_MsgQueue_lcd;
MSGQUEUE_OBJ_LCD msg_lcd;

GPIO_InitTypeDef GPIO_InitStruct_LCD;
unsigned char buffer [512];
ARM_SPI_STATUS stat;
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef htim7;
uint16_t positionL1 = 0;
uint16_t positionL2 = 0;

void Th_lcd(void *argument);
//FUNCIONES ABIERTAS PARA PRINCIPAL//
void ASCII_write_lcd(){
	msg_lcd.init_L1 = 30;
	msg_lcd.init_L2 = 15;
	
	osMessageQueuePut(mid_MsgQueue_lcd, &msg_lcd, NULL, 0U);
}


void delay (uint32_t n_microsegundos){
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 83; //83 para que 84MHz/84=1MHz
	htim7.Init.Period = n_microsegundos-1;
	__HAL_RCC_TIM7_CLK_ENABLE();
	HAL_TIM_Base_Init(&htim7);
	HAL_TIM_Base_Start(&htim7);
	while( TIM7->CNT < n_microsegundos-1){}
	HAL_TIM_Base_Stop(&htim7);
	HAL_TIM_Base_DeInit(&htim7);
}

void LCD_reset(){
	/*CS*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOD,&GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	
	/*A0*/
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOF,&GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	
	/*Reset*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_6;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	
	/*SPI*/
	SPIdrv->Initialize ( NULL );
  SPIdrv-> PowerControl ( ARM_POWER_FULL );
  SPIdrv-> Control ( ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
	delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	delay(5);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	delay(1001);
}

void LCD_wr_data(unsigned char data){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	SPIdrv->Send(&data, sizeof(data));
	do{
		stat=SPIdrv->GetStatus();
	}while (stat.busy);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char cmd){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	SPIdrv->Send(&cmd, sizeof(cmd));
	do{
		stat=SPIdrv->GetStatus();
	}	while (stat.busy);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_init(){
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
}

void LCD_update(){
	int i;	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i=0;i<128;i++){LCD_wr_data(buffer[i]);}

	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i=128;i<256;i++){LCD_wr_data(buffer[i]);}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	for(i=256;i<384;i++){LCD_wr_data(buffer[i]);}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3
	for(i=384;i<512;i++){LCD_wr_data(buffer[i]);}
}

void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for(i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i + positionL1] = value1;
		buffer[i + 128 + positionL1] = value2;
	}
	positionL1 = positionL1 + Arial12x12[offset];
	LCD_update();
}

void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for( i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i + 256 + positionL2] = value1;
		buffer[i + 384 + positionL2] = value2;
	}
	positionL2 = positionL2 + Arial12x12[offset];
	LCD_update();
}

void symbolToLocalBuffer(int lane, uint8_t symbol){
	if(lane == 1)
		symbolToLocalBuffer_L1(symbol);
	if(lane == 2)
		symbolToLocalBuffer_L2(symbol);
}

void clean_LCD(void){
	for(int i=0;i<512;i++){
		buffer[i]=0x00;
	}
}
	
void update_data(MSGQUEUE_OBJ_LCD msg){
	int i, j;
	positionL1 = msg.init_L1;
	positionL2 = msg.init_L2;
	for(i = 0; i < positionL1; i++){
		buffer[i] = 0x00;
		buffer[i+128] = 0x00;
	}	
	for(i = 0; i < strlen(msg.data_L1); i++){
		symbolToLocalBuffer_L1(msg.data_L1[i]);
	}
	for(i = positionL1; i < 128; i++){
		buffer[i] = 0x00;
		buffer[i+128] = 0x00;
	}
	LCD_update();
	
	
	for(j = 0; j < positionL2; j++){
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
	}	
	for(j = 0; j < strlen(msg.data_L2); j++){
		symbolToLocalBuffer_L2(msg.data_L2[j]);
	}
	for(j = positionL2; j < 128; j++){
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
	}
	LCD_update();
}

int Init_MsgQueue_lcd (void) {
  mid_MsgQueue_lcd = osMessageQueueNew(MSGQUEUE_OBJECTS_LCD, sizeof(MSGQUEUE_OBJ_LCD), NULL);
  if (mid_MsgQueue_lcd == NULL) 
		return(-1);
  return(0);
}

int Init_Th_lcd (void) {
  tid_Th_lcd = osThreadNew(Th_lcd, NULL, NULL);
  if(tid_Th_lcd == NULL)
    return(-1);
	LCD_reset();
	LCD_init();
	clean_LCD();
	Init_MsgQueue_lcd();
  return(0);
}

void Th_lcd (void *argument) {
	osStatus_t status;
  while(1) {
		status = osMessageQueueGet(mid_MsgQueue_lcd, &msg_lcd, NULL, 0U);
		if(status == osOK){
			update_data(msg_lcd);
		}
		osThreadYield();
  }
}




