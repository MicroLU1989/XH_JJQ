#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_ 


#include "device.h"


struct i2c_device_t
{
    struct device_t parent;
    void *i2c_handle;
};

struct i2c_device_cfg_t
{
    uint8_t  dev_addr;          //IIC设备地址
    uint32_t i2c_speed;         //IIC速度
    uint32_t sda_port_num;      //SDA引脚端口编号
    uint32_t scl_port_num;      //SCL引脚端口编号
    uint32_t sda_pin_num;       //SDA引脚编号
    uint32_t scl_pin_num;       //SCL引脚编号
 
};


extern struct device_t *i2c_device_get(char *name);
extern struct device_t *i2c_device_create(const char *dev_name);
#endif