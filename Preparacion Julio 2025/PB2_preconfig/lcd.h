#ifndef __LCD_H__
#define __LCD_H__

#include "cmsis_os2.h"


void lcd_init(osThreadId_t *id_task);
void lcd_clean(void);
void lcd_write_drawbuf(uint8_t *drawbuf);

osMessageQueueId_t get_id_MsgQueue_lcd(void);

#endif