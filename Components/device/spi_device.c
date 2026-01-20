#include "spi_device.h"
#include "w25qxx_driver.h"
#include <string.h>
#include "os_port.h"

#ifndef SYS_USING_HEAP
 struct flash_device_t flash_dev = {0};
#endif


struct spi_device_t *flash_device = NULL;

static int spi_device_init(struct device_t *pdev, void *config)
{
   struct spi_device_t *spi_dev = (struct spi_device_t *)pdev;
   if(spi_dev->id == FLASH_YTPE_W25QXX)
   {
       w25qxx_init();
       return 0;
   }
  return -1;
}


static int spi_device_deinit(struct device_t *pdev)
{
  return 0;
}


static int spi_device_read(struct device_t *pdev, void *exten_arg, uint8_t* buf, uint32_t size)
{
    struct spi_device_t *spi_dev = (struct spi_device_t *)pdev;
    if(spi_dev->id == FLASH_YTPE_W25QXX)
    {
        //W25QXX系列SPI Flash
        uint32_t addr = *((uint32_t *)exten_arg);
        w25qxx_read(addr,buf,size);
        return size;
    }
    return -1;
}

static int spi_device_write(struct device_t *pdev, void *exten_arg, uint8_t* buf, uint32_t size)
{
    struct spi_device_t *spi_dev = (struct spi_device_t *)pdev;
    if(spi_dev->id == FLASH_YTPE_W25QXX)
    {
        //W25QXX系列SPI Flash
        uint32_t addr = *((uint32_t *)exten_arg);
        w25qxx_write_with_erase(addr,buf,size);
        return size;
    }
      return -1;
}

struct device_t *spi_device_get(char *name)
{
    return (struct device_t *)flash_device; //目前只有一个flash设备
}


struct device_t *spi_device_create(const char *dev_name)  
{
    if(dev_name == NULL)
    {
        return NULL;
    }
    
    #if defined(SYS_USING_HEAP)
    struct spi_device_t *p_dev = (struct spi_device_t *)os_malloc(sizeof(struct spi_device_t));
	   memset(p_dev, 0, sizeof(struct spi_device_t));
#else
    p_dev = &flash_dev;
    flash_device = p_dev;
#endif
    if(strncmp(dev_name,W25QXX_DEV_NAME,strlen(W25QXX_DEV_NAME)) == 0)
    {
        //W25QXX系列SPI Flash
        p_dev->parent.type = DEVICE_TYPE_SPI;
        p_dev->parent.init = spi_device_init;
        p_dev->parent.deinit = spi_device_deinit;
        p_dev->parent.read = spi_device_read;
        p_dev->parent.write = spi_device_write;
        p_dev->parent.control = NULL;

        p_dev->id = FLASH_YTPE_W25QXX;
        int len = strlen(dev_name);
        if(len > (sizeof(p_dev->parent.name)-1))
        {
            len = sizeof(p_dev->parent.name)-1;
        }
        memcpy( p_dev->parent.name,dev_name,len);
        flash_device = p_dev;
        return (struct device_t *)p_dev;
    }
    else //GD32内置Flash
    {
        #if defined(SYS_USING_HEAP)
        os_free(p_dev);
        #endif
        return NULL;
    }
}