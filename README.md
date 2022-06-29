# SBM

## Como crear un nuevo proyecto:
Para crear un proyecto, creamos un proyecto en Project New uvision Project, Seleccionamos el dispositivo, en nuestro caso el
STM32F429ZITx. Una vez creado el proyecto aparecera una pestaña Manage Run, para seleccionar las librerias que queremos utilizar tenemos que seleccionar:   
1. Abriendo CMSIS, CMSIS-CORE   
2. Abriendo Device, Device-Startup   
	- Abriendo STM32CubeMX Framework(API) Seleccionar Classic   
	- Apareceran en naranja dependencias a resolver. Seleccionar en la esquina inferior izquierda "Resolve"   
3. Abriendo STM32Cube HAL, STM32Cube HAL- GPIO  
	- Apareceran en naranja dependencias a resolver. Seleccionar en la esquina inferior izquierda "Resolve"  
Damos Okey  

Se creara un proyecto. Hay que añadirle un main, para tener un sitio donde trabajar.
En source Group, click derecho, y seleccionar "Add New Item to Group 'Source Group 1'
Dentro seleccioanar User Code Template, En component STM32Cube Framework Clasic->'main' module for STM32Cube HAL  

Done!

## Pinout comunes
* LED1: PB0
* LED2: PB7
* LED3: PB14
## Objetivos de las practicas
Practica Numb:
1. Un contador binario con los leds en la placa Nucleo usando Hal_Delay()  
2. 3 Tareas:
	1. Utilizar una interrupcion provocada por el TIMER 7 para alternar (Toggle) el estado de un pin
	2. Capturar con un analizador logico 5 segundos de la linea PB0. Resultado: Se observa 1 señal de 1Hz, con una duraccion de 500 ms de pulso
	3. Utilizar el timer 2 para desarrollar una aplicacion que permita generar una señal configurable de 500Hz, 1000Hz, 2000Hz, y 3000Hz, en el pin PB11. La configuracion se realiza a traves de una interrupcion provocada por el BTN de usuario, que cambia en teoria el preescaler o el period register. (En el segundo caso si se configura el Output Compare Register a modo TOGGLE (TIM_OCMODE_TOGGLE))
3. 4 Tareas:
	1. Inicializacion de SPI y LCD
	>> Se configura el SPI segun los requisitos del LCD 
	>> * Modo Master ARM_SPI_MODE_MASTER
	>> * CPOL1 y CPHA1 ARM_SPI_CPOL1_CPHA1 (Para poner el SPI en modo 3 de operacion (si recuerdo bien))
	>> * Organización de la informacion de MSB a LSB ARM_SPI_MSB_LSB
	>> * Frecuencia de operacion 20 MHz
	>> * 3 pines configurados como RESET, CS y A0
	>> * Se debe generar un pulso de reset de duracion adecuada, y esperar 1ms antes de comenzar con la secuencia de comandos de inicializacion del LCD
	>> * Se debe utilizar una funcion llamada delay que asi mismo utilice el TIMER 7, en modo basico sin interrupciones, esperando hasta que se active el flag que indique final de cuenta.
	>>  
	2. Creacion de funciones de gestion del LCD
	>> Las funciones creadas son:  
	>> * *void LCD_wr_data(unsigned char data)*
	>> * *void LCD_wr_cmd(unsigned char cmd)*
	>>
	3. Creacion de la funcion de Init *LCD_init()*
	|Comando|Funcion|
	|---|---|
	|*wr_cmd(0xAE)*|Display off|
	|*wr_cmd(0xA2)*|Fija el valor de la relación de la tensión de polarización del LCD a 1/9|
	|*wr_cmd(0xA0);*|El direccionamiento de la RAM de datos del display es la normal|
	|*wr_cmd(0xC8);*|El scan en las salidas COM es el normal|
	|*wr_cmd(0x22);*|Fija la relación de resistencias interna a 2|
	|*wr_cmd(0x2F);*|Power on|
	|*wr_cmd(0x40);*|Display empieza en la línea 0|
	|*wr_cmd(0xAF);*|Display ON|
	|*wr_cmd(0x81);*|Contraste|
	|*wr_cmd(0x??);*|Valor Contraste|
	|*wr_cmd(0xA4);*|Display all points normal|
	|*wr_cmd(0xA6);*|LCD Display normal|




	
