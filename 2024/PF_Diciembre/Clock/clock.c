#include "clock.h"

// Estructura para almacenar la hora
program_clock_t current_time = {0, 0, 0};

// Prototipos de funciones
void ClockThread(void *argument);
static void TimerCallback(void *arg);

// Identificadores para el temporizador y el hilo
osTimerId_t timer_id;
osThreadId_t thread_id;

// Función para inicializar el hilo del reloj
int InitClockThread(void) {
    thread_id = osThreadNew(ClockThread, NULL, NULL);
    if (thread_id == NULL) {
        return -1;
    }
	return 0;
}

// Función del hilo del reloj
void ClockThread(void *argument) {
    timer_id = osTimerNew(TimerCallback, osTimerPeriodic, NULL, NULL);
    osTimerStart(timer_id, 1000U);  // Inicia el temporizador con un período de 1 segundo
    while (1) {
        osThreadYield();  // Cede el control para permitir que otros hilos se ejecuten
    }
}

// Callback del temporizador, se llama cada segundo
static void TimerCallback(void *arg) {
    current_time.segundo++;
    if (current_time.segundo == 60) {
        current_time.segundo = 0;
        current_time.minuto++;
        if (current_time.minuto == 60) {
            current_time.minuto = 0;
            current_time.hora++;
            if (current_time.hora == 24) {
                current_time.hora = 0;
            }
        }
    }
}

// Función para establecer la hora manualmente
void SetClock(uint16_t hour, uint16_t min, uint16_t sec) {
    current_time.hora = (hour < 24) ? hour : 0;
    current_time.minuto = (min < 60) ? min : 0;
    current_time.segundo = (sec < 60) ? sec : 0;
}
