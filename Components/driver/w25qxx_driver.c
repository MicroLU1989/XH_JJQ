
#include "w25qxx_driver.h"
#include "gpio_driver.h"
#include "gd32f30x_spi.h"
#include "user_log.h"
#include <stdbool.h>
#include <stdio.h>

#define FLASH_GPIO_PORT (GPIO_PORT_A)
#define FLASH_CS_PIN (GPIO_PIN_NUM(4))
#define FLASH_SCK_PIN (GPIO_PIN_NUM(5))
#define FLASH_MISO_PIN (GPIO_PIN_NUM(6))
#define FLASH_MOSI_PIN (GPIO_PIN_NUM(7))

#define FLASH_CS_L drv_gpio_write(FLASH_GPIO_PORT, FLASH_CS_PIN, 0) // 拉低片选信号,开启片选
#define FLASH_CS_H drv_gpio_write(FLASH_GPIO_PORT, FLASH_CS_PIN, 1) // 拉高片选信号,关闭片选

static void gd32_spi_init(uint32_t spi_periph)
{
    spi_parameter_struct spi_init_struct;

    // 启用SPI时钟
    if (spi_periph == SPI0)
    {
        rcu_periph_clock_enable(RCU_SPI0);
        rcu_periph_clock_enable(RCU_GPIOA); // 启用GPIOA时钟
    }
    else if (spi_periph == SPI1)
    {
        rcu_periph_clock_enable(RCU_SPI1);
        rcu_periph_clock_enable(RCU_GPIOB); // 启用GPIOB时钟
    }

    /* SPI parameter config */
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; // 改为低电平采样，第一边沿采样
    spi_init_struct.nss = SPI_NSS_SOFT;                            // 改为软件NSS，因为CS是手动管理的
    spi_init_struct.prescale = SPI_PSC_8;                          // 提高速度到SPI_PSC_8
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(spi_periph, &spi_init_struct);
    spi_enable(spi_periph); // 启用 SPI 外设
}

static void spi0_gpio_init(void)
{
    // 初始化 SPI0 的 GPIO 引脚
    drv_gpio_af_rcu_enable();
    drv_gpio_init(FLASH_GPIO_PORT, FLASH_CS_PIN, PIN_MODE_OUT_PP, PIN_SPEED_50M);
    drv_gpio_init(FLASH_GPIO_PORT, FLASH_SCK_PIN, PIN_MODE_AF_PP, PIN_SPEED_50M);
    drv_gpio_init(FLASH_GPIO_PORT, FLASH_MOSI_PIN, PIN_MODE_AF_PP, PIN_SPEED_50M);
    drv_gpio_init(FLASH_GPIO_PORT, FLASH_MISO_PIN, PIN_MODE_IN_FLOATING, PIN_SPEED_50M);
    // 设置 CS 引脚为高电平，表示未选中
    FLASH_CS_H;
}

static uint8_t spi0_write_read(uint8_t data)
{
    uint32_t timeout = 0xFFFFFF; // 超时计数器

    // 等待发送缓冲区为空
    while ((spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET) && (--timeout))
        ;

    if (timeout == 0)
        return 0xFF; // 超时返回

    // 发送数据
    spi_i2s_data_transmit(SPI0, data);

    timeout = 0xFFFFFF; // 重置超时计数器
    // 等待接收缓冲区非空
    while ((spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) == RESET) && (--timeout))
        ;

    if (timeout == 0)
        return 0xFF; // 超时返回

    // 读取接收到的数据
    uint16_t recv_val = 0;
    recv_val = spi_i2s_data_receive(SPI0);
    return recv_val;
}

void w25qxx_wait_busy(void)
{
    uint8_t status = 0;
    FLASH_CS_L;                          // 使能片选
    spi0_write_read(W25X_ReadStatusReg); // 发送读状态寄存器命令
    // 读取状态寄存器，检查 BUSY 位（bit 0）
    while ((status = spi0_write_read(0xFF)) & 0x01)
        ;       // 等待 BUSY 位清除
    FLASH_CS_H; // 禁用片选
}

void w25qxx_read_id(uint8_t *id_buf)
{
    FLASH_CS_L;                          // 使能片选
    spi0_write_read(W25X_JedecDeviceID); // 发送读取ID命令
    id_buf[0] = spi0_write_read(0xFF);   // 读取制造商ID
    id_buf[1] = spi0_write_read(0xFF);   // 读取设备ID高字节
    id_buf[2] = spi0_write_read(0xFF);   // 读取设备ID低字节
    FLASH_CS_H;                          // 禁用片选
}

void w25qxx_read(uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    FLASH_CS_L;                           // 使能片选
    spi0_write_read(0x03);                // 发送读取数据命令
    spi0_write_read((addr >> 16) & 0xFF); // 发送地址高字节
    spi0_write_read((addr >> 8) & 0xFF);  // 发送地址中字节
    spi0_write_read(addr & 0xFF);         // 发送地址低字节
    for (uint32_t i = 0; i < length; i++)
    {
        data_buf[i] = spi0_write_read(0xFF); // 读取数据
    }
    FLASH_CS_H; // 禁用片选
}

void w25qxx_write_enable(void)
{
    FLASH_CS_L;            // 使能片选
    spi0_write_read(0x06); // 发送写使能命令
    FLASH_CS_H;            // 禁用片选
}

void w25qxx_page_program(uint32_t addr, uint8_t *buf, uint32_t size)
{
    w25qxx_write_enable();                // 发送写使能命令
    FLASH_CS_L;                           // 使能片选
    spi0_write_read(W25X_PageProgram);    // 发送页编程命令
    spi0_write_read((addr >> 16) & 0xFF); // 发送地址高字节
    spi0_write_read((addr >> 8) & 0xFF);  // 发送地址中字节
    spi0_write_read(addr & 0xFF);         // 发送地址低字节
    for (uint32_t i = 0; i < size; i++)
    {
        spi0_write_read(buf[i]); // 写入数据
    }
    FLASH_CS_H;         // 禁用片选
    w25qxx_wait_busy(); // 等待写入完成
}

void w25qxx_sector_erase(uint32_t addr)
{
    w25qxx_write_enable();                // 发送写使能命令
    FLASH_CS_L;                           // 使能片选
    spi0_write_read(W25X_SectorErase);    // 发送扇区擦除命令
    spi0_write_read((addr >> 16) & 0xFF); // 发送地址高字节
    spi0_write_read((addr >> 8) & 0xFF);  // 发送地址中字节
    spi0_write_read(addr & 0xFF);         // 发送地址低字节
    FLASH_CS_H;                           // 禁用片选
    w25qxx_wait_busy();                   // 等待擦除完成
}

void w25qxx_chip_erase(void)
{
    w25qxx_write_enable();           // 发送写使能命令
    FLASH_CS_L;                      // 使能片选
    spi0_write_read(W25X_ChipErase); // 发送整片擦除命令
    FLASH_CS_H;                      // 禁用片选
    w25qxx_wait_busy();              // 等待擦除完成
}

void w25qxx_write_disable(void)
{
    FLASH_CS_L;                         // 使能片选
    spi0_write_read(W25X_WriteDisable); // 发送写失能命令
    FLASH_CS_H;                         // 禁用片选
}

/**
 * @brief 直接写入整个扇区（按页写入）
 * @param sector_addr 扇区起始地址
 * @param data 扇区数据
 * @param size 数据大小（应为SECTOR_SIZE）
 */
static void w25qxx_write_sector_direct(uint32_t sector_addr, uint8_t *data, uint32_t size)
{
    const uint32_t PAGE_SIZE = 256;

    // 按页写入整个扇区
    for (uint32_t page_offset = 0; page_offset < size; page_offset += PAGE_SIZE)
    {
        uint32_t page_addr = sector_addr + page_offset;
        w25qxx_page_program(page_addr, &data[page_offset], PAGE_SIZE);
    }
}

/**
 * @brief 擦除扇区并写入新数据
 * @param sector_addr 扇区起始地址
 * @param data 要写入的数据
 * @param size 数据大小（应为SECTOR_SIZE）
 */
static void w25qxx_erase_and_write_sector(uint32_t sector_addr, uint8_t *data, uint32_t size)
{
    // 擦除扇区
    w25qxx_sector_erase(sector_addr);

    // 写入新数据
    w25qxx_write_sector_direct(sector_addr, data, size);
}

uint8_t sector_buf[4096]; // 扇区缓冲区

/**
 * @brief 带智能擦除策略的Flash写入函数
 * @param addr 要写入的地址
 * @param buf 要写入的数据缓冲区
 * @param size 要写入的数据大小
 */
void w25qxx_write_with_erase(uint32_t addr, uint8_t *buf, uint32_t size)
{
    // 参数检查
    if (buf == NULL || size == 0)
    {
        return;
    }
    // W25Qxx 参数
    const uint32_t SECTOR_SIZE = 4096; // 扇区大小为 4KB
    const uint32_t PAGE_SIZE = 256;    // 页大小为 256 字节

    uint32_t sector_addr;
    uint32_t offset;
    uint32_t write_len;
    uint32_t current_addr = addr;
    uint32_t remaining_size = size;
    uint32_t buf_offset = 0;

    // 处理跨越多个扇区的情况
    while (remaining_size > 0)
    {
        sector_addr = current_addr & 0xFFFFF000; // 获取扇区起始地址（4KB 对齐）
        offset = current_addr - sector_addr;     // 计算在扇区内的偏移
        // 计算本次要写入的长度（不超过剩余大小和当前扇区剩余空间）
        write_len = (remaining_size > (SECTOR_SIZE - offset)) ? (SECTOR_SIZE - offset) : remaining_size;
        // 读取整个扇区的数据
        w25qxx_read(sector_addr, sector_buf, SECTOR_SIZE);
        // 检查是否需要写入（数据是否有变化）
        bool need_write = false;
        bool target_all_ff = true;
        bool source_all_zero = true;
        // 单次遍历完成所有条件检查
        for (uint32_t i = 0; i < write_len; i++)
        {
            uint8_t current_data = sector_buf[offset + i];
            uint8_t new_data = buf[buf_offset + i];
            // 检查数据是否相同
            if (current_data != new_data)
            {
                need_write = true;
            }
            // 检查目标区域是否全为0xFF
            if (current_data != 0xFF)
            {
                target_all_ff = false;
            }

            // 检查源数据是否全为0x00
            if (new_data != 0x00)
            {
                source_all_zero = false;
            }
            // 唯一有效的提前退出：确定需要擦除扇区
            if (need_write && !target_all_ff && !source_all_zero)
            {
                break;
            }
        }

        // 如果数据没有变化，跳过操作
        if (!need_write)
        {
					  log_d("数据没有变化");
            goto update_pointers;
        }
        // 将数据写入缓冲区
        memcpy(&sector_buf[offset], &buf[buf_offset], write_len);

        // 源数据全为0x00或Flash目标区域全为0xFF
        if (source_all_zero || target_all_ff) 
        {
            w25qxx_write_sector_direct(sector_addr, sector_buf, SECTOR_SIZE);
        }
        else // 否则，使用擦除并写入策略
        {
            w25qxx_erase_and_write_sector(sector_addr, sector_buf, SECTOR_SIZE);
        }
        
    update_pointers:
        // 更新地址、缓冲区偏移和剩余大小
        current_addr += write_len;
        buf_offset += write_len;
        remaining_size -= write_len;
    }
}
void w25qxx_read_write_test(int flg)
{
    uint8_t buf[] = "2026-01-15 18:36:43";
    static uint8_t tick = 0;
	  sprintf((char *)buf,"2026-01-15 18:36:%02d",tick++);
	  if(tick > 99)
		{
			 tick = 0;
		}
    uint8_t read_buf[sizeof(buf)] = {0};
    uint32_t test_addr = 0x000000; // 测试地址
    if (flg == 1)
    {
        w25qxx_write_with_erase(test_addr, buf, sizeof(buf));
    }
    else
    {
        memset(read_buf, 0, sizeof(read_buf));
        w25qxx_read(test_addr, read_buf, sizeof(buf) - 1);
        log_d("Read Data: %s\n", read_buf);
    }
}

void w25qxx_init(void)
{
    spi0_gpio_init();
    gd32_spi_init(SPI0);
}
