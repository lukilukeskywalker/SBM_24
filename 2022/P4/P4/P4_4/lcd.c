#include "lcd.h"

static GPIO_InitTypeDef GPIO_InitStruct;//variable para los pines
unsigned char buffer [512];//array de char sin signo de 512 elementos
ARM_SPI_STATUS stat;// variable STATUS para saber si se ha liverado el buffer
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;//variable del SPI
TIM_HandleTypeDef htim7; //define cual es el manejador timer que vamos a utilizar
uint16_t positionL1=0;
uint16_t positionL2=0;

//////////////////////////////////////////////RESET Y DELAY//////////////////////////////////////////////////
/* Función de retardo*/
void delay (uint32_t n_microsegundos){
	htim7.Instance = TIM7; //nombre de la instancia con la que trabajamos
	
	htim7.Init.Prescaler = 83; //valor del preescaler con el que vamos a trabajar-> 83 para que 84MHz/84=1MHz
	htim7.Init.Period = n_microsegundos-1; //valor del periodo con el que vamos a trabajar: en este caso queremos llegar a 1MHz (T=1us)
												   //por lo tanto 1MHz/n_us=tiempo que quieras
	__HAL_RCC_TIM7_CLK_ENABLE();
	HAL_TIM_Base_Init(&htim7); // configura el timer
	HAL_TIM_Base_Start(&htim7); //start el timer 
	/*Esperar que se active el flag del registro Match correspondiente*/
	
	
	while( TIM7->CNT < n_microsegundos-1){}
	/*paro el reset*/	
	HAL_TIM_Base_Stop(&htim7);
	/*inicializo el timer de nuevo*/
	HAL_TIM_Base_DeInit(&htim7); // configura el timer
	
//	
//	//¿¿USARRRRR TIM_FLAG_UPDATE????
//	if(__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) == 0){}
//		/*Borrar el flag*/
//		__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
//		/*paro el reset*/	
//		HAL_TIM_Base_Stop(&htim7);
//		/*inicializo el timer de nuevo*/
//		HAL_TIM_Base_DeInit(&htim7); // configura el timer	
}


void PIN_Init(){
/*pin CS*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //modo push pull de salida
	GPIO_InitStruct.Pull = GPIO_PULLUP; //activacion a nivel alto
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //modo alta frecuencia
	GPIO_InitStruct.Pin = GPIO_PIN_14; //pin CS
	HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);//pin CS a 1
	
	/*pin A0*/
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //modo push pull de salida
	GPIO_InitStruct.Pull = GPIO_PULLUP; //activacion a nivel alto
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //modo alta frecuencia
	GPIO_InitStruct.Pin = GPIO_PIN_13; //pin A0
	HAL_GPIO_Init(GPIOF,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);//pin A0 a 1
	
	/*pin reset*/
	__HAL_RCC_GPIOA_CLK_ENABLE(); //activo el reloj del pin de los reset
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //modo push pull de salida
	GPIO_InitStruct.Pull = GPIO_PULLUP; //activacion a nivel alto
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //modo alta frecuencia
	GPIO_InitStruct.Pin = GPIO_PIN_6; //pin Reset
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);//pin reset a 1
	
}

	
	
/*Controlador SPI1*/


void LCD_reset(){
	/* Inicializar el controlador SPI */
	SPIdrv->Initialize ( NULL );// Porque no devuelve nada: de momento no se usa el callback
	/* Encienda el periférico SPI */
  SPIdrv-> PowerControl ( ARM_POWER_FULL );
  /* Configure el SPI como maestro, modo de 8 bits a 20MBits / seg */
  SPIdrv-> Control ( ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);

	
	/////pulso de reset de 1us/////
	delay(1);//para respetar los tiempos para ver la señal en el analizador
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);//ESCRIBE EL PIN  1 O 0 COMO ABAJO PARA ACTIVAR Y DESACTIVAR EL RESET
	delay(5);//para 1us-> 5-1/2
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	
	
	/////retardo de 1ms///
	delay(1001);
}




///////////////////////////////////////////FUNCIONES DATA Y CMD//////////////////////////////////////////////////////

/*Función que escribe un dato en el LCD*/
void LCD_wr_data(unsigned char data){
	//// Seleccionar CS = 0 ////
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	//// Seleccionar A0 = 1 ////
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	////Escribir un dato usando SPIDrv... ////
	SPIdrv->Send(&data, sizeof(data));
	////Esperar a que se libere el bus del SPI ////
	do
	{
		stat=SPIdrv->GetStatus();
	}
	while (stat.busy);
	////Seleccionar CS = 1 ////
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}


/*Función que escribe un comando en el LCD*/
void LCD_wr_cmd(unsigned char cmd){
	//// Seleccionar CS = 0 ////
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	//// Seleccionar A0 = 0 ////
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	////Escribir un comando (cmd) usando SPIDrv... ////
	SPIdrv->Send(&cmd, sizeof(cmd));
	////Esperar a que se libere el bus del SPI ////
	do
	{
		stat=SPIdrv->GetStatus();
	}
	while (stat.busy);
	////Seleccionar CS = 1 ////
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}


//////////////////////////////////////////////INIT DEL LCD/////////////////////////////////////////////////////
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


////////////////////////////////////////////////UPDATE Y DIBUJOS///////////////////////////////////////////////////////////
void Pintar_Cuadrado(){
	/*PINTAR CUADRADO*/
	buffer[0]=0xFF;
	buffer[1]=0x81;
	buffer[2]=0x81;
	buffer[3]=0x81;
	buffer[4]=0x81;
	buffer[5]=0x81;
	buffer[6]=0x81;
	buffer[7]=0xFF;
}
void Pintar_A(){
		/*PINTAR A*/
	buffer[10]=0xFE;
	buffer[11]=0x12;
	buffer[12]=0x12;
	buffer[13]=0x11;
	buffer[14]=0x11;
	buffer[15]=0x12;
	buffer[16]=0x12;
	buffer[17]=0xFE;
}
void Pintar_A1(){
		/*PINTAR A*/
	buffer[0]=0x00;
	buffer[1]=0xFC;
	buffer[2]=0x12;
	buffer[3]=0x11;
	buffer[4]=0x11;
	buffer[5]=0x12;
	buffer[6]=0xFC;
	buffer[7]=0x00;
}


void LCD_update(void)
{
	int i;	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	
	for(i=0;i<128;i++){
		LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	
	for(i=128;i<256;i++){
		LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	
	for(i=256;i<384;i++){
		LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3

	for(i=384;i<512;i++){
		LCD_wr_data(buffer[i]);
	}
}


///////////////////////////FUNCION QUE ESCRIBE EN LA LINEA 1/////////////////////////////////
void symbolToLocalBuffer_L1(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset=0;
		
	offset=25*(symbol - ' ');
		
	for(i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];//coge datos alternos-> uno si,uno no, empezando por el 1 en el valor 1 y el dos en el valor 2
		value2=Arial12x12[offset+i*2+2];
			
		buffer[i+positionL1]=value1;//escribe en la página 0
		buffer[i+128+positionL1]=value2;//escribe en la página 1

	}
	positionL1=positionL1+Arial12x12[offset];//va guardando las posiciones finales de las letras para que continue por ahí
}

///////////////////////////FUNCION QUE ESCRIBE EN LA LINEA 2//////////////////////////////////////
void symbolToLocalBuffer_L2(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset=0;
		
	offset=25*(symbol - ' ');
		
	for(i=0; i<12; i++){
		
		value1=Arial12x12[offset+i*2+1];//coge datos alternos-> uno si,uno no, empezando por el 1 en el valor 1 y el dos en el valor 2
		value2=Arial12x12[offset+i*2+2];
			
		buffer[i+256+positionL2]=value1;//escribe en la página 0
		buffer[i+384+positionL2]=value2;//escribe en la página 1

	}
	positionL2=positionL2+Arial12x12[offset];//va guardando las posiciones finales de las letras para que continue por ahí
}

//////////////////////////////////////////FUNCION PARA ESCRIBIR AMBAS LINEAS//////////////////////////////////////
void symnolToLocalBuffer(uint8_t line,uint8_t symbol){
	
	if (line==1){
		symbolToLocalBuffer_L1(symbol);
	}
	if(line==2){
		symbolToLocalBuffer_L2(symbol);
	}
	
}
