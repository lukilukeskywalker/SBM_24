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
* JOY_ARRIBA: PB10  (pin15 en mbed_board);
* JOY_DERECHA: PB11 (pin16 en mbed_board);
* JOY_ABAJO: PE12	(pin12 en mbed_board);
* JOY_IZQUIEDA: PE14(pin13 en mbed_board);
* JOY_CENTRO: PE15	(pin14 en mbed_board);
## Datasheet y posiciones
[Link Datasheet](https://www.st.com/resource/en/datasheet/dm00071990.pdf)
[Link Nucleo_MBED](https://os.mbed.com/platforms/ST-Nucleo-F429ZI/)
### Posiciones del datasheet
Funciones alternativas de pines: Pag 75
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
	Explicacion de los comandos en la tabla de abajo
	4. Escritura de datos en el LCD
	Se escribe una funcion denominada *LCD_update()* que permite copiar la informacion de un array de datos global, *buffer* de 512 elementos. Cada uno de los bits de ese array (4096=512x8) representa el estado de uno de los 128*32 pixeles de la pantalla
4. 5 Tareas:
	1. Incluir fichero Arial12x12.h y incluir una funcion *void symbolToLocalBuffer_L1(uint8_t symbol)*
	```C
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
	```
	2. Crear un proyecto que escriba una linea de texto al LCD **Prueba de texto L1** Hay que modificar la funcion para que la anchura por cada letra no sea fija, y el texto se ajuste
	3. Crear una segunda función *void symbolToLocalBuffer_L2(uint8_t symbol)* que permita ver en la segunda linea el texto **Prueba de texto L2**
	```C
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
	```
	4. Estudiar el prototipo de la funcion de libreria *sprintf()* de tal forma que podamos escribir variables en la pantalla, apartir de diferentes tipos de datos, como uint32_t o float
	5. Utilizar una fuente diferente para el texto.
5. Esta practica consiste en utilizar RTOSV2. Hay 3 tareas
	1. Crear un hilo llamado Thled1 que encienda y apague periodicamente un led, 200ms encendido 800 ms apagado.
	Ademas hay que utilizar el wizard del fihero RTX_Config.h para cambiar el tick del sistema
	2. Cambiar el tick de sistema a 1 ms, e introducir 2 nuevos hilos Thled2 y Thled3. Los peridos de enendido y apagado de los leds son 137 ms y 137 ms para el led 2 y 287 ms y 287 ms para el led 3. Los hilos deben ser ejecutados de forma concurrente.
	3. Sincronizar la ejecucion de los 3 hilos mediante el uso de flags. Utilizar un watch para ver la ejecucion de los hilos (View->Watch Windows->RTX RTOS)
6. 5 Tareas:
	1. Contador Binario, que cuenta con el boton B1 (usando interrupciones), y mostrando el valor en los leds 1, 2, 3 (Valor, 4, 2, 1). No se deben utilizar variables globales
	(Muestra de puta madre los rebotes)
	2. Detectar pulsaciones del gesto RIGHT del joystick 
	3. Hacer un contador de 3 segundos, que se puede resetear mientras que no haya terminado la cuenta atras con el boton 1. Mientras que esta ccontando, el led verde (LED 1)esta encendido, una vez termina, el led verde se apaga y se enciende el led rojo (LED 3)
	4. Eliminar rebotes con timers del sistema operativo. Cada interrupcion en un pin debe iniciar un timer one-shot. Una vez finaliza el timer one-shot, se observa el estado de los pines, y se guarda en una variable el numero de vees que ha sido accionado el joystick en determinada direccion. Ademas se debe mostrar en los leds de forma codificada (Arriba=1, Derecha=2, Abajo=3, Izquierda=4, Centro=5);
	5. Encolar las pulsaciones en una cola de mensajes, para que otro modulo lo lea posteriormente. La informacion a enviar a la cola es el valor del joystick leido como un conjunto de 5 bits. Para comprobar que funciona, representar el gesto pulsado en el LCD ccon otro hilo.


### Tabla comandos LCD
>> | Comando         | Funcion                                                                  |
>> |-----------------|--------------------------------------------------------------------------|
>> | *wr_cmd(0xAE)*  | Display off                                                              |
>> | *wr_cmd(0xA2)*  | Fija el valor de la relación de la tensión de polarización del LCD a 1/9 |
>> | *wr_cmd(0xA0);* | El direccionamiento de la RAM de datos del display es la normal          |
>> | *wr_cmd(0xC8);* | El scan en las salidas COM es el normal                                  |
>> | *wr_cmd(0x22);* | Fija la relación de resistencias interna a 2                             |
>> | *wr_cmd(0x2F);* | Power on                                                                 |
>> | *wr_cmd(0x40);* | Display empieza en la línea 0                                            |
>> | *wr_cmd(0xAF);* | Display ON                                                               |
>> | *wr_cmd(0x81);* | Contraste                                                                |
>> | *wr_cmd(0x??);* | Valor Contraste                                                          |
>> | *wr_cmd(0xA4);* | Display all points normal                                                |
>> | *wr_cmd(0xA6);* | LCD Display normal                                                       | 
	

	
