#include "device.h"
#include "uart_device.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "my_queue.h"
#include "os_port.h"

#if defined(SYS_SUPPORT_OS)
#include "os_port.h"
#endif

#include "user_log.h"

#define USART_DATA_ADDRESS(uartx) ((uint32_t)&USART_DATA(uartx))

struct uart_physical_t uart_phy_map[UART_MAX] = {
    // UART_0 配置
    [UART_0] = {
        .tx_pin = GPIO_PIN_9,
        .rx_pin = GPIO_PIN_10,
        .tx_port = GPIOA,
        .rx_port = GPIOA,
        .tx_port_rcu = RCU_GPIOA,
        .rx_port_rcu = RCU_GPIOA,
        .uart_rcu = RCU_USART0,
        .uart_id = USART0,
        .dma_id = DMA0,
        .dma_rcu = RCU_DMA0,
        .dma_tx_channel = DMA_CH3,
        .dma_rx_channel = DMA_CH4,
        .uart_irqn = USART0_IRQn,
        .dma_tx_irqn = DMA0_Channel3_IRQn,
        .dma_tx_irq_priority = 6},

    // UART_1 配置
    [UART_1] = {.tx_pin = GPIO_PIN_2, .rx_pin = GPIO_PIN_3, .tx_port = GPIOA, .rx_port = GPIOA, .tx_port_rcu = RCU_GPIOA, .rx_port_rcu = RCU_GPIOA, .uart_rcu = RCU_USART1, .uart_id = USART1, .dma_id = DMA0, .dma_rcu = RCU_DMA0, .dma_tx_channel = DMA_CH6, .dma_rx_channel = DMA_CH5, .uart_irqn = USART1_IRQn, .dma_tx_irqn = DMA0_Channel6_IRQn, .dma_tx_irq_priority = 6},

    // UART_2 配置
    [UART_2] = {.tx_pin = GPIO_PIN_10, .rx_pin = GPIO_PIN_11, .tx_port = GPIOA, .rx_port = GPIOA, .tx_port_rcu = RCU_GPIOA, .rx_port_rcu = RCU_GPIOA, .uart_rcu = RCU_USART2, .uart_id = USART2, .dma_id = DMA0, .dma_rcu = RCU_DMA0, .dma_tx_channel = DMA_CH1, .dma_rx_channel = DMA_CH2, .uart_irqn = USART2_IRQn, .dma_tx_irqn = DMA0_Channel1_IRQn, .dma_tx_irq_priority = 6},

    // UART_3 配置
    [UART_3] = {.tx_pin = GPIO_PIN_3, .rx_pin = GPIO_PIN_2, .tx_port = GPIOA, .rx_port = GPIOA, .tx_port_rcu = RCU_GPIOA, .rx_port_rcu = RCU_GPIOA, .uart_rcu = RCU_UART3, .uart_id = UART3, .dma_id = DMA1, .dma_rcu = RCU_DMA1, .dma_tx_channel = DMA_CH4, .dma_rx_channel = DMA_CH2, .dma_tx_irqn = DMA1_Channel4_IRQn, .dma_tx_irq_priority = 6},

    // UART_4 配置
    [UART_4] = {.tx_pin = GPIO_PIN_5, .rx_pin = GPIO_PIN_6, .tx_port = GPIOA, .rx_port = GPIOA, .tx_port_rcu = RCU_GPIOA, .rx_port_rcu = RCU_GPIOA, .uart_rcu = RCU_UART4, .uart_id = UART4,
                .dma_id = -1,         // UART4 没有 DMA
                .dma_rcu = -1,        // UART4 没有 DMA
                .dma_tx_channel = -1, // UART4 没有 DMA
                .dma_rx_channel = -1, // UART4 没有 DMA
                .uart_irqn = UART4_IRQn,
                .dma_tx_irqn = -1,
                .dma_tx_irq_priority = 6},
};

static uint32_t uart_stopbits_map[UART_STOPBITS_MAX] = {
    [UART_STOPBITS_1] = USART_STB_1BIT,
    [UART_STOPBITS_2] = USART_STB_2BIT,
    [UART_STOPBITS_1_5] = USART_STB_1_5BIT,
};

#ifndef SYS_USING_HEAP

static uint8_t uart_device_index = 0;
struct uart_device_t uart_device_table[UART_MAX] = {0};

uint8_t uart_0_rx_buf[UART_0_RX_BUF_SIZE];
uint8_t uart_1_rx_buf[UART_1_RX_BUF_SIZE];
uint8_t uart_2_rx_buf[UART_2_RX_BUF_SIZE];
uint8_t uart_3_rx_buf[UART_3_RX_BUF_SIZE];
uint8_t uart_4_rx_buf[UART_4_RX_BUF_SIZE];

static queue_t uart_rx_queue_map[UART_MAX] = {
    [UART_0] = {
        .size = UART_0_RX_BUF_SIZE,
        .data_size = 1,
        .buf = uart_0_rx_buf,
        .head = 0,
        .tail = 0,
    },
    [UART_1] = {
        .size = UART_1_RX_BUF_SIZE,
        .data_size = 1,
        .buf = uart_1_rx_buf,
        .head = 0,
        .tail = 0,
    },
    [UART_2] = {
        .size = UART_2_RX_BUF_SIZE,
        .data_size = 1,
        .buf = uart_2_rx_buf,
        .head = 0,
        .tail = 0,
    },
    [UART_3] = {
        .size = UART_3_RX_BUF_SIZE,
        .data_size = 1,
        .buf = uart_3_rx_buf,
        .head = 0,
        .tail = 0,
    },

    [UART_4] = {
        .size = UART_4_RX_BUF_SIZE,
        .data_size = 1,
        .buf = uart_4_rx_buf,
        .head = 0,
        .tail = 0,
    },
};

#endif

static struct uart_device_t *uart_device_map[UART_MAX] = {
    [UART_0] = NULL,
    [UART_1] = NULL,
    [UART_2] = NULL,
    [UART_3] = NULL,
    [UART_4] = NULL,
};

struct uart_device_t *uart_device_get_by_id(enum uart_id id)
{
    return uart_device_map[id];
}

struct device_t *uart_device_get(const char *dev_name)
{
    for (int i = 0; i < UART_MAX; i++)
    {
        if (uart_device_map[i] != NULL && strcmp(uart_device_map[i]->parent.name, dev_name) == 0)
        {
            return (struct device_t *)uart_device_map[i];
        }
    }
    return NULL;
}

int uart_device_init(struct device_t *pdev, void *config)
{
    struct uart_device_t *uart_dev = (struct uart_device_t *)pdev;
    struct uart_config_t *uart_cfg = (struct uart_config_t *)config;

    if (pdev == NULL || config == NULL)
    {
        return -1;
    }
    uart_dev->id = uart_cfg->id;
    // 配置GPIO引脚
    // GPIO和USART时钟配置
    if (uart_phy_map[uart_cfg->id].tx_port_rcu != uart_phy_map[uart_cfg->id].rx_port_rcu)
    {
        rcu_periph_clock_enable(uart_phy_map[uart_cfg->id].tx_port_rcu);
    }
    rcu_periph_clock_enable(uart_phy_map[uart_cfg->id].rx_port_rcu);
    gpio_init(uart_phy_map[uart_cfg->id].tx_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, uart_phy_map[uart_cfg->id].tx_pin);
    gpio_init(uart_phy_map[uart_cfg->id].rx_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, uart_phy_map[uart_cfg->id].rx_pin);

    // 初始化UART硬件
    rcu_periph_clock_enable(uart_phy_map[uart_cfg->id].uart_rcu);

    usart_deinit(uart_phy_map[uart_cfg->id].uart_id);
    usart_baudrate_set(uart_phy_map[uart_cfg->id].uart_id, uart_cfg->baudrate);
    usart_receive_config(uart_phy_map[uart_cfg->id].uart_id, USART_RECEIVE_ENABLE);
    usart_transmit_config(uart_phy_map[uart_cfg->id].uart_id, USART_TRANSMIT_ENABLE);
    usart_enable(uart_phy_map[uart_cfg->id].uart_id);

    // 3. 创建接收队列
    uart_dev->rx_queue = queue_create(uart_cfg->rx_size, sizeof(uint8_t)); // 注意：改 rx_szie -> rx_size
    if (uart_dev->rx_queue == NULL)
        return -1;
    queue_t *rx_queue = (queue_t *)uart_dev->rx_queue;

    // 4. 如果不是 UART4，配置 DMA
    if (uart_cfg->id != UART_4)
    {
        uint32_t dma_id = uart_phy_map[uart_cfg->id].dma_id;
        uint32_t dma_rcu = uart_phy_map[uart_cfg->id].dma_rcu;
        uint32_t dma_rx_ch = uart_phy_map[uart_cfg->id].dma_rx_channel;
        uint32_t uart_id = uart_phy_map[uart_cfg->id].uart_id;
        // DMA 配置
        dma_parameter_struct dma_init_struct;
        // 使能 DMA 时钟
        rcu_periph_clock_enable(dma_rcu);
        // --- DMA RX 配置 ---
        dma_deinit(dma_id, dma_rx_ch);
        usart_flag_clear(uart_id, USART_FLAG_RBNE);
        usart_dma_receive_config(uart_id, USART_RECEIVE_DMA_ENABLE); // 使能 USART DMA 接收
        dma_struct_para_init(&dma_init_struct);

        dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
        dma_init_struct.memory_addr = (uint32_t)(rx_queue->buf);
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
        dma_init_struct.number = uart_cfg->rx_size; // 循环缓冲区大小
        dma_init_struct.periph_addr = USART_DATA_ADDRESS(uart_id);
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_init(dma_id, dma_rx_ch, &dma_init_struct);
        dma_circulation_enable(dma_id, dma_rx_ch); // 循环模式
        dma_channel_enable(dma_id, dma_rx_ch);
        uart_dev->rx_dma_initial_len = uart_cfg->rx_size; // 记录初始 DMA 剩余长度（= 总大小）
        // --- DMA TX 配置：不在 init 中配置具体 buffer，留到 write() 动态设 ---
        // 可在此仅 deinit TX channel，避免上电误传
        uint32_t dma_tx_ch = uart_phy_map[uart_cfg->id].dma_tx_channel;
        dma_deinit(dma_id, dma_tx_ch);
    }
    else
    {
        // UART4 走中断方式
        nvic_irq_enable(uart_phy_map[uart_cfg->id].uart_irqn, uart_phy_map[uart_dev->id].dma_tx_irq_priority, 0);
    }

    uart_device_map[uart_dev->id] = uart_dev;
    return 0;
}

int uart_device_deinit(struct device_t *pdev)
{
    if (pdev == NULL)
    {
        return -1;
    }
    struct uart_device_t *uart_dev = (struct uart_device_t *)pdev;
    // 反初始化UART硬件
    usart_disable(uart_phy_map[uart_dev->id].uart_id);
    return 0;
}

int uart_device_control(struct device_t *pdev, uint32_t cmd, void *arg)
{
    struct uart_device_t *uart_dev = (struct uart_device_t *)pdev;
    // 根据cmd执行相应的控制操作
    return 0;
}

int uart_device_write(struct device_t *pdev, void *extern_arg, uint8_t *buf, uint32_t size)
{
    struct uart_device_t *uart_dev = (struct uart_device_t *)pdev;
    if (pdev == NULL || buf == NULL || size == 0)
        return -1;

    // UART4 无 DMA，用阻塞发送
    if (uart_dev->id == UART_4)
    {
        for (uint32_t i = 0; i < size; i++)
        {
            while (usart_flag_get(uart_phy_map[uart_dev->id].uart_id, USART_FLAG_TBE) == RESET)
                ;
            usart_data_transmit(uart_phy_map[uart_dev->id].uart_id, buf[i]);
        }
        return size;
    }

    // 有 DMA 的 UART
    uint32_t dma_id = uart_phy_map[uart_dev->id].dma_id;
    uint32_t dma_ch = uart_phy_map[uart_dev->id].dma_tx_channel;
    uint32_t uart_id = uart_phy_map[uart_dev->id].uart_id;

    // 每次发送前重新配置 DMA
    dma_deinit(dma_id, dma_ch);
    dma_channel_disable(dma_id, dma_ch);

    dma_parameter_struct dma_init_struct;
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_addr = USART_DATA_ADDRESS(uart_id);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.number = size;

    dma_init(dma_id, dma_ch, &dma_init_struct);
    dma_circulation_disable(dma_id, dma_ch);
    dma_channel_enable(dma_id, dma_ch);

    nvic_irq_enable(uart_phy_map[uart_dev->id].dma_tx_irqn, uart_phy_map[uart_dev->id].dma_tx_irq_priority, 0);
    dma_interrupt_enable(dma_id, dma_ch, DMA_INT_FTF);             // 开启中断
    usart_dma_transmit_config(uart_id, USART_TRANSMIT_DMA_ENABLE); // 启动传输

#if defined(SYS_SUPPORT_OS)
    if (uart_dev->tx_complete_sem != NULL)
    {
        os_sem_take(uart_dev->tx_complete_sem, OS_MAX_DELAY);
    }
#endif

    return size;
}

int uart_device_read(struct device_t *pdev, void *extern_arg, uint8_t *buf, uint32_t size)
{
    if (pdev == NULL || buf == NULL || size == 0)
        return 0;

    struct uart_device_t *uart_dev = (struct uart_device_t *)pdev;
    if (uart_dev->id == UART_4)
    {
        // UART4 中断接收处理（略）
        return 0;
    }

    queue_t *q = (queue_t *)uart_dev->rx_queue;
    uint32_t dma_id = uart_phy_map[uart_dev->id].dma_id;
    uint32_t dma_ch = uart_phy_map[uart_dev->id].dma_rx_channel;
    uint32_t uart_id = uart_phy_map[uart_dev->id].uart_id;

    // 获取 DMA 剩余未接收字节数
    uint32_t remaining = dma_transfer_number_get(dma_id, dma_ch);
    // 已接收字节数
    q->tail = uart_dev->rx_dma_initial_len - remaining;

#if defined(SYS_SUPPORT_OS)
    // os_enter_critical();
#endif

    // 将队列中从 head 到 tail（不含 tail）的数据弹出到用户 buf
    uint32_t read_size = 0;
    uint8_t data;
    while ((read_size < size) && !queue_is_empty(q))
    {
        queue_pop(q, &data);
        buf[read_size++] = data;
    }

#if defined(SYS_SUPPORT_OS)
    // os_exit_critical();
#endif

    return read_size;
}

struct device_t *uart_device_create(const char *dev_name)
{
#if defined(SYS_USING_HEAP)
    struct uart_device_t *uart_dev = (struct uart_device_t *)malloc(sizeof(struct uart_device_t));
    memset(uart_dev, 0, sizeof(struct uart_device_t));
#else
    if (uart_device_index >= UART_MAX)
    {
        return NULL;
    }
    struct uart_device_t *uart_dev = &uart_device_table[uart_device_index++];
    uart_dev->rx_queue = uart_rx_queue_map[uart_cfg->id];
#endif

#if defined(SYS_SUPPORT_OS)
    char uart_sem_name[32] = {0};
    sprintf(uart_sem_name, "sem_%s", dev_name);
    uart_dev->tx_complete_sem = os_sem_create(uart_sem_name);
#endif
    uart_dev->parent.type = DEVICE_TYPE_UART;
    uart_dev->parent.init = uart_device_init;
    uart_dev->parent.deinit = uart_device_deinit;
    uart_dev->parent.control = uart_device_control;
    uart_dev->parent.read = uart_device_read;
    uart_dev->parent.write = uart_device_write;
    uart_dev->parent.status = DEVICE_STATUS_UNKNOWN;
    int len = strlen(dev_name);
    if (len > (sizeof(uart_dev->parent.name) - 1))
        len = sizeof(uart_dev->parent.name) - 1;
    memcpy(uart_dev->parent.name, dev_name, len);
    return (struct device_t *)uart_dev;
}

/**
 * @brief  :  DMA发送完成中断处理函数
 * @param  : id :串口编号
 * @return : undefinedundefined
 */
static void dma_tx_irq_handler(enum uart_id id)
{
    if (dma_interrupt_flag_get(uart_phy_map[id].dma_id, uart_phy_map[id].dma_tx_channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(uart_phy_map[id].dma_id, uart_phy_map[id].dma_tx_channel, DMA_INT_FLAG_G);
        usart_dma_transmit_config(uart_phy_map[id].uart_id, USART_TRANSMIT_DMA_DISABLE); // 关闭DMA
#if defined(SYS_SUPPORT_OS)
        struct uart_device_t *uart_dev = uart_device_get_by_id(id);
        if (uart_dev->tx_complete_sem != NULL)
        {
            int flg = 0;
            os_sem_release_from_isr(uart_dev->tx_complete_sem, &flg);
        }
#endif
    }
}

/**
 * @brief  : UART0 DMA发送完成中断
 * @return : undefinedundefined
 */
void DMA0_Channel3_IRQHandler(void)
{
    dma_tx_irq_handler(UART_0);
}

/**
 * @brief  : UART1 DMA发送完成中断
 * @return : undefinedundefined
 */
void DMA0_Channel6_IRQHandler(void)
{
    dma_tx_irq_handler(UART_1);
}

/**
 * @brief  : UART2 DMA发送完成中断
 * @return : undefinedundefined
 */
void DMA0_Channel1_IRQHandler(void)
{
    dma_tx_irq_handler(UART_2);
}

/**
 * @brief  : UART3 DMA发送完成中断
 * @return : undefinedundefined
 */
void DMA1_Channel4_IRQHandler(void)
{
    dma_tx_irq_handler(UART_3);
}

static void uart_irq_handler(enum uart_id id)
{
    if (RESET != usart_interrupt_flag_get(uart_phy_map[id].uart_id, USART_INT_FLAG_RBNE))
    {
        /* receive data */
        uint8_t data = usart_data_receive(uart_phy_map[id].uart_id);
        struct uart_device_t *uart_dev = uart_device_get_by_id(id);
        if (uart_dev != NULL)
        {
            queue_push(uart_dev->rx_queue, &data);
        }
        usart_interrupt_flag_clear(uart_phy_map[id].uart_id, USART_INT_FLAG_RBNE);
    }
    if (RESET != usart_interrupt_flag_get(uart_phy_map[id].uart_id, USART_INT_FLAG_TBE))
    {
        /* transmit data */
        usart_interrupt_flag_clear(uart_phy_map[id].uart_id, USART_INT_FLAG_TBE);
    }
}

// UART4中断处理函数
void UART4_IRQHandler(void)
{
    uart_irq_handler(UART_4);
}
