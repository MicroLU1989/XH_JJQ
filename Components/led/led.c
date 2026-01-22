#include "led.h"
#include "os_port.h"
#include "gpio_driver.h"

#define GPS_PIN_NUM GPIO_PIN_NUM(0)
#define GPS_PIN_PORT GPIO_PORT_C

#define GSM_PIN_NUM  GPIO_PIN_NUM(1)
#define GSM_PIN_PORT GPIO_PORT_C

void led_task(void *param)
{
	drv_gpio_init(GPS_PIN_PORT, GPS_PIN_NUM, PIN_MODE_OUT_PP, PIN_SPEED_2M);
	drv_gpio_init(GSM_PIN_PORT, GSM_PIN_NUM, PIN_MODE_OUT_PP, PIN_SPEED_2M);
	while (1)
	{
		os_task_sleep(250);
		drv_gpio_write(GPS_PIN_PORT, GPS_PIN_NUM, 0);
		drv_gpio_write(GSM_PIN_PORT, GSM_PIN_NUM, 0);
		os_task_sleep(250);
		drv_gpio_write(GPS_PIN_PORT, GPS_PIN_NUM, 1);
		drv_gpio_write(GSM_PIN_PORT, GSM_PIN_NUM, 1);
	}
}