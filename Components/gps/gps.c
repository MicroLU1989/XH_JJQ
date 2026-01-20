#include "gps.h"
#include "uart_device.h"
#include "os_port.h"
#include "gpio_driver.h"
#include "user_log.h"

#define GPS_PWR_PORT GPIO_PORT_C
#define GPS_PWR_PIN GPIO_PIN_NUM(4)

static uint8_t gps_data_buf[128];

static void gps_pwr_gpio_init(void)
{
   drv_gpio_init(GPS_PWR_PORT, GPS_PWR_PIN, PIN_MODE_OUT_PP, PIN_SPEED_2M);
}

static void gps_pwr_on(void)
{
   drv_gpio_write(GPS_PWR_PORT, GPS_PWR_PIN, 1);
}

static void gps_pwr_off(void)
{
   drv_gpio_write(GPS_PWR_PORT, GPS_PWR_PIN, 0);
}





void gps_task(void *param)
{
   uint8_t ch = 0, last_ch = 0;
   uint8_t index = 0;
   gps_pwr_gpio_init();
   gps_pwr_on();
   struct device_t *uart_dev = device_create(DEVICE_TYPE_UART, "gpsuart");
   struct uart_config_t uart_cfg = {
       .baudrate = 115200,
       .id = UART_1,
       .rx_size = 512,
   };
   device_init(uart_dev, &uart_cfg);
   while (1)
   {
      int len = device_read(uart_dev, NULL, &ch, 1);
      if (len > 0 && index < sizeof(gps_data_buf) - 1)
      {
         gps_data_buf[index++] = ch;
         if (ch == '\n' && last_ch == '\r') // 收到完整的一帧数据
         {
            gps_data_buf[index] = '\0'; // 字符串结束符
            // log_d("GPS Data: %s", gps_data_buf);
            index = 0; // 重置索引，准备接收下一帧数据
                       // 解析数据

            /// 发送数据测试
            // int ret = device_write(uart_dev,(uint8_t *)"123",3);
            // log_d("send = %d",ret);
         }
         else
         {
            last_ch = ch;
         }
      }
      else
      {
         os_task_mdelay(10);
         continue;
      }
   }
}