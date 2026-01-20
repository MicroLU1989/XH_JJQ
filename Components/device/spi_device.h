#ifndef     _SPI_DEVICE_H_
#define     _SPI_DEVICE_H_ 

#include  <stdint.h>
#include "device.h"


#define W25QXX_DEV_NAME      "w25qxx"
#define GD32_FLASH_DEV_NAME  "gd32_flash"


enum FLASH_TYPE_ID
{
    FLASH_YTPE_W25QXX = 0,   //W25QXX系列SPI Flash
    FLASH_YTPE_GD32  = 1,    //GD32内置Flash
};


struct spi_device_t
{
    struct device_t parent;
    enum FLASH_TYPE_ID id;
};

struct device_t *spi_device_get(char *name);
struct device_t *spi_device_create(const char *dev_name);
#endif