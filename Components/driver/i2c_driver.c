#include "i2c_driver.h"
#include "gpio_driver.h"
#include "os_port.h"
// #include "systick.h"
#include "user_log.h"

/**
 * \brief 计算软件模拟I2C的delay_us值
 * \param clk_speed I2C时钟速度，单位Hz，例如100kHz=100000
 * \return 计算得出的delay_us值
 */
static uint32_t calculate_soft_i2c_delay(uint32_t clk_speed)
{
    // 理论上，对于给定的时钟频率，一个完整周期的时间为 1/clk_speed 秒
    // 转换为微秒：(1/clk_speed) * 1000000 = 1000000 / clk_speed
    // 在软件模拟I2C中，我们需要在一个完整周期内完成数据设置和采样
    // 通常，我们将这个周期分成两半，一半用于设置数据，一半用于采样

    if (clk_speed == 0)
    {
        return 0;
    }

    // 计算理论周期的一半（以微秒为单位）
    // 使用更精确的计算方法来处理小数部分
    uint32_t half_period_calc = (1000000UL + clk_speed - 1) / (clk_speed * 2U); // 向上取整

    // 考虑到执行指令的时间开销，需要适当调整
    // 通常，实际使用的delay_us值要比理论值小一些
    // 这里使用一个经验系数，可根据实际情况调整

    if (half_period_calc > 1)
    {
        // 对于较慢的速度，可以减去一个小的开销值
        return half_period_calc - 1;
    }
    else
    {
        // 对于较高的速度，不能低于最小值，否则会导致速度过快
        return 1; // 最小延迟为1微秒
    }
}

/**
 * \brief 自动根据速度计算delay_us值
 * \param i2c_adp I2C适配器指针
 */
void auto_calculate_i2c_delay(i2c_adapter_t *i2c_adp)
{
    i2c_adp->delay_us = calculate_soft_i2c_delay(i2c_adp->i2c_speed);
}

void i2c_init(i2c_adapter_t *i2c_adp)
{
    // 如果设置了i2c_speed，自动计算delay_us
    if (i2c_adp->i2c_speed > 0)
    {
        auto_calculate_i2c_delay(i2c_adp);
    }
    else
    {
        i2c_adp->delay_us = 5;
        i2c_adp->i2c_speed = 100000;
    }
    drv_gpio_init(i2c_adp->scl_port, i2c_adp->scl_pin_number, PIN_MODE_OUT_PP, PIN_SPEED_50M);
    drv_gpio_init(i2c_adp->sda_port, i2c_adp->sda_pin_number, PIN_MODE_OUT_PP, PIN_SPEED_50M);
    // 设置SCL和SDA为高电平,空闲状态
    drv_gpio_write(i2c_adp->scl_port, i2c_adp->scl_pin_number, 1);
    drv_gpio_write(i2c_adp->sda_port, i2c_adp->sda_pin_number, 1);
}

static inline void i2c_scl_high(i2c_adapter_t *i2c_adp)
{
    drv_gpio_write(i2c_adp->scl_port, i2c_adp->scl_pin_number, 1);
}

static inline void i2c_scl_low(i2c_adapter_t *i2c_adp)
{
    drv_gpio_write(i2c_adp->scl_port, i2c_adp->scl_pin_number, 0);
}

static inline void i2c_sda_high(i2c_adapter_t *i2c_adp)
{
    drv_gpio_write(i2c_adp->sda_port, i2c_adp->sda_pin_number, 1);
}

static inline void i2c_sda_low(i2c_adapter_t *i2c_adp)
{
    drv_gpio_write(i2c_adp->sda_port, i2c_adp->sda_pin_number, 0);
}

static inline void i2c_sda_out(i2c_adapter_t *i2c_adp)
{
    drv_gpio_init(i2c_adp->sda_port, i2c_adp->sda_pin_number, PIN_MODE_OUT_PP, PIN_SPEED_50M);
}

static inline void i2c_sda_in(i2c_adapter_t *i2c_adp)
{
    drv_gpio_init(i2c_adp->sda_port, i2c_adp->sda_pin_number, PIN_MODE_IN_FLOATING, PIN_SPEED_50M);
}

static inline uint8_t i2c_sda_read(i2c_adapter_t *i2c_adp)
{
    return drv_gpio_read(i2c_adp->sda_port, i2c_adp->sda_pin_number, PIN_DIRECTION_IN);
}

static inline void i2c_delay_us(i2c_adapter_t *i2c_adp)
{
    extern void delay_1us(uint32_t count);
    delay_1us(i2c_adp->delay_us);
}

static void i2c_start(i2c_adapter_t *i2c_adp)
{
    i2c_sda_out(i2c_adp);
    i2c_sda_high(i2c_adp);
    i2c_scl_high(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_sda_low(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_scl_low(i2c_adp); // 拉低时钟线,准备传输数据
}


#if 0
// IIC停止信号
void i2c_stop(i2c_adapter_t *i2c_adp)
{
    i2c_sda_out(i2c_adp);
    i2c_sda_low(i2c_adp);
    i2c_scl_low(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_scl_high(i2c_adp);
    i2c_sda_high(i2c_adp);
    i2c_delay_us(i2c_adp);
}
#else
void i2c_stop(i2c_adapter_t *i2c_adp)
{
    i2c_sda_out(i2c_adp);
    i2c_scl_high(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_sda_high(i2c_adp);
    i2c_delay_us(i2c_adp);
}
#endif

void i2c_ack(i2c_adapter_t *i2c_adp, unsigned char ack_en)
{
    i2c_sda_out(i2c_adp);
    i2c_scl_low(i2c_adp);
    if (ack_en)
    {
        i2c_sda_low(i2c_adp); // 产生应答信号
    }
    else
    {
        i2c_sda_high(i2c_adp); // 不产生应答信号
    }
    i2c_delay_us(i2c_adp);
    i2c_scl_high(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_scl_low(i2c_adp);
}

// 等待应答
/**
 * @brief  : 等待应答
 * @param  : i2c_adp : 适配器
 * @return : 0 无应答, 1有应答
 */
uint8_t i2c_wait_ack(i2c_adapter_t *i2c_adp)
{
    uint8_t err_cnt = 0;
    i2c_sda_out(i2c_adp);
    i2c_sda_high(i2c_adp);
    i2c_delay_us(i2c_adp);
    i2c_sda_in(i2c_adp);
    i2c_scl_high(i2c_adp);
    i2c_delay_us(i2c_adp);
    while (i2c_sda_read(i2c_adp))
    {
        err_cnt++;
        i2c_delay_us(i2c_adp);
        if (err_cnt > 250)
        {
            i2c_stop(i2c_adp);
            return 0; // 在SCL高电平期间,SDA为高电平,从机没有应答
        }
    }
    i2c_scl_low(i2c_adp);
    return 1; // 在SCL高电平期间,SDA为低电平,从机有应答
}

// 发送一个字节
void i2c_send_byte(i2c_adapter_t *i2c_adp, uint8_t dat)
{
    i2c_sda_out(i2c_adp);
    i2c_scl_low(i2c_adp); // 拉低时钟线,开始传输数据
    for (uint8_t i = 0; i < 8; i++)
    {
        if (dat & 0x80)
        {
            i2c_sda_high(i2c_adp);
        }
        else
        {
            i2c_sda_low(i2c_adp);
        }
        dat <<= 1;
        i2c_delay_us(i2c_adp);
        i2c_scl_high(i2c_adp); // SCL高电平期间,SDA数据保持稳定
        i2c_delay_us(i2c_adp);
        i2c_scl_low(i2c_adp);
    }
}

// 读取一个字节
uint8_t i2c_read_byte(i2c_adapter_t *i2c_adp, uint8_t ack_flg)
{
    uint8_t dat = 0, i;
    i2c_sda_in(i2c_adp);
    for (i = 0; i < 8; i++)
    {
        i2c_scl_high(i2c_adp); // 在SCL高电平期间,读取SDA数据
        dat <<= 1;
        if (i2c_sda_read(i2c_adp))
        {
            dat++;
        }
        i2c_scl_low(i2c_adp);
    }
    i2c_ack(i2c_adp, ack_flg);
    return dat;
}

/**
 * @brief  : 从寄存器读取数据
 * @param  : i2c_adp :设备地址
 * @param  : reg_addr :寄存器
 * @param  : buf :数据缓存
 * @param  : read_len :读取长度
 * @return : 1 成功, 0 失败
 */
uint8_t i2c_read_reg(i2c_adapter_t *i2c_adp, uint8_t reg_addr, uint8_t *buf, uint32_t read_len)
{
    if (read_len == 0 || i2c_adp == NULL)
    {
        log_e("err !");
        return 0;
    }
    i2c_start(i2c_adp); // 发送起始信号
    // i2c_send_byte(i2c_adp, (i2c_adp->dev_addr << 1) | 0); //发送设备地址与写信号
    i2c_send_byte(i2c_adp, i2c_adp->dev_addr | 0); // 发送设备地址与写信号, 8bit 地址
    i2c_wait_ack(i2c_adp);                         // 等待从机应答
    i2c_send_byte(i2c_adp, reg_addr);              // 发送寄存器地址
    i2c_wait_ack(i2c_adp);                         // 等待从机应答
    i2c_delay_us(i2c_adp);
		
    i2c_start(i2c_adp); // 发送起始信号
    i2c_send_byte(i2c_adp, i2c_adp->dev_addr | 1); // 发送设备地址与读信号
    i2c_wait_ack(i2c_adp);                         // 等待从机应答
    for (uint32_t i = 0; i < read_len; i++)
    {
        if (i == read_len - 1)
        {
            buf[i] = i2c_read_byte(i2c_adp, 0); // 最一个字节,不产生ACK
        }
        else
        {
            buf[i] = i2c_read_byte(i2c_adp, 1);
        }
    }
    i2c_stop(i2c_adp);
    return 1;
}

/**
 * @brief  : 往寄存器写入数据
 * @param  : i2c_adp :设备地址
 * @param  : reg_addr :寄存器
 * @param  : *data_buf :数据
 * @param  : write_len :数据长度
 * @return : 无
 */
void i2c_write_reg(i2c_adapter_t *i2c_adp, uint8_t reg_addr, uint8_t *data_buf, uint32_t write_len)
{
    if (i2c_adp == NULL)
    {
        log_e("err !");
    }
    i2c_start(i2c_adp); // 发送起始信号
    // i2c_send_byte(i2c_adp, (i2c_adp->dev_addr << 1) | 0); //发送设备地址与写信号
    i2c_send_byte(i2c_adp, i2c_adp->dev_addr | 0); // 发送设备地址与写信号, 8bit 地址
    i2c_wait_ack(i2c_adp);                         // 等待从机应答
    i2c_send_byte(i2c_adp, reg_addr);              // 发送寄存器地址
    i2c_wait_ack(i2c_adp);                         // 等待从机应答
    for (uint32_t i = 0; i < write_len; i++)
    {
        i2c_send_byte(i2c_adp, data_buf[i]); // 写入数据
        i2c_wait_ack(i2c_adp);               // 等待从机应答
    }
    i2c_stop(i2c_adp);
    i2c_delay_us(i2c_adp);
}
