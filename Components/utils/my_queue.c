/**************************************************************************
 * @FilePath     : /nc-link-v2-adapter/components/utils/queue.c
 * @Description  :
 * @Author       : MicroLU1989 916739459@qq.com
 * @Version      : 0.0.1
 * @LastEditors  : MicroLU1989 916739459@qq.com
 * @LastEditTime : 2024-12-19 11:33:28
 * @Copyright    :
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_queue.h"
#include "user_log.h"
#include "os_port.h"

void *queue_create(uint32_t size, uint32_t data_format_size)
{
#ifndef SYS_USING_HEAP
    return NULL;
#endif
    queue_t *p_queue = (queue_t *)os_malloc(sizeof(queue_t));
    if (p_queue == NULL)
    {
        log_e("malloc error");
        return NULL;
    }
    p_queue->buf = (uint8_t *)os_malloc(size * data_format_size);
    if (p_queue->buf == NULL)
    {
        log_e("malloc error");
        os_free(p_queue);
        return NULL;
    }
    p_queue->size = size;
    p_queue->data_szie = data_format_size;
    p_queue->head = 0;
    p_queue->tail = 0;
    return p_queue;
}

void queue_push(void *queue, void *data)
{
    queue_t *p_queue = (queue_t *)queue;
    if (p_queue == NULL || data == NULL)
    {
        return;
    }
    if (p_queue->data_szie == 1)
    {
        p_queue->buf[p_queue->tail * p_queue->data_szie] = *((uint8_t *)data);
    }
    else
    {
        memcpy(p_queue->buf + p_queue->tail * p_queue->data_szie, data, p_queue->data_szie);
    }
    p_queue->tail = (p_queue->tail + 1) % p_queue->size;
    if (p_queue->tail == p_queue->head)
    {
        p_queue->head = (p_queue->head + 1) % p_queue->size;
    }
}

void queue_push_len(void *queue, void *data, uint32_t data_len)
{
    queue_t *p_queue = (queue_t *)queue;
    if (p_queue == NULL || data == NULL || data_len == 0)
    {
        return;
    }
    for (int i = 0; i < data_len; i++)
    {
        if (p_queue->data_szie == 1)
        {
            p_queue->buf[p_queue->tail * p_queue->data_szie] = *((uint8_t *)data++);
        }
        else
        {
            memcpy(p_queue->buf + p_queue->tail * p_queue->data_szie, data, p_queue->data_szie);
        }
        p_queue->tail = (p_queue->tail + 1) % p_queue->size;
        if (p_queue->tail == p_queue->head)
        {
            p_queue->head = (p_queue->head + 1) % p_queue->size;
        }
    }
}

bool queue_pop(void *queue, void *data)
{
    queue_t *p_queue = (queue_t *)queue;
    if (p_queue == NULL || data == NULL)
    {
        return false;
    }
    if (p_queue->head == p_queue->tail)
    {
        p_queue->head = p_queue->tail = 0;
        return false;
    }
    memcpy(data, p_queue->buf + p_queue->head * p_queue->data_szie, p_queue->data_szie);
    p_queue->head = (p_queue->head + 1) % p_queue->size;
    return true;
}

bool queue_is_empty(void *queue)
{
    if (queue == NULL)
    {
        return true;
    }
    return ((queue_t *)queue)->head == ((queue_t *)queue)->tail;
}

void queue_free(void *queue)
{
#ifndef SYS_USING_HEAP
    return;
#endif
    if (queue == NULL)
    {
        return;
    }
    if (((queue_t *)queue)->buf != NULL)
    {
        os_free(((queue_t *)queue)->buf);
    }
    os_free(queue);
    queue = NULL;
}

void queue_clear(void *queue)
{
    if (queue == NULL)
    {
        return;
    }
    ((queue_t *)queue)->head = 0;
    ((queue_t *)queue)->tail = 0;
}

uint32_t queue_size(void *queue)
{
    if (queue == NULL)
    {
        return 0;
    }
    queue_t *p = (queue_t *)queue;
    if (p->tail >= p->head)
    {
        return p->tail - p->head;
    }
    else
    {
        return p->size - p->head + p->tail;
    }
}