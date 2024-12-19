# Clock

Sencillo Programa que ejecuta 2 hilos paralelos del CMSISV2-os. En el hilo Th_clock se inicializa un timer del OS que ejecuta un callback que incrementa una variable **sec**, hasta que desborda, y vuelve a ser 0.
Incluidas vienen funciones que convierten segundos_totales en segundos, minutos y horas y viceversa

El hilo de test comprueba que realmente se incremente el segundero una vez al segundo
