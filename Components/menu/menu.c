#include "lcd_draw.h"
#include "lcd_st756x_driver.h"
#include "os_port.h"
#include "user_log.h"
#include "my_linklist.h"

#define MENU_PAGE_NUM 4

static uint8_t menu_page_index = 0;

typedef void (*menu_page_ptr)(void);

// 菜单1,主菜单.
static void menu_1_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 0, (uint8_t *)&"2026-01-21 12:34:56", 12);
	lcd_show_string(24, 16, (uint8_t *)&"Bienvenue", 12);
	lcd_show_string(30, 32, (uint8_t *)&"OLE TOGO",12);
	lcd_show_string(0, 48, (uint8_t *)&"T:T:T        v3.1.5",12);
	// log_d("菜单1绘制完成");
}

// 菜单计价器界面
static void menu_2_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 0, (uint8_t *)&"2025-11", 12);
	// log_d("菜单2绘制完成");
}

// 菜单3 刷卡界面
static void menu_3_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 0, (uint8_t *)&"menu 003", 12);
	// log_d("菜单3绘制完成");
}

// 菜单4 信息界面
static void menu_4_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 0, (uint8_t *)&"menu 004", 12);
	// log_d("菜单4绘制完成");
}

static menu_page_ptr menu_list[MENU_PAGE_NUM] =
	{
		menu_1_dis,
		menu_2_dis,
		menu_3_dis,
		menu_4_dis,
};

void menu_key_press_handle(uint8_t key_id)
{
	if (key_id == 0)
	{
		// 向下翻页：如果当前页是最后一页，则回到第一页；否则下一页
		if (menu_page_index >= MENU_PAGE_NUM - 1)
		{
			menu_page_index = 0;
		}
		else
		{
			menu_page_index++;
		}
	}
	else if (key_id == 1)
	{
		// 向上翻页：如果当前页是第一页，则跳到最后一页；否则上一页
		if (menu_page_index == 0)
		{
			menu_page_index = MENU_PAGE_NUM - 1;
		}
		else
		{
			menu_page_index--;
		}
	}
	log_d("menu_page_index = %d", menu_page_index);
}

void test_contrast(void)
{
	uint8_t contrast_values[] = {0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
	// uint8_t contrast_values[] = {0x10, , 0x20, 0x28, 0x30, 0x38};
	for (uint8_t i = 0x10; i < 0x38; i++)
	{
		// lcd_set_contrast(contrast_values[i]);
		// lcd_set_contrast(0x11);
		//  log_d("亮度== 0x%x",i);
		// 绘制测试图形
		lcd_clear_screen();
		os_task_sleep(2000);
		// lcd_draw_line(0, 0, 127, 63);     // 对角线
		// lcd_draw_line(0, 63, 127, 0);   	 // 反对角线
		// lcd_draw_line(0, 31, 127, 31);  	 // 水平中线
		// lcd_draw_line(63, 0, 63, 63);   	 // 垂直中线
		// lcd_draw_line(30, 0, 30, 63);     // 垂直中线
		// lcd_draw_circle(30,32,25);
		// lcd_show_icon(0,0,ICON_BT);
		lcd_show_string(0, 16, (uint8_t *)&"2026-1-15 10:36:30", 12);
		lcd_draw_refresh();
		os_task_sleep(3000);
	}
}

static void menu_init(void)
{
	lcd_darw_init();
}

void menu_task(void *param)
{
	menu_init();
	while (1)
	{
		menu_list[menu_page_index]();
		lcd_draw_refresh();
		os_task_sleep(100);
	}
}
