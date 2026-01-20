#ifndef  _W25QXX_DRIVER_H_
#define  _W25QXX_DRIVER_H_ 

#include  <stdint.h>


/*W25QXX 闪存芯片ID*/
#define W25Q80  0XEF13
#define W25Q16  0XEF14
#define W25Q32	0XEF15
#define W25Q64	0XEF16


/*W25QXX 闪存芯片指令表*/
#define W25X_WriteEnable					0x06   //写使能      
#define W25X_WriteDisable					0x04   //写失能
#define W25X_ReadStatusReg				0x05   //读状态寄存器
#define W25X_WriteStatusReg				0x01   //写状态寄存器
#define W25X_ReadData					    0x03   //读数据
#define W25X_FastReadData					0x0B   //快速读取数据
#define W25X_FastReadDual					0x3B
#define W25X_PageProgram					0x02   //页编程
#define W25X_BlockErase						0xD8   //块擦除
#define W25X_SectorErase					0x20   //扇区擦除
#define W25X_ChipErase						0xC7   //整片擦除
#define W25X_PowerDown						0xB9   //关闭FLASH
#define W25X_ReleasePowerDown			0xAB   //唤醒FLASH
#define W25X_DeviceID						  0xAB   //器件ID
#define W25X_ManufactDeviceID			0x90   //产品ID
#define W25X_JedecDeviceID			  0x9F   //JEDEC ID


//常用的容量定义
#define W25QXX_PAGE_SIZE                    256
#define W25QXX_SECTOR_SIZE                  4096
#define W25QXX_BLOCK_32K_SIZE               32768
#define W25QXX_BLOCK_64K_SIZE               65536


extern void w25qxx_init(void);
extern void w25qxx_read(uint32_t addr, uint8_t *buf, uint32_t size);
extern void w25qxx_write(uint32_t addr, uint8_t* data_buf, uint32_t size);
extern void w25qxx_sector_erase(uint32_t addr);
extern void w25qxx_chip_erase(void);
extern void w25qxx_write_with_erase(uint32_t addr, uint8_t *buf, uint32_t size);
extern void w25qxx_read_id(uint8_t* id_buf);
// extern void w25qxx_read_manufacturer_and_device_id(uint8_t* id_buf);
// extern void w25qxx_read_devid(uint8_t* id_buf);

void w25qxx_read_write_test(int flg);


#endif