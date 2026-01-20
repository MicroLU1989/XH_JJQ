#include "lcd_draw.h"
#include "lcd_st756x_driver.h"
#include "os_port.h"
#include "user_log.h"
#include "my_linklist.h"

linklist_t *menu_list = NULL; // 菜单项链表

struct menu_item_t
{
	char *name;
	void (*dis_func)(void);		// 绘制菜单项
	void (*refresh_func)(void); // 刷新菜单项
};

// 菜单1,主菜单
static void menu_1_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 0, (uint8_t *)&"menu 001", 12);
	log_d("菜单1绘制完成");
}

static void menu_1_refresh(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 8, (uint8_t *)&"2026-1-15 10:36:30", 12);

}

// 菜单计价器界面
static void menu_2_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 16, (uint8_t *)&"2026-1-15 10:36:02", 12);
	log_d("菜单2绘制完成");
}

static void menu_2_refresh(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 8, (uint8_t *)&"2026-1-15 10:36:30", 12);
}

// 菜单3 刷卡界面
static void menu_3_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 16, (uint8_t *)&"2026-1-15 10:36:03", 12);
	log_d("菜单3绘制完成");
}

static void menu_3_refresh(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 8, (uint8_t *)&"2026-1-15 10:36:30", 12);

}

// 菜单4 信息界面
static void menu_4_dis(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 16, (uint8_t *)&"2026-1-15 10:36:04", 12);
	log_d("菜单4绘制完成");
}

static void menu_4_refresh(void)
{
	lcd_clear_screen();
	lcd_show_string(0, 8, (uint8_t *)&"2026-1-15 10:36:30", 12);
}

void menu_add_item(char *name, void (*dis_func)(void), void (*refresh_func)(void))
{
	struct menu_item_t *item = (struct menu_item_t *)os_malloc(sizeof(struct menu_item_t));
	linklist_t *node = (linklist_t *)os_malloc(sizeof(linklist_t));
	item->name = os_malloc(strlen(name) + 1);
    if(item == NULL || node == NULL || item->name == NULL)
	{
		log_e("内存分配失败");
		return;
	}
	memset(item->name, 0, strlen(name) + 1);
	strcpy(item->name, name);
	item->dis_func = dis_func;
	item->refresh_func = refresh_func;
	node->data = item;
	linklist_insert_after(menu_list,node);
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
		os_task_mdelay(2000);
		// lcd_draw_line(0, 0, 127, 63);     // 对角线
		// lcd_draw_line(0, 63, 127, 0);   	 // 反对角线
		// lcd_draw_line(0, 31, 127, 31);  	 // 水平中线
		// lcd_draw_line(63, 0, 63, 63);   	 // 垂直中线
		// lcd_draw_line(30, 0, 30, 63);     // 垂直中线
		// lcd_draw_circle(30,32,25);
		// lcd_show_icon(0,0,ICON_BT);
		lcd_show_string(0, 16, (uint8_t *)&"2026-1-15 10:36:30", 12);
		lcd_draw_refresh();
		os_task_mdelay(3000);
	}
}

static void menu_init(void)
{
	menu_list = linklist_create();
	lcd_darw_init();
	menu_add_item("menu_1", menu_1_dis, menu_1_refresh);
	menu_add_item("menu_2", menu_2_dis, menu_2_refresh);
	menu_add_item("menu_3", menu_3_dis, menu_3_refresh);
	menu_add_item("menu_4", menu_4_dis, menu_4_refresh);
}

void menu_task(void *param)
{
	menu_init();
	os_task_mdelay(1000);
	while (1)
	{
		//test_contrast();
		// 循环绘制菜单项
		linklist_t *item = menu_list->next;
        while (item != NULL)
		{
			struct menu_item_t *menu_item = (struct menu_item_t *)item->data;
			menu_item->dis_func();
		    lcd_draw_refresh();
			item = item->next;
			os_task_mdelay(2000);
		}
	}
}
