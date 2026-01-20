#ifndef _LCD_DRAW_H_
#define _LCD_DRAW_H_

#include<stdint.h>

typedef enum
{
    ICON_WIFI = 0,
    ICON_ETHERNET = 1,
    ICON_BT = 2,

}ICON_INDE_E;


void lcd_draw_point(uint8_t x, uint8_t y);
void lcd_clear_point(uint8_t x, uint8_t y);
void lcd_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void lcd_draw_circle(uint8_t x, uint8_t y, uint8_t r);
void lcd_draw_refresh(void);
void lcd_darw_init(void);
void lcd_show_icon(uint8_t x, uint8_t y, ICON_INDE_E index);
void lcd_show_string(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size);
void lcd_clear_screen(void);


// 全屏填充
void lcd_fill_screen(void);

// 设置对比度（可调参数）
void lcd_set_contrast(uint8_t contrast);

#endif