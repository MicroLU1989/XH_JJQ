#ifndef _USB_CDC_DEVICE_H_
#define _USB_CDC_DEVICE_H_

#include "device.h"


struct usb_device_t
{
    struct device_t parent;
    uint32_t id;
	  void *usb_hdl;
    void *tx_sem;
    void *rx_sem;
	  void *tx_queue;
	  void *rx_queue;
};


struct usb_config_t
{
    uint16_t  tx_queue_size;
	  uint16_t  rx_queue_size;
};


extern struct device_t *usb_device_get(char *name);
extern struct device_t *usb_device_create(const char *name);
extern void usb_sem_rx_release(void);  //usb接收1包数据信号量
extern void usb_sem_tx_release(void);  //usb发送1包数据信号量
#endif