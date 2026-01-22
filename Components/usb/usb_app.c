#include "usb_device.h"
#include "usb_app.h"
#include <stdlib.h>
#include "os_port.h"
#include "my_queue.h"
#include <drv_usb_hw.h>
#include "cdc_acm_core.h"
#include "user_log.h"

extern usb_core_driver cdc_acm;
static uint8_t tx_buf[64];
void usb_app_task(void *param)
{
	struct device_t *dev = device_create(DEVICE_TYPE_USB, "USB0");
	struct usb_config_t cfg = {1024, 1024};
	device_init(dev, &cfg);
	struct usb_device_t *udev = (struct usb_device_t *)dev;
	usb_cdc_handler *cdc = (usb_cdc_handler *)cdc_acm.dev.class_data[CDC_COM_INTERFACE];
	while (1)
	{
		if (USBD_CONFIGURED == cdc_acm.dev.cur_status)
		{
			if (0U == cdc_acm_check_ready(&cdc_acm))
			{
				cdc_acm_data_receive(&cdc_acm);
				queue_push_len(udev->rx_queue, cdc->data, cdc->receive_length); // 将接收到的数据存入队列
			}
			else
			{
				while (queue_is_empty(udev->tx_queue) == false)
				{
					uint32_t i = 0;
					memset(tx_buf, 0, sizeof(tx_buf));
					for (; i < sizeof(tx_buf) - 1; i++)
					{
						if (queue_pop(udev->tx_queue, &tx_buf[i]) == false)
						{
							break;
						}
					}
					usbd_ep_send(&cdc_acm, CDC_DATA_IN_EP, (uint8_t *)tx_buf, i);
					cdc->receive_length = 0;
					os_task_sleep(5);
				}
			}
		}
		os_task_sleep(5);
	}
}
