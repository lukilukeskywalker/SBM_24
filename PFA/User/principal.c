#include "principal.h"
#include <assert.h>


/*--------------------------------*
 *					LCD
 *--------------------------------*/
static MSGQUEUE_OBJ_LCD msg_lcd;
extern osMessageQueueId_t mid_MsgQueue_lcd;


/*--------------------------------*
 *					CLOCK
 *--------------------------------*/
extern int hours;
extern int minutes;
extern int seconds;


/*--------------------------------*
 *					JOYSTICK
 *--------------------------------*/
static MSGQUEUE_OBJ_JOY msg_joy;
extern osMessageQueueId_t mid_MsgQueue_joy;


/*--------------------------------*
 *					RGB
 *--------------------------------*/
static MSGQUEUE_OBJ_RGB msg_rgb;
extern osMessageQueueId_t mid_MsgQueue_rgb;


/*--------------------------------*
 *					I2C (TSL2591)
 *--------------------------------*/
static MSGQUEUE_OBJ_I2C msg_i2c;
extern osMessageQueueId_t mid_MsgQueue_I2C;
extern osThreadId_t tid_Th_I2C;


/*--------------------------------*
 *					COM
 *--------------------------------*/
static MSGQUEUE_OBJ_COM msg_com_rx;
static MSGQUEUE_OBJ_COM msg_com_tx;
extern osMessageQueueId_t mid_MsgQueue_com_rx;
extern osMessageQueueId_t mid_MsgQueue_com_tx;


/*--------------------------------*
 *					BUFFER
 *--------------------------------*/
 
typedef struct{
	uint8_t hora;
	uint8_t min;
	uint8_t seg;
	uint8_t lum_cen; //‰ es un por millaje ...suck it
	uint8_t lum_dec;
	uint8_t lum_uni;
}time_lum_pack_t;
//Tiempos desesperados requieren medidas desperadas...
//Voy a asignar el pointer a buffer_circ_mem, y apuntar desde buffer_irc a buffer_circ_mem...
//Pero ni zorra de porque no se aloca la memoria.
static time_lum_pack_t buffer_circ_mem[BUFFER_SIZE];

typedef struct{
	uint32_t capacidad;
	uint32_t elementos;
	uint32_t ultima_entrada;
	time_lum_pack_t* i_buffer_circ;
}buffer_t;
static buffer_t* buffer_circ=NULL;

static time_lum_pack_t* parse_data_rx(MSGQUEUE_OBJ_I2C *msg){
	time_lum_pack_t* data;
	data =malloc(sizeof(time_lum_pack_t));
	data->hora = hours;
	data->min = minutes;
	data->seg = seconds;
	data->lum_cen = msg->light_cen;
	data->lum_dec = msg->light_dec;
	data->lum_uni = msg->light_uni;
	
	return data;
}

static buffer_t* init_Buffer(buffer_t *buf, uint16_t _capacidad){
	//buf->i_buffer_circ = malloc(_capacidad*sizeof(time_lum_pack_t));
	//buffer_circ_mem=malloc(_capacidad*sizeof(time_lum_pack_t));
	buf->i_buffer_circ=buffer_circ_mem;
	assert(buf->i_buffer_circ!=NULL);//if(buf->i_buffer_circ == NULL)return NULL;
	buf->capacidad =_capacidad;
	buf->elementos = 0;
	buf->ultima_entrada = 0;
	if(buf->capacidad == _capacidad && buf->elementos == 0 && buf->ultima_entrada == 0)return buf;
	return NULL;	
}

static time_lum_pack_t* introducirDatoBuffer(buffer_t* buffer,  time_lum_pack_t *dato){
	//time_lum_pack_t *last_pos_pointer=buffer->i_buffer_circ + (buffer->ultima_entrada);//*sizeof(time_lum_pack_t));
	memcpy(buffer->i_buffer_circ+buffer->ultima_entrada, dato, sizeof(time_lum_pack_t));
	buffer->ultima_entrada = (buffer->ultima_entrada + 1) % buffer->capacidad;
	if(buffer->elementos < buffer->capacidad)
		buffer->elementos++;
	free(dato);
	return (buffer->i_buffer_circ + (buffer->ultima_entrada * sizeof(time_lum_pack_t)));
}

static int borrarDatosBuffer(buffer_t* buffer){
	buffer->elementos = 0;
	buffer->ultima_entrada = 0;
	return buffer->elementos|buffer->ultima_entrada;
}

static int *recogerDatoBuffer(buffer_t* buffer, time_lum_pack_t *medida, uint8_t n_elemento){
	if(n_elemento >buffer->elementos) return NULL;
	return memcpy(medida, buffer->i_buffer_circ+n_elemento, sizeof(time_lum_pack_t));
}
static uint32_t num_elementos(buffer_t* buffer){
	//Deberia haber empezado desde el principio con una API en condiciones... vaya desastre
	uint32_t elementos = buffer->elementos;
	return elementos;
}


/*--------------------------------*
 *					PRINCIPAL
 *--------------------------------*/
typedef enum{UNLOCKED = 0, LOCKED}lock_t;

typedef enum{INACTIVO = 0, ACTIVO, MEDIDA}estados_t;
static uint32_t tiempo_entre_ciclos;
static uint8_t numero_ciclos;
static uint8_t ciclos_left;
static uint8_t countdown;
static uint8_t countdown_counter;
static estados_t estado;  //CHANGE! Ademas, hay un limbo por ahi...
static lock_t lock;

osThreadId_t tid_Th_principal;
 
static void Th_principal (void *argument);
static void hour_setup(MSGQUEUE_OBJ_COM msg);
static void countdown_setup(MSGQUEUE_OBJ_COM msg);
static void countdown_read(MSGQUEUE_OBJ_COM msg);

/*++++FUNCIONES DE LUMINOSIDAD++++*/
static void start_measure_cycle(MSGQUEUE_OBJ_COM msg);
static void muestreo_hora(void);
static void muestreo_lum(void);
static uint8_t start_cycle; 

/*++++FUNCIONES DE RESPUESTA++++*/
static void number_measures(MSGQUEUE_OBJ_COM msg);
static void last_measure(MSGQUEUE_OBJ_COM msg);
static void ret_all_measure(MSGQUEUE_OBJ_COM msg);
static MSGQUEUE_OBJ_COM measureToMsg(MSGQUEUE_OBJ_COM *msg, uint8_t entrada);



int Init_Th_principal (void) {
  tid_Th_principal = osThreadNew(Th_principal, NULL, NULL);
  if (tid_Th_principal == NULL)
    return(-1);
  return(0);
}

static void Th_principal (void *argument) {
	buffer_circ = malloc(sizeof(buffer_t));
	assert(buffer_circ !=NULL);
	//printf("Pointer del buffer: %p \n\r", buffer_circ);
	osStatus_t status_msg;
	estado = INACTIVO;
	msg_rgb.estado_rgb = ON;
	osMessageQueuePut(mid_MsgQueue_rgb, &msg_rgb, NULL, 0U);
	msg_lcd.init_L1 = 30;
	msg_lcd.init_L2 = 10;
	countdown = 9;
	lock = UNLOCKED;
	init_Buffer(buffer_circ, BUFFER_SIZE);
	
	sprintf(msg_lcd.data_L2, "ESTADO INACTIVO");
  while (1) {
		muestreo_lum();
		muestreo_hora();
		
		switch (estado){
			case INACTIVO:			
				status_msg = osMessageQueueGet(mid_MsgQueue_joy, &msg_joy, NULL, 0U);
				if((status_msg == osOK) && (msg_joy.tecla == Arriba) && (msg_joy.duracion == Larga)){
					msg_rgb.estado_led2 = ON;
					msg_rgb.estado_led3 = OFF;
					osMessageQueuePut(mid_MsgQueue_rgb, &msg_rgb, NULL, 0U);
					msg_lcd.init_L2 = 20;
					sprintf(msg_lcd.data_L2, "ESTADO ACTIVO");
					estado = ACTIVO;
				}
			break;
			
			case ACTIVO:				
				status_msg = osMessageQueueGet(mid_MsgQueue_com_rx, &msg_com_rx, NULL, 0U);
				if(status_msg == osOK){
					switch (msg_com_rx.op){
						case HOUR_SETUP:
							hour_setup(msg_com_rx);
						break;
						
						case COUNTDOWN_SETUP:
							countdown_setup(msg_com_rx);
						break;
						
						case COUNTDOWN_READ:
							countdown_read(msg_com_rx);
						break;
						
						default:
							
						break;
					}
				}				
				status_msg = osMessageQueueGet(mid_MsgQueue_joy, &msg_joy, NULL, 0U);
				if((status_msg == osOK) && (msg_joy.tecla == Arriba) && (msg_joy.duracion == Larga)){
					msg_rgb.estado_led2 = OFF;
					msg_rgb.estado_led3 = OFF;
					osMessageQueuePut(mid_MsgQueue_rgb, &msg_rgb, NULL, 0U);
					msg_lcd.init_L2 = 10;
					sprintf(msg_lcd.data_L2, "ESTADO INACTIVO");
					estado = INACTIVO;
				}
			break;
			
			case MEDIDA:
				if(lock == UNLOCKED){//En mitad de ciclo medidas
					//MANUAL
					status_msg = osMessageQueueGet(mid_MsgQueue_joy, &msg_joy, NULL, 0U);
					if((status_msg == osOK) && (msg_joy.tecla == Abajo) && (msg_joy.duracion == Corta)){
						osThreadFlagsSet(tid_Th_I2C, INIT_LIGHT_MEASURE);
					}
					//COMS
					status_msg = osMessageQueueGet(mid_MsgQueue_com_rx, &msg_com_rx, NULL, 0U);
					if(status_msg == osOK){
						switch (msg_com_rx.op){
							case START_MEASURE:
								start_measure_cycle(msg_com_rx);
							break;
							
							case RET_NUM_MEASURE:
								number_measures(msg_com_rx);
							break;
								
							case RET_LAST_MEASURE:
								last_measure(msg_com_rx);
							break;
							
							case RET_ALL_MEASURE:
								ret_all_measure(msg_com_rx);
							break;
							
							case DEL_ALL_MEASURE:
								borrarDatosBuffer(buffer_circ);
								msg_com_rx.last_msg=true;
								osMessageQueuePut(mid_MsgQueue_com_tx, &msg_com_rx, NULL, 0U);
							break;
							
							default:	

							break;
						}
					}
				}
			break;
		}
	}
}


static void hour_setup(MSGQUEUE_OBJ_COM msg){ 
    char data[32];
    int index, index_p = 0;
    int horas, minutos, segundos;

    memcpy(data, msg.data, msg.size);
	
    index = strchr(data,':') - data + 1;	//Devuelve un puntero a 
    memcpy(data, data,(index - index_p));
    horas = atoi(data);
    
    index_p = index;
    memcpy(data, msg.data + index_p, (msg.size - index_p));

    index = strchr(data,':') - data + 1;
    memcpy(data, data,(index - index_p));
    minutos = atoi(data);
    
    index_p = index;
    memcpy(data, msg.data + index_p, (msg.size - index_p));

    index = strchr(data,':') - data + 1;
    memcpy(data, data,(index - index_p));
    segundos = atoi(data);;

		lock = UNLOCKED;
		estado = MEDIDA;
		set_clock(horas,minutos,segundos);
		sprintf(msg_lcd.data_L2, "ESTADO MEDIDA");
		msg.last_msg = true;		
		osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}







/**********************************************/
/*		FUNCIONES DE MUESTREO EN PANTALLA				*/
/**********************************************/
static void muestreo_hora(){
	static int last_sec;
	if(last_sec != seconds){
		last_sec=seconds;
		msg_lcd.init_L1 = 30;
		sprintf(msg_lcd.data_L1, "%02d : %02d : %02d", hours, minutes, seconds);
		if(lock==LOCKED & countdown_counter!=0){
			msg_lcd.init_L2 = 0;
			sprintf(msg_lcd.data_L2, "Cycle starts in: %d", countdown_counter);
			countdown_counter--;
			if(countdown_counter == 0){
				osThreadFlagsSet(tid_Th_I2C, INIT_LIGHT_CYCLE|(numero_ciclos<<N_CYCLE_SHIFT)|(tiempo_entre_ciclos<<N_TIME_SHIFT));
			}
		}
		osMessageQueuePut(mid_MsgQueue_lcd, &msg_lcd, NULL, 0U);
	}
	
}
static void muestreo_lum(){
	//Primero leo la cola. Si no tiene contenido, se vuelve. Si tiene contenido, se escribe en la pantalla
	//Non-blocking
	osStatus_t status_msg;
	status_msg= osMessageQueueGet(mid_MsgQueue_I2C, &msg_i2c, NULL, 0);
	if(status_msg==osOK){
		msg_rgb.estado_led3 = ON;
		osMessageQueuePut(mid_MsgQueue_rgb, &msg_rgb, NULL, 0U);
		introducirDatoBuffer(buffer_circ, parse_data_rx(&msg_i2c));
		if(msg_i2c.ciclo == true){
			msg_lcd.init_L2 = 30;
			sprintf(msg_lcd.data_L2, "C Luz: %d%d.%d%%", msg_i2c.light_cen, msg_i2c.light_dec, msg_i2c.light_uni);
			numero_ciclos--;
			if(numero_ciclos == 0)lock=UNLOCKED;
		}
		else{
			msg_lcd.init_L2 = 35;
			sprintf(msg_lcd.data_L2, "Luz: %d%d.%d%%", msg_i2c.light_cen, msg_i2c.light_dec, msg_i2c.light_uni);
		}

		
	}else if(status_msg==osErrorResource){
		//There is nothing to see here
	}	
	return;
}

/**********************************************/
/*			FUNCIONES DE MUESTREO LUMINICA				*/
/**********************************************/
static void start_measure_cycle(MSGQUEUE_OBJ_COM msg){ 
	char data[32];
	int index_N, index_T;
	
	index_N = strchr(msg.data,'N') - msg.data + 1;
	index_T = strchr(msg.data,'T') - msg.data + 1;

	memcpy(data, msg.data + index_N, (index_T - index_N - 1));
	numero_ciclos = atoi(data);
	
	memset(data,0,sizeof(data));
	memcpy(data, (msg.data + index_T), (msg.size - index_T));      
	tiempo_entre_ciclos = atoi(data);
	lock=LOCKED;
	countdown_counter=countdown;
	//osThreadFlagsSet(tid_Th_I2C, INIT_LIGHT_CYCLE|(numero_ciclos<<N_CYCLE_SHIFT)|(tiempo_entre_ciclos<<N_TIME_SHIFT));
	msg.last_msg=true;
	osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}


/**********************************************/
/*					FUNCIONES DE RESPUESTA						*/
/**********************************************/
static void countdown_setup(MSGQUEUE_OBJ_COM msg){ 
	countdown = atoi(msg.data);
	msg.last_msg = true;
	osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}

static void countdown_read(MSGQUEUE_OBJ_COM msg){ 
	msg.data[0] = countdown;
	msg.size = 1;
	osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}
static void number_measures(MSGQUEUE_OBJ_COM msg){
	//msg.data[0]=&buffer_circ->elementos;
	//msg.size=1;
	//memcpy(msg.data, (void *)buffer_circ->elementos, 1);
	uint32_t elementos=num_elementos(buffer_circ);
	if(elementos >= 10){
		msg.size=2;
		msg.data[0]=(elementos/10)+ASCII_DISP;	//Este 30 esta para hacer que sea ASCII
		msg.data[1]=(elementos%10)+ASCII_DISP;	//Ya que en el enunciado no se especifica si debe ser ascii o no
	}else{
		msg.size=1;
		msg.data[0]=(elementos+ASCII_DISP);	//ASCII DISPLACEMENT. Un numero en ascii es el mismo en hex + 30
	}
	msg.last_msg=true;
	osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}
static void ret_all_measure(MSGQUEUE_OBJ_COM msg){
	//if(buffer_circ->elementos==buffer_circ->capacidad)	//Nuestro buffer_circ esta lleno
	//Primero supongamos que el buffer_circ nunca se lleno. En ese caso tenemos que devolver desde la posicion 0 a la posicion elementos
	/*v1
	if(buffer_circ->elementos < buffer_circ->capacidad){
		for(int i=0; i<buffer_circ->elementos; i++){
			msg.Number=buffer_circ->elementos-i;
			measureToMsg(&msg, i);
			osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
		}
	}
	else{
		//En cambio, si el buffer esta lleno, debo empezar en buffer->ultimaentrada+1 incrementar y rollear hasta llegar a buffer->ultimaentrada
		for(int i=buffer->ultima_entrada+1; i!=buffer->ultima_entrada; i=(i+1)%buffer->capacidad){
			measureToMsg(&msg, i);
			osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
		}
	}*/
	
	//v2
	int i=-1;
	msg.last_msg=false;
	msg.op=0x50;	//Debemos devolver el comando de medida, y no el inverso
	if(buffer_circ->elementos == 0) return;	//Puede producirse un memory dump sino
	if(buffer_circ->elementos == buffer_circ->capacidad){
		i=buffer_circ->ultima_entrada;
	}
	do{
		i=(i+1)%buffer_circ->capacidad;
		if(i==buffer_circ->ultima_entrada-1)msg.last_msg=true;
		measureToMsg(&msg, i);
		osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 20U);
	}while(!msg.last_msg);
}
	

static void last_measure(MSGQUEUE_OBJ_COM msg){
	msg.last_msg=true;
/*	msg.op=0x40;
//	time_lum_pack_t medida;
//	recogerDatoBuffer(buffer,&medida, buffer->ultima_entrada);
//	msg.data[0]=medida.hora%10;
//	msg.data[1]=medida.hora-msg.data[0];
//	msg.data[2]=0x3A;
//	msg.data[3]=medida.min%10;
//	msg.data[4]=medida.min-msg.data[3];
//	msg.data[5]=0x3A;
//	msg.data[6]=medida.seg%10;
//	msg.data[7]=medida.seg-msg.data[6];
//	msg.data[8]=0x2D;
//	msg.data[9]=medida.lum_cen;
//	msg.data[10]=medida.lum_dec;
//	msg.data[11]=0x2E;
//	msg.data[12]=medida.lum_uni;
//	msg.data[13]=0x25;
//	msg.size=14;*/
	measureToMsg(&msg, buffer_circ->ultima_entrada-1);
	osMessageQueuePut(mid_MsgQueue_com_tx, &msg, NULL, 0U);
}
static MSGQUEUE_OBJ_COM measureToMsg(MSGQUEUE_OBJ_COM *msg, uint8_t entrada){
	//msg.Number=0;
	msg->op=0x50;
	time_lum_pack_t medida;
	recogerDatoBuffer(buffer_circ,&medida, entrada);
	msg->data[0]=medida.hora/10+ASCII_DISP;
	msg->data[1]=medida.hora%10+ASCII_DISP;
	msg->data[2]=0x3A;
	msg->data[3]=medida.min/10+ASCII_DISP;
	msg->data[4]=medida.min%10+ASCII_DISP;
	msg->data[5]=0x3A;
	msg->data[6]=medida.seg/10+ASCII_DISP;
	msg->data[7]=medida.seg%10+ASCII_DISP;
	msg->data[8]=0x2D;
	if(medida.lum_cen != 0){
		msg->data[9]=medida.lum_cen+ASCII_DISP;
		msg->data[10]=medida.lum_dec+ASCII_DISP;
		msg->data[11]=0x2E;
		msg->data[12]=medida.lum_uni+ASCII_DISP;
		msg->data[13]=0x25;
		msg->size=14;
	}
	else{
		msg->data[9]=medida.lum_dec+ASCII_DISP;
		msg->data[10]=0x2E;
		msg->data[11]=medida.lum_uni+ASCII_DISP;
		msg->data[12]=0x25;
		msg->size=13;
	}
	return *msg;
}



