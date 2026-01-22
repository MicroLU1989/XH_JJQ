#include <stdlib.h>
#include "device.h"
#include "uart_device.h"
#include "usb_device.h"
#include "spi_device.h"
#include "i2c_device.h"
#include "rtc_device.h"


struct device_t *device_create(enum device_type dev_tpye, const char *dev_name)
{
    struct device_t *dev = NULL;
    switch (dev_tpye)
    {
    case DEVICE_TYPE_UART:
        dev = uart_device_create(dev_name);
		    break;
    case DEVICE_TYPE_USB:
        dev = usb_device_create(dev_name);
		    break;
    case DEVICE_TYPE_SPI:
        dev = spi_device_create(dev_name);
        break;
    case DEVICE_TYPE_I2C:
        dev = i2c_device_create(dev_name);
        break;
    case DEVICE_TYPE_RTC:
        dev = rtc_device_create(dev_name);
        break;
    default:
        return NULL;
    }
		if(dev == NULL)
		{
			while(1);
		}
		return dev;
}

struct device_t *device_get_obj(enum device_type dev_tpye, char *dev_name)
{
    struct device_t *dev = NULL;
    switch (dev_tpye)
    {
    case DEVICE_TYPE_UART:
        dev = uart_device_get(dev_name);
        break;

    case DEVICE_TYPE_USB:
        dev = usb_device_get(dev_name);
        break;

    case DEVICE_TYPE_SPI:
        dev = spi_device_get(dev_name);
        break;
    case DEVICE_TYPE_I2C:
        dev = i2c_device_get(dev_name);
        break;
    case DEVICE_TYPE_RTC:
        dev = rtc_device_get(dev_name);
        break;
    default:
        return NULL;
    }
    return dev;
}

int device_init(struct device_t *pdev, void *config)
{
    if (pdev->init)
    {
        return pdev->init(pdev, config);
    }
    return -1;
}

int device_deinit(struct device_t *pdev)
{
    if (pdev->deinit)
    {
        return pdev->deinit(pdev);
    }
    return -1;
}

int device_control(struct device_t *pdev, uint32_t cmd, void *arg)
{
    if (pdev->control)
    {
        return pdev->control(pdev, cmd, arg);
    }
    return -1;
}

int device_read(struct device_t *pdev,void *extern_arg, uint8_t *buf, uint32_t size)
{
    if (pdev->read)
    {
        return pdev->read(pdev,extern_arg, buf, size);
    }
    return -1;
}

int device_write(struct device_t *pdev,void *extern_arg, uint8_t *buf, uint32_t size)
{
    if (pdev->write)
    {
        return pdev->write(pdev,extern_arg, buf, size);
    }
    return -1;
}
