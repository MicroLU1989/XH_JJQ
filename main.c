
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f30x_misc.h"
#include "os_port.h"
#include "usb_app.h"
#include "user_log.h"
#include "led.h"
#include "gps.h"
#include "menu.h"
#include "key.h"
// #include "w25qxx_driver.h"
#include "rfid.h"

void *USBTask_Handler;
void *Task1_Handler;
void *Task2_Handler;

void *sem = NULL;

static int tick = 0;
static int tick2 = 0, tick3 = 0;
static int flash_test_done = 0; // 标记Flash测试是否已完成

void task1(void *pvParameters)
{
    // w25qxx_init();
    uint8_t id_buf[3] = {0};
    uint8_t devid = 0;
    // 标记基本测试已完成
    flash_test_done = 1;
    // w25qxx_read_write_test(1);
    while (1)
    {
        os_task_mdelay(10);
        tick++;
        if (tick == 100)
        {
            tick = 0;
            // w25qxx_read_write_test(0);
            //  周期性读取主要ID用于监控
            // w25qxx_read_id(id_buf);
            // log_d("W25Qxx ID: %02X %02X %02X\n", id_buf[0], id_buf[1], id_buf[2]);

            os_sem_release(sem);
        }
    }
}

void task2(void *pvParameters)
{
    size_t heap_size = 0;
    while (1)
    {
        os_sem_take(sem, OS_MAX_DELAY);
        tick2++;
        // log_d("tick2 = %d\n", tick2);
    }
}

void task3(void *pvParameters)
{
    while (1)
    {
        os_task_mdelay(100);
        tick3++;
        // log_d("tick3 = %d\n", tick3);
    }
}

void app_task_start(void)
{
    os_enter_critical();
    os_task_create("usbTask", usb_app_task, NULL, 1024, 1, 10); // USB任务要最先启动,优先级要最高,因为log调试用到USB
    os_task_create("keytsk", key_scan_task, NULL, 1024, 2, 10); // 按键扫描任务
    os_task_create("task2",  task2, NULL, 1024, 3, 10);
    os_task_create("ledtsk", led_task, NULL, 1024, 4, 10);
    os_task_create("task3", task3, NULL, 1024, 5, 10);
    os_task_create("gpstsk", gps_task, NULL, 1024, 6, 10);      // GPS任务
    os_task_create("menutsk", menu_task, NULL, 1024, 7, 10);    // 菜单任务 
    os_task_create("rfidtsk", rfid_task, NULL, 1024, 8, 10);    // RFID任务
    sem = os_sem_create("sem");
    os_exit_critical();
    os_sart_scheduler();
}

int main(void)
{
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    app_task_start();
    while (1)
    {
        delay_1ms(500);
    }
}
