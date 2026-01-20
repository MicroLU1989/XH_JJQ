#ifndef _UART_DEVICE_H_
#define _UART_DEVICE_H_

#include <stdbool.h>

#include "device.h"



#define UART_0_RX_BUF_SIZE    256
#define UART_1_RX_BUF_SIZE    256
#define UART_2_RX_BUF_SIZE    256
#define UART_3_RX_BUF_SIZE    256
#define UART_4_RX_BUF_SIZE    256



struct uart_device_t
{
    struct device_t parent;
    uint32_t id;
    uint32_t rx_dma_initial_len;          // 新增：记录DMA接收初始长度
    void *rx_queue;
    void *tx_complete_sem;
};

enum uart_cmd
{
    UART_CMD_SET_BAUDRATE = 0x01,
};

enum uart_parity
{
    UART_PARITY_NONE = 0x00,
    UART_PARITY_EVEN = 0x01,
    UART_PARITY_ODD  = 0x02,
};

enum uart_stopbits
{
    UART_STOPBITS_1 = 0,
    UART_STOPBITS_2 ,
    UART_STOPBITS_1_5 ,
    UART_STOPBITS_MAX ,
};

enum uart_id
{
    UART_0 = 0,
    UART_1 ,
    UART_2 ,
    UART_3 ,
    UART_4 ,
    UART_MAX ,
};



struct uart_config_t
{
    uint32_t id;
    uint32_t baudrate;
    uint32_t rx_size;
};


struct uart_physical_t
{
    uint32_t tx_pin;
    uint32_t rx_pin;
    uint32_t tx_port;
    uint32_t rx_port;
    uint32_t tx_port_rcu;
    uint32_t rx_port_rcu;
    uint32_t uart_rcu;
    uint32_t uart_id;
    uint32_t dma_id;
    uint32_t dma_rcu;
    uint32_t dma_tx_channel;
    uint32_t dma_rx_channel;
    uint32_t uart_irqn;
    uint32_t dma_tx_irqn;
    uint32_t uart_irq_priority;     //串口中断优先级
    uint32_t dma_tx_irq_priority;   //DMA中断优先级
};


#define UART_DEVICE_CONFIG(uart_num,baudrate,databits,stopbits,parity,rx_buf_szie) \
    { \
        .id = uart_num, \
        .baudrate = baudrate, \
        .databits = databits, \
        .stopbits = stopbits, \
        .parity = parity, \
        .rx_szie = rx_buf_szie, \
    }
extern struct device_t *uart_device_get(const char *dev_name);
extern struct device_t *uart_device_create(const char *dev_name);

#endif  