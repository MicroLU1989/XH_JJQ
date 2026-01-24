#include "gpio_driver.h"
#include "systick.h"
#include "gd32f30x.h"





#define LCD_GPIO_PORT           GPIO_PORT_B
#define LCD_RES_PIN_NUM         GPIO_PIN_NUM(4)
#define LCD_CS_PIN_NUM          GPIO_PIN_NUM(3)
#define LCD_A0_PIN_NUM          GPIO_PIN_NUM(5)
#define LCD_SCL_PIN_NUM         GPIO_PIN_NUM(6)  
#define LCD_SDA_PIN_NUM         GPIO_PIN_NUM(7)
#define LCD_BACKLIGHT_PIN_NUM   GPIO_PIN_NUM(8)

#define LCD_PWR_PORT            GPIO_PORT_C
#define LCD_PWR_PIN_NUM         GPIO_PIN_NUM(8)


#define LCD_RES_HIGH   drv_gpio_write(LCD_GPIO_PORT, LCD_RES_PIN_NUM, 1)
#define LCD_RES_LOW    drv_gpio_write(LCD_GPIO_PORT, LCD_RES_PIN_NUM, 0)

#define LCD_CS_HIGH    drv_gpio_write(LCD_GPIO_PORT, LCD_CS_PIN_NUM, 1)
#define LCD_CS_LOW     drv_gpio_write(LCD_GPIO_PORT, LCD_CS_PIN_NUM, 0)

#define LCD_A0_HIGH    drv_gpio_write(LCD_GPIO_PORT, LCD_A0_PIN_NUM, 1)
#define LCD_A0_LOW     drv_gpio_write(LCD_GPIO_PORT, LCD_A0_PIN_NUM, 0)

#define LCD_SCL_HIGH   drv_gpio_write(LCD_GPIO_PORT, LCD_SCL_PIN_NUM, 1)
#define LCD_SCL_LOW    drv_gpio_write(LCD_GPIO_PORT, LCD_SCL_PIN_NUM, 0)

#define LCD_SDA_HIGH   drv_gpio_write(LCD_GPIO_PORT, LCD_SDA_PIN_NUM, 1)
#define LCD_SDA_LOW    drv_gpio_write(LCD_GPIO_PORT, LCD_SDA_PIN_NUM, 0)

#define LCD_BACKLIGHT_ON  drv_gpio_write(LCD_GPIO_PORT, LCD_BACKLIGHT_PIN_NUM, 1)
#define LCD_BACKLIGHT_OFF drv_gpio_write(LCD_GPIO_PORT, LCD_BACKLIGHT_PIN_NUM, 0)


#define LCD_PWR_ON  drv_gpio_write(LCD_PWR_PORT, LCD_PWR_PIN_NUM, 1)
#define LCD_PWR_OFF drv_gpio_write(LCD_PWR_PORT, LCD_PWR_PIN_NUM, 0)


#if 1
static uint8_t lcd_init_cmd[] = 
{
    0xA2, //1/9 Bias 
    0xA6, //Set Normal display
    0xA0, //ADC set (SEG)
    0xC8, //COM reverse
    0x24, //Set COM output scan direction
    0x81, //Electronic Volume Mode Set
    0x15, //
    0x2C, //
    0x2E, //
    0x2F, //The Power Control Set
    0xAF  //Lcd Display ON
};
#else
static uint8_t lcd_init_cmd[] = 
{
   0xA2,
   0xA0,
   0xC8,
   0x24,
   0x81,
   0x1D,
   0x2C,
   0x2E,
   0x2F,
   0xF8,
   0xAF
};
#endif


void lcd_gpio_init(void)
{
	
	
			rcu_periph_clock_enable(RCU_AF);//禁用JTAG引脚功能，操作的是AF寄存器，所以要先使能AF时钟
		rcu_periph_clock_enable(RCU_GPIOB);
		gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	
	
    drv_gpio_init(LCD_GPIO_PORT,PIN_MODE_OUT_PP | LCD_RES_PIN_NUM | LCD_CS_PIN_NUM | LCD_A0_PIN_NUM | LCD_SCL_PIN_NUM | LCD_SDA_PIN_NUM | LCD_BACKLIGHT_PIN_NUM, 
                  PIN_MODE_OUT_PP, PIN_SPEED_10M);
	
	  drv_gpio_init(LCD_PWR_PORT, LCD_PWR_PIN_NUM, 
                  PIN_MODE_OUT_PP, PIN_SPEED_10M);
	
		LCD_PWR_ON;
		LCD_SDA_HIGH;
		LCD_CS_HIGH;   
		LCD_SCL_HIGH;	
		LCD_A0_HIGH;
		LCD_RES_HIGH;

}

void lcd_write_cmd(uint8_t cmd)
{ 
    LCD_CS_LOW;
    LCD_A0_LOW;
    int j = 8;
    do 
    {
        if(cmd & 0x80)
            LCD_SDA_HIGH;
        else
            LCD_SDA_LOW;
        LCD_SCL_LOW;
        LCD_SCL_HIGH;
        --j;
        cmd <<= 1;
    } while(j);
		LCD_CS_HIGH;
		LCD_A0_HIGH;
}

void lcd_write_data(uint8_t data)
{ 
    LCD_CS_LOW;
    LCD_A0_HIGH;
    int j = 8;
    do 
    {
        if(data & 0x80)
            LCD_SDA_HIGH;
        else
            LCD_SDA_LOW;
        LCD_SCL_LOW;
        LCD_SCL_HIGH;
        --j;
        data <<= 1;
    } while(j); 
		LCD_CS_HIGH;
}




void lcd_init(void)
{ 
    lcd_gpio_init();
	  delay_1ms(10);
  LCD_RES_LOW;
						delay_1ms(10);
						LCD_RES_HIGH;
						delay_1ms(10);

  //lcd_write_cmd(0xE2);												/* 软复位 */
	delay_1ms(5);
	lcd_write_cmd(0xA2);	                      /* 偏压比（bias）,0xA2:1/9   0xA3:1=1/7  */
	lcd_write_cmd(0xA6);	                      /* 正常显示 */
	lcd_write_cmd(0xA0);	                      /* 列扫描顺序：从左到右 */
	lcd_write_cmd(0xC8);	                      /* 行扫描顺序：从上到下 */
	
	lcd_write_cmd(0x23);			  								/* 调整对比度，设置范围0x20～0x27 */
	lcd_write_cmd(0x81);												/* 微调对比度 */
	lcd_write_cmd(0x30);												/* 微调对比度的值 */
	
	lcd_write_cmd(0x2C);											  /* 升压步骤1 */

	delay_1ms(50);
	lcd_write_cmd(0x2E);												/* 升压步骤2 */
	delay_1ms(50);
	lcd_write_cmd(0x2F);												/* 升压步骤3 */
	delay_1ms(50);
	
	
	lcd_write_cmd(0xA2);	                      /* 偏压比（bias）,0xA2:1/9   0xA3:1=1/7  */
	delay_1ms(50);
	lcd_write_cmd(0xAF);	                      /* 开显示 */
	delay_1ms(50);
	lcd_write_cmd(0xA6);	                      /* 正常显示 */
	lcd_write_cmd(0xA4);	                      /* 全部点阵打开	*/
	lcd_write_cmd(0x40);	                      /* 起始行：第一行开始 */	

	LCD_BACKLIGHT_ON;
}

