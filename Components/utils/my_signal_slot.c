#include "my_signal_slot.h"
#include "my_linklist.h"
#include "user_log.h"
#include "os_port.h"

// 内部结构体用于存储槽函数
typedef struct {
    slot_func_t func;
    linklist_t node;
} slot_node_t;



// 创建新的信号发射器
void *create_signal_emitter() 
{
    return linklist_create();
}

// 连接信号与槽的实现
int connect_signal_slot(void *signal_emitter, slot_func_t slot) {
    if (signal_emitter == NULL || slot == NULL) {
        log_e("Invalid parameters for connect_signal_slot\n");
        return -1;
    }

    // 创建新的槽节点
    slot_node_t *new_slot = (slot_node_t *)os_malloc(sizeof(slot_node_t));
    if (new_slot == NULL) {
        log_e("Failed to allocate memory for new slot\n");
        return -1;
    }

    new_slot->func = slot;
    linklist_insert(signal_emitter, &new_slot->node);

    log_i("Slot connected successfully to emitter %p\n", signal_emitter);
    return 0;
}

// 断开信号与槽的实现
void disconnect_signal_slot(void *signal_emitter, slot_func_t slot) {
    if (signal_emitter == NULL || slot == NULL) {
        log_e("Invalid parameters for disconnect_signal_slot\n");
        return;
    }

    linklist_t *current = ((linklist_t *)signal_emitter)->next;
    while (current != NULL) {
        slot_node_t *slot_node = (slot_node_t *)current;
        if (slot_node->func == slot) {
            linklist_delete(signal_emitter, current);
            os_free(slot_node);
            log_i("Slot disconnected from emitter %p\n", signal_emitter);
            return;
        }
        current = current->next;
    }

    log_w("Slot not found in emitter %p\n", signal_emitter);
}

// 发射信号的实现
void emit_signal(void *signal_emitter, generic_signal_t event_data) {
    if (signal_emitter == NULL) {
        log_w("Invalid signal_emitter in emit_signal\n");
        return;
    }

    linklist_t *current = ((linklist_t *)signal_emitter)->next;
    while (current != NULL) {
        slot_node_t *slot_node = (slot_node_t *)current;
        if (slot_node->func != NULL) {
            slot_node->func(event_data);
        }
        current = current->next;
    }
}

// 销毁信号发射器及其所有槽函数
void destroy_signal_emitter(void *signal_emitter) {
    if (signal_emitter == NULL) {
        log_w("Invalid signal_emitter in destroy_signal_emitter\n");
        return;
    }

    linklist_t *current = ((linklist_t *)signal_emitter)->next;
    while (current != NULL) {
        linklist_t *next_node = current->next;
        slot_node_t *slot_node = (slot_node_t *)current;
        os_free(slot_node);
        current = next_node;
    }

    linklist_destroy(signal_emitter);
    log_i("Signal emitter destroyed\n");
}