#include "lcd_draw.h"
#include <string.h>
#include "lcd_st756x_driver.h"
#include "font.h"

#define LCD_COLUMN_SIZE 128
#define LCD_LINE_SIZE 64
#define LCD_PAGE_SIZE (LCD_LINE_SIZE / 8)
#define LCD_COLUMN_OFFSET 0
#define LCD_LINE_OFFSET 0


unsigned char lcd_gram[LCD_PAGE_SIZE][LCD_COLUMN_SIZE] = {0x00};


#if 0
/**************************************************************************
 * @brief        : 画点
 * @param         {uint8_t} x: 列坐标
 * @param         {uint8_t} y: 行坐标
 * @return        {*}
 ***************************************************************************/
void lcd_draw_point(uint8_t x, uint8_t y)
{
    if (x > 127 || y > 63)
        return;
    lcd_gram[y / 8][x] |= (1 << (y % 8));
}

/**************************************************************************
 * @brief        : 清除一个点
 * @param         {uint8_t} x:列坐标 x:0~127
 * @param         {uint8_t} y:行坐标 y:0~63
 * @return        {*} 
 ***************************************************************************/
void lcd_clear_point(uint8_t x, uint8_t y)
{
    if (x > 127 || y > 63)
        return;
    lcd_gram[y / 8][x] = ~lcd_gram[y / 8][x];
    lcd_gram[y / 8][x] |= (1 << (y % 8));
    lcd_gram[y / 8][x] = ~lcd_gram[y / 8][x];
}


/**************************************************************************
 * @brief        :   画线
 * @param         {uint8_t} x1: 起始列坐标 0~127
 * @param         {uint8_t} y1: 起始行坐标 0~63
 * @param         {uint8_t} x2: 终点列坐标 0~127
 * @param         {uint8_t} y2: 终点行坐标 0~63
 * @return        {*}
 ***************************************************************************/
void lcd_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    uint8_t i, k, k1, k2, y0;
    if ((x2 > 127) || (y2 > 63) || (x1 > x2) || (y1 > y2))
        return;
    if (x1 == x2) // 画竖线
    {
        for (i = 0; i < (y2 - y1); i++)
        {
            lcd_draw_point(x1, y1 + i);
        }
    }
    else if (y1 == y2) // 画横线
    {
        for (i = 0; i < (x2 - x1); i++)
        {
            lcd_draw_point(x1 + i, y1);
        }
    }
    else // 画斜线
    {
        k1 = y2 - y1;
        k2 = x2 - x1;
        k = k1 * 10 / k2;
        for (i = 0; i < (x2 - x1); i++)
        {
            lcd_draw_point(x1 + i, y1 + i * k / 10);
        }
    }
}


/**************************************************************************
 * @brief        : 清除指定区域,区域形状为矩形
 * @param         {uint8_t} x1: 起始列坐标
 * @param         {uint8_t} x2: 结束列坐标
 * @param         {uint8_t} y1: 起始行坐标
 * @param         {uint8_t} y2: 结束行坐标
 * @return        {*}
 ***************************************************************************/
void lcd_clear_specified_area(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
    uint8_t column, row;
    for (column = x1; column <= x2; column++)
    {
        for (row = y1; row <= y2; row++)
        {
            lcd_clear_point(column, row);
        }
    }
}

/**************************************************************************
 * @brief        : 画圆,x,y为圆心坐标,r为半径
 * @param         {uint8_t} x: 列坐标 x:0~127
 * @param         {uint8_t} y: 行坐标 y:0~63
 * @param         {uint8_t} r: 半径
 * @return        {*}
 ***************************************************************************/
void lcd_draw_circle(uint8_t x, uint8_t y, uint8_t r)
{
    int a, b, num;
    a = 0;
    b = r;
    while (2 * b * b >= r * r)
    {
        lcd_draw_point(x + a, y - b);
        lcd_draw_point(x - a, y - b);
        lcd_draw_point(x - a, y + b);
        lcd_draw_point(x + a, y + b);

        lcd_draw_point(x + b, y + a);
        lcd_draw_point(x + b, y - a);
        lcd_draw_point(x - b, y - a);
        lcd_draw_point(x - b, y + a);
        a++;
        num = (a * a + b * b) - r * r; // 计算画的点离圆心的距离
        if (num > 0)
        {
            b--;
            a--;
        }
    }
}

#else
// 修正画点函数
void lcd_draw_point(uint8_t x, uint8_t y)
{
    if (x >= LCD_COLUMN_SIZE || y >= LCD_LINE_SIZE)
        return;
    
    uint8_t page = y / 8;
    uint8_t bit_mask = 1 << (y % 8);
    
    lcd_gram[page][x] |= bit_mask;
}

// 修正清除点函数
void lcd_clear_point(uint8_t x, uint8_t y)
{
    if (x >= LCD_COLUMN_SIZE || y >= LCD_LINE_SIZE)
        return;
    
    uint8_t page = y / 8;
    uint8_t bit_mask = ~(1 << (y % 8));
    
    lcd_gram[page][x] &= bit_mask;
}

void lcd_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    if (x1 > LCD_COLUMN_SIZE || y1 > LCD_LINE_SIZE || 
        x2 > LCD_COLUMN_SIZE || y2 > LCD_LINE_SIZE)
        return;
    
    if (x1 == x2) { // 竖线
        uint8_t start_y = (y1 < y2) ? y1 : y2;
        uint8_t end_y = (y1 < y2) ? y2 : y1;
        for (uint8_t y = start_y; y <= end_y; y++) {
            lcd_draw_point(x1, y);
        }
    }
    else if (y1 == y2) { // 横线
        uint8_t start_x = (x1 < x2) ? x1 : x2;
        uint8_t end_x = (x1 < x2) ? x2 : x1;
        for (uint8_t x = start_x; x <= end_x; x++) {
            lcd_draw_point(x, y1);
        }
    }
    else { // Bresenham直线算法（避免浮点运算）
        int dx = x2 - x1;
        int dy = y2 - y1;
        int sx = (dx > 0) ? 1 : -1;
        int sy = (dy > 0) ? 1 : -1;
        dx = (dx > 0) ? dx : -dx;
        dy = (dy > 0) ? dy : -dy;
        
        int err = dx - dy;
        int e2;
        
        while (1) {
            lcd_draw_point(x1, y1);
            if (x1 == x2 && y1 == y2) break;
            
            e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }
}


#endif


/**************************************************************************
 * @brief        : 画圆,x,y为圆心坐标,r为半径
 * @param         {uint8_t} x: 列坐标 x:0~127
 * @param         {uint8_t} y: 行坐标 y:0~63
 * @param         {uint8_t} r: 半径
 * @return        {*}
 ***************************************************************************/
void lcd_draw_circle(uint8_t cx, uint8_t cy, uint8_t r)
{
    if (r == 0 || cx < r || cx >= LCD_COLUMN_SIZE - r || 
        cy < r || cy >= LCD_LINE_SIZE - r) {
        return;
    }
    
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 1 - r;
    
    while (x <= y) {
        // 使用局部变量减少重复计算
        uint8_t cx_p_x = cx + x;
        uint8_t cx_m_x = cx - x;
        uint8_t cx_p_y = cx + y;
        uint8_t cx_m_y = cx - y;
        uint8_t cy_p_y = cy + y;
        uint8_t cy_m_y = cy - y;
        uint8_t cy_p_x = cy + x;
        uint8_t cy_m_x = cy - x;
        
        lcd_draw_point(cx_p_x, cy_p_y);
        lcd_draw_point(cx_m_x, cy_p_y);
        lcd_draw_point(cx_p_x, cy_m_y);
        lcd_draw_point(cx_m_x, cy_m_y);
        lcd_draw_point(cx_p_y, cy_p_x);
        lcd_draw_point(cx_m_y, cy_p_x);
        lcd_draw_point(cx_p_y, cy_m_x);
        lcd_draw_point(cx_m_y, cy_m_x);
        
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}


/**************************************************************************
 * @brief        : 清除指定区域,区域形状为矩形 (优化版)
 * @param         {uint8_t} x1: 起始列坐标
 * @param         {uint8_t} x2: 结束列坐标  
 * @param         {uint8_t} y1: 起始行坐标
 * @param         {uint8_t} y2: 结束行坐标
 * @return        {*}
 ***************************************************************************/
void lcd_clear_specified_area(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
    // 参数标准化和边界检查
    if (x1 > x2) { uint8_t temp = x1; x1 = x2; x2 = temp; }
    if (y1 > y2) { uint8_t temp = y1; y1 = y2; y2 = temp; }
    
    // 确保坐标在有效范围内
    x1 = (x1 < LCD_COLUMN_SIZE) ? x1 : 0;
    x2 = (x2 < LCD_COLUMN_SIZE) ? x2 : LCD_COLUMN_SIZE - 1;
    y1 = (y1 < LCD_LINE_SIZE) ? y1 : 0;  
    y2 = (y2 < LCD_LINE_SIZE) ? y2 : LCD_LINE_SIZE - 1;
    
    // 空区域检查
    if (x1 > x2 || y1 > y2) return;
    
    // 按页处理，提高效率
    uint8_t start_page = y1 / 8;
    uint8_t end_page = y2 / 8;
    uint8_t page_start_bit = y1 % 8;
    uint8_t page_end_bit = y2 % 8;
    
    for (uint8_t page = start_page; page <= end_page; page++) {
        uint8_t mask = 0xFF;
        
        // 处理页首的不完整字节
        if (page == start_page && page_start_bit != 0) {
            mask &= 0xFF << page_start_bit;
        }
        
        // 处理页尾的不完整字节  
        if (page == end_page && page_end_bit != 7) {
            mask &= 0xFF >> (7 - page_end_bit);
        }
        
        // 整列处理
        for (uint8_t col = x1; col <= x2; col++) {
            if (mask == 0xFF) {
                // 整个字节清除
                lcd_gram[page][col] = 0x00;
            } else {
                // 部分位清除
                lcd_gram[page][col] &= ~mask;
            }
        }
    }
}


/**************************************************************************
 * @brief        : 刷新显存   
 * @return        {*}
 ***************************************************************************/
void lcd_draw_refresh(void)
{
	for(int i=0;i<LCD_PAGE_SIZE;i++)
	{
	    lcd_write_cmd(0Xb0 | i);
	    lcd_write_cmd(0x10);
	    lcd_write_cmd(0x00);
        for(int j=0;j<128;j++)
        {
            lcd_write_data(lcd_gram[i][j]);
        }
	}
}


// 清屏函数
void lcd_clear_screen(void)
{
    memset(lcd_gram, 0x00, sizeof(lcd_gram));
    lcd_draw_refresh();
}

// 全屏填充
void lcd_fill_screen(void)
{
    memset(lcd_gram, 0xFF, sizeof(lcd_gram));
    lcd_draw_refresh();
}

// 设置对比度（可调参数）
void lcd_set_contrast(uint8_t contrast)
{
    if (contrast > 0x3F) contrast = 0x3F; // ST7567对比度范围0x00-0x3F
    lcd_write_cmd(0x81);
    lcd_write_cmd(contrast);
}



// 显示图标
// x列地址  y行地址
void lcd_show_icon(uint8_t x, uint8_t y, ICON_INDE_E index)
{
    uint8_t i, m, n = 0, temp, chr1;
    uint8_t x0 = x, y0 = y;
    uint8_t size = 16;
    uint8_t page_size = size / 8;
    while (page_size--)
    {
        chr1 = index * size / 8 + n;
        n++;
        for (i = 0; i < size; i++)
        {
            temp = icon_16[chr1][i];
            for (m = 0; m < 8; m++)
            {
                if (temp & 0x01)
                    lcd_draw_point(x, y);
                else
                    lcd_clear_point(x, y);
                temp >>= 1;
                y++;
            }
            x++;
            if ((x - x0) == size)
            {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}


/**************************************************************************
 * @brief        : 在指定位置显示字符,y 为行坐标,0~(63 - 8),可以在任意行显示
 * @param         {uint8_t} x: 列坐标 x:0~127
 * @param         {uint8_t} y: 行坐标 y:0~(63 - 8) ,一个字符最小占8个点位
 * @param         {uint8_t} chr: 字符
 * @param         {uint8_t} size:选择字体 12/16/24
 * @return        {*}
 ***************************************************************************/
void lcd_show_chr(uint8_t x, uint8_t y, uint8_t chr, uint8_t size)
{
    uint8_t i, m, temp = 0, chr1 = 0;
    uint8_t x0 = x, y0 = y;
    uint8_t column_size = size / 2; // 一个字符占几列
    uint16_t index = chr - ' ';
    for (i = 0; i < size; i++)
    {
        if (size == 12)
        {
            temp = ascii_6x12[index][i];
        }
        else if (size == 16)
        {
            temp = ascii_8x16[index][i];
        }
        for (m = 0; m < 8; m++)
        {
            if (temp & 0x01)
                lcd_draw_point(x, y);
            else
                lcd_clear_point(x, y);
            temp >>= 1;
            y++;
        }
        x++;
        if ((x - x0) == column_size)
        {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}


/**************************************************************************
 * @brief        : 显示字符串,
 * @param         {uint8_t} x: 列坐标 x:0~127
 * @param         {uint8_t} y: 行坐标 y:0~(63 - 8) ,一个字符最小占8个点位
 * @param         {uint8_t*} chr: 字符串起始地址
 * @param         {uint8_t} size: 字体大小
 * @return        {*}
 ***************************************************************************/
void lcd_show_string(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size)
{
    while ((*chr >= ' ') && (*chr <= '~')) // 判断是不是非法字符!
    {
        lcd_show_chr(x, y, *chr, size);
        x += size / 2;
        if (x > 128 - size) // 换行
        {
            x = 0;
            y += 2;
        }
        chr++;
    }
}



void lcd_darw_init(void)
{
   lcd_init();
	 lcd_draw_refresh(); //清屏
}