
#ifndef _OS_PORT_H_
#define _OS_PORT_H_

#include "os_config.h"

#include <stdint.h>
#include <stdlib.h>


#define OS_RET_OK               1 // pdTRUE
#define OS_RET_ERR              0 //pdFALSE

#define  OS_MAX_DELAY           (-1)   //最大延时,相当于永久等待,直到某个条件满足后恢复调度

#define OS_EVENT_RECV_AND       0x01   //等待所有事件满足,逻辑与    
#define OS_EVENT_RECV_OR        0x02   //等待其中一个事件,逻辑或
#define OS_EVENT_RECV_CLEAR     0x04   //接收完成后,清除事件


#define OS_EVENT_BIT31      0x80000000
#define OS_EVENT_BIT30      0x40000000
#define OS_EVENT_BIT29      0x20000000
#define OS_EVENT_BIT28      0x10000000
#define OS_EVENT_BIT27      0x08000000
#define OS_EVENT_BIT26      0x04000000
#define OS_EVENT_BIT25      0x02000000
#define OS_EVENT_BIT24      0x01000000
#define OS_EVENT_BIT23      0x00800000
#define OS_EVENT_BIT22      0x00400000
#define OS_EVENT_BIT21      0x00200000
#define OS_EVENT_BIT20      0x00100000
#define OS_EVENT_BIT19      0x00080000
#define OS_EVENT_BIT18      0x00040000
#define OS_EVENT_BIT17      0x00020000
#define OS_EVENT_BIT16      0x00010000
#define OS_EVENT_BIT15      0x00008000
#define OS_EVENT_BIT14      0x00004000
#define OS_EVENT_BIT13      0x00002000
#define OS_EVENT_BIT12      0x00001000
#define OS_EVENT_BIT11      0x00000800
#define OS_EVENT_BIT10      0x00000400
#define OS_EVENT_BIT9       0x00000200
#define OS_EVENT_BIT8       0x00000100
#define OS_EVENT_BIT7       0x00000080
#define OS_EVENT_BIT6       0x00000040
#define OS_EVENT_BIT5       0x00000020
#define OS_EVENT_BIT4       0x00000010
#define OS_EVENT_BIT3       0x00000008
#define OS_EVENT_BIT2       0x00000004
#define OS_EVENT_BIT1       0x00000002
#define OS_EVENT_BIT0       0x00000001






uint32_t os_tick_ms_get(void);

void os_task_mdelay(uint32_t ms);

void *os_task_create(const char *name, void(*task_entry)(void *param), void *param,  int stack_size, int priority,int tick);

void os_task_delete(void *task_handle);

void *os_task_create_to_core(const char *name, void(*task_entry)(void *param), void *param,  int stack_size, int priority,int tick,int core_num);

void os_task_startup(void *task_handle);

void os_task_suspend(void *task_handle);

void os_task_resume(void *task_handle);

void *os_event_create(const char *name);

void *os_sem_create(const char *name);

void *os_mutex_create(const char *name);

int os_mutex_take(void *mutex, int timeout);

int os_mutex_release(void *mutex);

int os_sem_take(void *sem, int timeout);

void os_sem_reset(void *sem);

int os_sem_release(void *sem);

int os_event_recv(void *event, int event_bit, int timeout, int recv_option,unsigned int *resevent);

int os_event_send(void *event, int event_bit);

void *os_timer_create(const char *name, void (*timeout_cb)(void *param), void *param, int mtick);

void os_timer_start(void *timer);

void *os_msg_queue_create(const char *name, void *mqpool_buffer, unsigned int mqpool_size, int msg_size, int flg);

int os_msg_queue_send(void *msg_queue, void *msg, int msg_size, int timeout);

int os_msg_queue_recv(void *msg_queue, void *msg, int timeout);

int os_sem_release_from_isr(void *sem, int *pxHigherPriorityTaskWoken);

uint32_t os_delay_tick_get(uint32_t ms);

void os_enter_critical(void); 	//进入临界区

void os_exit_critical(void); 	//退出临界区

void os_free(void *ptr);

void *os_malloc(unsigned int size);

size_t os_get_free_heapsize(void);

void os_sart_scheduler(void); //启动调度器

#endif

