
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
//#include "w25qxx_driver.h"
#include "rfid.h"


void app_task_start(void)
{
    os_enter_critical();
    os_task_create("usbTask", usb_app_task, NULL, 1024, 1, 10); // USB任务要最先启动,优先级要最高,因为log调试用到USB
    os_task_create("keytsk", key_scan_task, NULL, 1024, 2, 10); // 按键扫描任务
    //os_task_create("task2",  task2, NULL, 1024, 3, 10);
    os_task_create("ledtsk", led_task, NULL, 1024, 4, 10);
    //os_task_create("task3", task3, NULL, 1024, 5, 10);
    os_task_create("gpstsk", gps_task, NULL, 1024, 6, 10);      // GPS任务
    os_task_create("menutsk", menu_task, NULL, 1024, 7, 10);    // 菜单任务 
   // os_task_create("rfidtsk", rfid_task, NULL, 1024, 8, 10);    // RFID任务
    os_exit_critical();
    os_sart_scheduler();
}

int main(void)
{
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    app_task_start();
}
