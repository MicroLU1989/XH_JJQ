#ifndef _DEVICE_H_
#define _DEVICE_H_


#include <stdint.h>



struct device_t
{
   uint32_t type;           //设备类型
   uint32_t status;         //设备状态
   void* private_data;      //私有数据
   char name[32];           //设备名称
   int (*init)(struct device_t * pdev,void *config);
   int (*deinit)(struct device_t *pdev);
   int (*control)(struct device_t *pdev, uint32_t cmd, void* arg);
   int (*write)(struct device_t *pdev,void *extern_arg, uint8_t* buf, uint32_t size);
   int (*read)(struct device_t *pddev,void *extern_arg, uint8_t* buf, uint32_t size);
   void *write_sem;
   void *read_sem;
};

enum device_type
{
    DEVICE_TYPE_UNKNOWN,
    DEVICE_TYPE_SPI,
    DEVICE_TYPE_I2C,
    DEVICE_TYPE_UART,
	DEVICE_TYPE_USB,
};

enum device_status
{
    DEVICE_STATUS_UNKNOWN,
    DEVICE_STATUS_READY,
    DEVICE_STATUS_BUSY,
    DEVICE_STATUS_ERROR,
};


int device_init(struct device_t* pdev,void *config);
int device_deinit(struct device_t* pdev);
int device_control(struct device_t* pdev, uint32_t cmd, void* arg);
int device_write(struct device_t*pdev,void *extern_arg, uint8_t* buf, uint32_t size);
int device_read(struct device_t* pdev,void *extern_arg, uint8_t* buf, uint32_t size);
struct device_t * device_create(enum device_type dev_tpye, const char *dev_name);

struct device_t *device_get_obj(enum device_type dev_tpye, char *dev_name);

#endif