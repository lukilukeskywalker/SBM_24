#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#define BUFFER_SIZE 99

#include "stdlib.h"
#include "clock.h"  
#include "lcd.h"
#include "joystick.h"
#include "rgb.h"
#include "TSL2591.h"
#include "com.h"

int Init_Th_principal (void);
#define RET_ASCII
#ifdef RET_ASCII
#define ASCII_DISP 0x30	//Para establecer un desplazamiento ASCII que es 0x30
#else 
#define ASCII_DISP
#endif

#endif
