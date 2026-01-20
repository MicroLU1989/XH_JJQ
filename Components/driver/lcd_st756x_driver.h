#ifndef _LCD_ST756X_DRIVER_
#define _LCD_ST756X_DRIVER_


#include <stdint.h>


void lcd_init(void);
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);



#endif