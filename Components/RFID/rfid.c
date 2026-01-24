#include "rfid.h"
#include "os_port.h"
#include "rfid_si522_driver.h"
#include "user_log.h"

void rfid_task(void *param)
{
    log_d("开始初始化RFID");
	  os_task_sleep(100);
    rfid_si522_init();
     while (1)
    {
        Test_Si522_GetUID(); // 读ISO14443_4A卡
        //extern void si522_write_reg(uint8_t reg_addr,uint8_t data);
        //si522_write_reg(0x01,0x0f); //测试I2C;
			  //uint8_t val = si522_read_reg(0x37);
			  //log_d("0x37寄存器数据 = 0x%02x",val);
        os_task_sleep(100);
    }
}
