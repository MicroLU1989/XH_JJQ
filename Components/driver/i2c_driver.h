#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_ 


#include <stdint.h>

typedef struct  
{
   uint8_t  dev_addr; //7bit address I2c设备地址
   uint32_t sda_port;
   uint32_t scl_port;
   uint32_t sda_pin_number;
   uint32_t scl_pin_number;
   uint32_t delay_us;
   uint32_t i2c_speed;  // 新增：I2C速度，单位Hz
}i2c_adapter_t;



extern void i2c_init(i2c_adapter_t *i2c_adp);

extern uint8_t i2c_read_reg(i2c_adapter_t *i2c_adp, uint8_t reg_addr, uint8_t *buf, uint32_t read_len);
extern void i2c_write_reg(i2c_adapter_t* i2c_adp, uint8_t reg_addr, uint8_t *data_buf, uint32_t write_len);


#endif