#ifndef LCD_16X2_H
#define LCD_16X2_H

#include "estado.h"

void lcd_init(void);
void lcd_limpiar(void);
void lcd_actualizar(const EstadoECU *est);

#endif