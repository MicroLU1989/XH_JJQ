#include "usb_device.h"
#include <string.h>
#include <stdlib.h>
#include <drv_usb_hw.h>
#include "cdc_acm_core.h"
#include "os_port.h"
#include "my_queue.h"

usb_core_driver cdc_acm;

struct usb_device_t *usb_device = NULL;

static int total_size = 0;

static int usb_device_init(struct device_t *pdev, void *config)
{
    usb_gpio_config();
    usb_rcu_config();
    usb_timer_init();
    usbd_init(&cdc_acm, &cdc_desc, &cdc_class);
    usb_intr_config();
	  while(USBD_CONFIGURED != cdc_acm.dev.cur_status);
	  struct usb_config_t *cfg = (struct usb_config_t *)config;
	  struct usb_device_t *dev = (struct usb_device_t *)pdev;
	  if(cfg != NULL && dev != NULL)
		{
			  dev->rx_queue =  queue_create(cfg->rx_queue_size,1);
	      dev->tx_queue =  queue_create(cfg->tx_queue_size,1);
		}
		usb_device->rx_sem = os_sem_create("usbrxsem");
		usb_device->tx_sem = os_sem_create("usbtxsem");
	  return 0;
}

static int usb_device_deinit(struct device_t *pdev)
{
    //usbd_deinit(&cdc_acm);
    return 0;
}

static int usb_device_control(struct device_t *pdev, uint32_t cmd, void *arg)
{
    return 0;
}



static int usb_device_read(struct device_t *pdev, void *extern_arg,uint8_t *buf, uint32_t size)
{
    if(pdev == NULL || buf == NULL || size == 0)
		return -1;
	struct usb_device_t *dev = (struct usb_device_t *)pdev;
	uint32_t read_size = 0;
	uint8_t data;
	while ((read_size < size) && !queue_is_empty(dev->rx_queue))
	{
		queue_pop(dev->rx_queue, &data);
		buf[read_size++] = data;
	}
	return read_size;
}

static int usb_device_write(struct device_t *pdev,void *extern_arg, uint8_t *buf, uint32_t size)
{
	struct usb_device_t *dev = (struct usb_device_t *)pdev;
    queue_push_len(dev->tx_queue,buf,size);
    return size;
}



struct device_t *usb_device_get(char *name)
{
    return (struct device_t *)usb_device;
}


struct device_t *usb_device_create(const char *name)
{
    usb_device = (struct usb_device_t *)os_malloc(sizeof(struct usb_device_t));
    memset(usb_device, 0, sizeof(struct usb_device_t));
    usb_device->parent.type = DEVICE_TYPE_USB;
    usb_device->parent.init = usb_device_init;
    usb_device->parent.deinit = usb_device_deinit;
    usb_device->parent.control = usb_device_control;
    usb_device->parent.read = usb_device_read;
    usb_device->parent.write = usb_device_write;
    usb_device->parent.status = DEVICE_STATUS_UNKNOWN;
	  usb_device->usb_hdl = &cdc_acm;
    return (struct device_t *)usb_device;
}


void usb_sem_rx_release(void)
{
	if(usb_device && usb_device->rx_sem)
	{
	  int flg = 0;
     os_sem_release_from_isr(usb_device->rx_sem,&flg);
	}
}

void usb_sem_tx_release(void)
{
	if(usb_device && usb_device->rx_sem)
	{
		 int flg = 0;
	   os_sem_release_from_isr(usb_device->rx_sem,&flg);
	}
}

