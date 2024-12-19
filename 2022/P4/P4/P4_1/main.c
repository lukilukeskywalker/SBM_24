/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F4xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI
#include "Arial12x12.h"
#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
	
	///////FUNCIONES////	
void PIN_Init(void);
void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init(void);
void LCD_update(void);
void delay (uint32_t n_microsegundos);
void Pintar_A1(void);
void Pintar_A(void);
void Pintar_Cuadrado(void);
void symbolToLocalBuffer_L1(uint8_t symbol);

//////////VARIABLES//////////
static GPIO_InitTypeDef GPIO_InitStruct;//variable para los pines
unsigned char buffer [512];//array de char sin signo de 512 elementos
ARM_SPI_STATUS stat;// variable STATUS para saber si se ha liverado el buffer
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;//variable del SPI
TIM_HandleTypeDef htim7; //define cual es el manejador timer que vamos a utilizar
uint16_t positionL1=0;
	
int main(void)
{

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	PIN_Init();//Configuracion de los pines reset, CS y A0
	LCD_reset(); 
	LCD_init();
	symbolToLocalBuffer_L1('H');
	LCD_update();
#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */

  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



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


//////////////////////////////////EJEMPLO 1///////////////////////////////////////////
void symbolToLocalBuffer_L1(uint8_t symbol){
	
uint8_t i, value1, value2;
uint16_t offset=0;
	
offset=25*(symbol - ' ');
	
for(i=0; i<12; i++){
	
value1=Arial12x12[offset+i*2+1];//coge datos alternos-> uno si,uno no, empezando por el 1 en el valor 1 y el dos en el valor 2
value2=Arial12x12[offset+i*2+2];
	
buffer[i/*+positionL1*/]=value1;//escribe en la página 0
buffer[i+128/*+positionL1*/]=value2;//escribe en la página 1

}
//positionL1=positionL1+Arial12x12[offset];
}
