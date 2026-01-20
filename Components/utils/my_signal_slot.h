
#ifndef MY_SIGNAL_SLOT_H
#define MY_SIGNAL_SLOT_H


#include <stdint.h>
#include <stdbool.h>

// 定义信号参数结构体
typedef struct {
    uint8_t key_id;
    uint8_t event_type; // 例如 KEY_PRESS_CLICK, KEY_PRESS_LONG
} key_event_signal_t;



// 通用信号参数结构体
typedef struct {
    void* data;
    uint32_t data_size;
} generic_signal_t;

// 槽函数类型定义
typedef void (*slot_func_t)(generic_signal_t event_data);

// 连接信号与槽的函数声明
int connect_signal_slot(void *signal_emitter, slot_func_t slot);

// 断开信号与槽的函数声明
void disconnect_signal_slot(void *signal_emitter, slot_func_t slot);

// 发射信号的函数声明
void emit_signal(void *signal_emitter, generic_signal_t event_data);

#endif // SIGNAL_SLOT_H