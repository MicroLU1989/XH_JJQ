/*
 * @Author: MicroLU1989 916739459@qq.com
 * @Date: 2025-01-14 14:32:50
 * @LastEditors: MicroLU1989 916739459@qq.com
 * @LastEditTime: 2025-02-08 14:57:01
 * @FilePath: \nc-link-v2-adapter\components\utils\queue.h
 * @Description:
 * Copyright (c) 2025 by xhzl , All Rights Reserved.
 */

#ifndef __MY_QUEUE_H__
#define __MY_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{

    uint8_t *buf;       // 队列缓冲区
    uint32_t data_szie; // 数据
    uint32_t size;      // 队列大小
    uint32_t head;      // 头指针
    uint32_t tail;      // 尾指针

} queue_t;

void *queue_create(uint32_t size, uint32_t data_format_size);
void queue_push(void *queue, void *data);
bool queue_pop(void *queue, void *data);
bool queue_is_empty(void *queue);
void queue_clear(void *queue);
void queue_free(void *queue);
void queue_push_len(void *queue, void *data, uint32_t data_len);
uint32_t queue_size(void *queue);

#endif