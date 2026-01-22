#include "i2c_device.h"
#include "i2c_driver.h"

#include <string.h>
#include "os_port.h"

#ifndef SYS_USING_HEAP
 struct i2c_device_t i2c_dev_instance = {0};
#endif

struct i2c_device_t *i2c_device = NULL;

static int i2c_device_init(struct device_t *pdev, void *config)
{
   if (pdev == NULL || config == NULL)
   {
      return -1;
   }
   struct i2c_device_t *i2c_dev = (struct i2c_device_t *)pdev;
   struct i2c_device_cfg_t *i2c_cfg = (struct i2c_device_cfg_t *)config;
   i2c_adapter_t *adp = (i2c_adapter_t *)i2c_dev->i2c_handle;
   adp->dev_addr = i2c_cfg->dev_addr;
   adp->i2c_speed = i2c_cfg->i2c_speed;
   adp->scl_pin_number = i2c_cfg->scl_pin_num;
   adp->scl_port = i2c_cfg->scl_port_num;
   adp->sda_pin_number = i2c_cfg->sda_pin_num;
   adp->sda_port = i2c_cfg->sda_port_num;
   i2c_init(adp);
  return 0;
}


static int i2c_device_deinit(struct device_t *pdev)
{
   return 0;
}


static int i2c_device_read(struct device_t *pdev, void *exten_arg, uint8_t* buf, uint32_t size)
{
    if(buf == NULL || size == 0 || pdev == NULL || exten_arg == NULL)
    {
        return -1;
    }
    struct i2c_device_t *i2c_dev = (struct i2c_device_t *)pdev;
    i2c_read_reg((i2c_adapter_t *)i2c_dev->i2c_handle, *((uint8_t*)exten_arg), buf, size);
    return size;
}

static int i2c_device_write(struct device_t *pdev, void *exten_arg, uint8_t* buf, uint32_t size)
{
    if(buf == NULL || size == 0 || pdev == NULL || exten_arg == NULL)
    {
        return -1;
    }
    struct i2c_device_t *i2c_dev = (struct i2c_device_t *)pdev;
    i2c_write_reg((i2c_adapter_t *)i2c_dev->i2c_handle, *((uint8_t*)exten_arg), buf, size);
    return size;
}

struct device_t *i2c_device_get(char *name)
{
    return (struct device_t *)i2c_device; //目前只有一个flash设备
}


struct device_t *i2c_device_create(const char *dev_name)  
{
    if(dev_name == NULL)
    {
        return NULL;
    }
    
#if defined(SYS_USING_HEAP)
    struct i2c_device_t *p_dev = (struct i2c_device_t *)os_malloc(sizeof(struct i2c_device_t));
	   memset(p_dev, 0, sizeof(struct i2c_device_t));
    p_dev->i2c_handle = os_malloc(sizeof(i2c_adapter_t));   
#else
    p_dev = i2c_dev_instance;
    static i2c_adapter_t i2c_adp;
    p_dev->i2c_handle = &i2c_adp;
#endif
        p_dev->parent.type = DEVICE_TYPE_I2C;
        p_dev->parent.init = i2c_device_init;
        p_dev->parent.deinit = i2c_device_deinit;
        p_dev->parent.read = i2c_device_read;
        p_dev->parent.write = i2c_device_write;
        p_dev->parent.control = NULL;
        int len = strlen(dev_name);
        if(len > (sizeof(p_dev->parent.name)-1))
        {
            len = sizeof(p_dev->parent.name)-1;
        }
        memcpy( p_dev->parent.name,dev_name,len);
        i2c_device = p_dev;
        return (struct device_t *)p_dev;
}
