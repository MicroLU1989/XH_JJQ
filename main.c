
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
#include "rtc_driver.h"
#include "rtc_device.h"
#include "rfid.h"
#include "common_utils.h"
#include "buzzer.h"


uint32_t cnt = 0;

void test_task(void *param)
{
    time_s time = {26,1,22,9,57,27};
    struct device_t *rtc_dev = device_create(DEVICE_TYPE_RTC,"rtc");
    device_init(rtc_dev,NULL);
    device_control(rtc_dev,RTC_CMD_SET_TIME,&time);
    while(1)
    {
        device_read(rtc_dev,NULL,(void *)&time,sizeof(time_s));
        log_d("time = %d-%d-%d %d:%d:%d",time.year+2000,time.month,time.date,time.hour,time.min,time.sec);
        os_task_sleep(1000);
    }
}
void app_task_start(void)
{
    os_enter_critical();
    os_task_create("usbTask", usb_app_task, NULL, 1024, 1, 10); // USB任务要最先启动,优先级要最高,因为log调试用到USB
    os_task_create("keytsk", key_scan_task, NULL, 1024, 2, 10); // 按键扫描任务
    os_task_create("testtsk",  test_task, NULL, 1024, 3, 10);
    os_task_create("ledtsk", led_task, NULL, 1024, 4, 10);
    os_task_create("buzzertsk", buzzer_task, NULL, 1024, 5, 10);
    os_task_create("gpstsk", gps_task, NULL, 1024, 6, 10);      // GPS任务
    os_task_create("menutsk", menu_task, NULL, 1024, 7, 10);    // 菜单任务 
    os_task_create("rfidtsk", rfid_task, NULL, 1024, 8, 10);    // RFID任务
    os_exit_critical();
    os_sart_scheduler();
}

int main(void)
{
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    app_task_start();
}
