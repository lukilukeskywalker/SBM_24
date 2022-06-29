# SBM

## Como crear un nuevo proyecto:
Para crear un proyecto, creamos un proyecto en Project New uvision Project, Seleccionamos el dispositivo, en nuestro caso el
STM32F429ZITx. Una vez creado el proyecto aparecera una pestaña Manage Run, para seleccionar las librerias que queremos utilizar
Tenemos que seleccionar:  
	- Abriendo CMSIS, CMSIS-CORE  
	- Abriendo Device, Device-Startup  
		- Abriendo STM32CubeMX Framework(API) Seleccionar Classic  
		- Apareceran en naranja dependencias a resolver. Seleccionar en la esquina inferior izquierda "Resolve"  
	- Abriendo STM32Cube HAL, STM32Cube HAL- GPIO
		- Apareceran en naranja dependencias a resolver. Seleccionar en la esquina inferior izquierda "Resolve"
Damos Okey  

Se creara un proyecto. Hay que añadirle un main, para tener un sitio donde trabajar.
En source Group, click derecho, y seleccionar "Add New Item to Group 'Source Group 1'
Dentro seleccioanar User Code Template, En component STM32Cube Framework Clasic->'main' module for STM32Cube HAL  

Done!