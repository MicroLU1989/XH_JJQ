/*
 * @Author: MicroLU1989 916739459@qq.com
 * @Date: 2024-12-18 15:24:09
 * @LastEditors: MicroLU1989 916739459@qq.com
 * @LastEditTime: 2025-03-05 14:12:01
 * @FilePath: \nc-link-v2-adapter\components\utils\linklist.c
 * @Description: 
 * Copyright (c) 2025 by xhzl , All Rights Reserved.
 */

#include <stdlib.h>
#include "my_linklist.h"
#include "user_log.h"
#include "os_port.h"



/* my_linklist.c */
linklist_t *linklist_create(void) {
    linklist_t *node = (linklist_t *)os_malloc(sizeof(linklist_t));
    if (node == NULL) {
        log_e("Memory allocation failed\n");
        return NULL;
    }
    node->next = NULL;
    node->prev = NULL;  // 初始化前驱指针
    node->data = NULL;
    return node;
}

void linklist_insert(linklist_t *header, linklist_t *node) {
    if (header == NULL || node == NULL) {
        log_e("Invalid parameters\n");
        return;
    }

    // 插入到header之后
    node->next = header->next;
    node->prev = header;  // 设置前驱指针
    
    if (header->next != NULL) {
        header->next->prev = node;  // 更新后续节点的前驱指针
    }
    header->next = node;
}


void linklist_insert_before(linklist_t *target, linklist_t *new_node) {
    if (target == NULL || new_node == NULL) {
        log_e("Invalid parameters for insert_before\n");
        return;
    }

    // 将新节点插入到目标节点之前
    new_node->next = target;
    new_node->prev = target->prev;

    if (target->prev != NULL) {
        target->prev->next = new_node;
    }
    target->prev = new_node;
}


void linklist_insert_after(linklist_t *target, linklist_t *new_node) {
    if (target == NULL || new_node == NULL) {
        log_e("Invalid parameters for insert_after\n");
        return;
    }
    // 将新节点插入到目标节点之后
    new_node->prev = target;
    new_node->next = target->next;

    if (target->next != NULL) {
        target->next->prev = new_node;
    }
    target->next = new_node;
}



void linklist_delete(linklist_t *header, linklist_t *node) {
    if (header == NULL || node == NULL) {
        log_e("Invalid parameters\n");
        return;
    }

    // 断开前后连接
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    // 释放资源
    if (node->data) {
        free(node->data);
        node->data = NULL;
    }
    os_free(node);
}

void linklist_destroy(linklist_t *header) {
    if (header == NULL) {
        log_e("Invalid header\n");
        return;
    }

    linklist_t *current = header;
    while (current != NULL) {
        linklist_t *next_node = current->next;
        
        // 释放当前节点资源
        if (current->data) {
            free(current->data);
        }
        os_free(current);
        
        current = next_node;  // 移动到下一个节点
    }
}