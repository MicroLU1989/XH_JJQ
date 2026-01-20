
#include <stdint.h>

#include "os_port.h"
#include "os_config.h"
#include "user_log.h"

#if (OS_TYPE == FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "timers.h"
#endif

uint32_t os_tick_ms_get(void)
{
#if (OS_TYPE == FREERTOS)
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
}

void os_task_mdelay(uint32_t ms)
{
#if (OS_TYPE == FREERTOS)
  vTaskDelay(ms / portTICK_PERIOD_MS);
#endif
}

uint32_t os_delay_tick_get(uint32_t ms)
{
#if (OS_TYPE == FREERTOS)
  return ms > portTICK_PERIOD_MS ? ms / portTICK_PERIOD_MS : 1;
#endif
}
void *os_task_create(const char *name, void (*task_entry)(void *param), void *param, int stack_size, int priority, int tick)
{
#if (OS_TYPE == FREERTOS)
  tick = tick;
  static TaskHandle_t task_handle; // 任务句柄
 BaseType_t xReturned = xTaskCreate(task_entry,
              name,
              stack_size,
              param,
              priority,
              &task_handle);
	if(xReturned != pdPASS)
	{
		return NULL;
	}
  return task_handle;
#endif
}

void os_task_delete(void *task_handle)
{
#if (OS_TYPE == FREERTOS)
  vTaskDelete((TaskHandle_t)task_handle);
#endif
}

// 将任务创建到指定内核, 适用多核芯片
void *os_task_create_to_core(const char *name, void (*task_entry)(void *param), void *param, int stack_size, int priority, int tick, int core_num)
{
#if 0
    tick = tick;
    static TaskHandle_t task_handle; // 任务句柄
    xTaskCreatePinnedToCore(task_entry, name, stack_size, param, priority, &task_handle, core_num);
    return task_handle;
#endif
  return NULL;
}

// task_handle 任务句柄
void os_task_startup(void *task_handle)
{
#if (OS_TYPE == FREERTOS)
  task_handle = task_handle;
#endif
}

void os_task_suspend(void *task_handle)
{
#if (OS_TYPE == FREERTOS)
  vTaskSuspend(task_handle);
#endif
}

void os_task_resume(void *task_handle)
{
#if (OS_TYPE == FREERTOS)
  vTaskResume(task_handle);
#endif
}

void *os_event_create(const char *name)
{
#if (OS_TYPE == FREERTOS)
  name = name;
  EventGroupHandle_t event = xEventGroupCreate();
  return event;
#endif
}

void *os_sem_create(const char *name)
{
#if (OS_TYPE == FREERTOS)
  name = name;
  SemaphoreHandle_t sem = xSemaphoreCreateCounting(65535, 0);
  return sem;
#endif
}

void *os_mutex_create(const char *name)
{
#if (OS_TYPE == FREERTOS)
  name = name;
  SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
  return mutex;
#endif
}

int os_mutex_take(void *mutex, int timeout)
{
#if (OS_TYPE == FREERTOS)
  uint32_t delay = timeout < 0 ? portMAX_DELAY : timeout / portTICK_PERIOD_MS;
  int ret = (int)xSemaphoreTake((SemaphoreHandle_t)(mutex), delay);
  return ret;
#endif
}

int os_mutex_release(void *mutex)
{
#if (OS_TYPE == FREERTOS)
  int ret = (int)xSemaphoreGive((SemaphoreHandle_t)mutex);
  return ret;
#endif
}

int os_sem_take(void *sem, int timeout)
{
#if (OS_TYPE == FREERTOS)
  uint32_t delay = timeout < 0 ? portMAX_DELAY : timeout / portTICK_PERIOD_MS;
  int ret = (int)xSemaphoreTake((SemaphoreHandle_t)sem, delay);
  return ret;
#endif
}

void os_sem_reset(void *sem)
{
#if (OS_TYPE == FREERTOS)
  sem = sem;
#endif
}

int os_sem_release(void *sem)
{
#if (OS_TYPE == FREERTOS)
  int ret = (int)xSemaphoreGive((SemaphoreHandle_t)sem);
  return ret;
#endif
}

// 中断中调用
int os_sem_release_from_isr(void *sem, int *pxHigherPriorityTaskWoken)
{
#if (OS_TYPE == FREERTOS)
    int ret = (int)xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, 
                                         (BaseType_t *)pxHigherPriorityTaskWoken);
    return ret;
#endif
    // 其他OS的默认返回
    return -1;
}

int os_event_recv(void *event, int event_bit, int timeout, int recv_option, unsigned int *event_ret)
{
#if (OS_TYPE == FREERTOS)
  uint32_t delay = timeout < 0 ? portMAX_DELAY : timeout / portTICK_PERIOD_MS;
  BaseType_t clearflg = (recv_option & OS_EVENT_RECV_CLEAR) ? pdTRUE : pdFALSE;
  BaseType_t WaitForAllBits = (recv_option & OS_EVENT_RECV_AND) ? pdTRUE : pdFALSE;
  EventBits_t eventbit = xEventGroupWaitBits((EventGroupHandle_t)event, event_bit, clearflg, WaitForAllBits, delay);
  if (event_ret != NULL)
  {
    *event_ret = (unsigned int)eventbit;
  }
  return 1;
#endif
}

int os_event_send(void *event, int event_bit)
{
#if (OS_TYPE == FREERTOS)
  int ret = (int)xEventGroupSetBits((EventGroupHandle_t)event, event_bit);
  return ret;
#endif
}

void *os_timer_create(const char *name, void (*timeout_cb)(void *param), void *param, int mtick)
{
#if (OS_TYPE == FREERTOS)
  TimerHandle_t timhandle = xTimerCreate(name,
                                         pdMS_TO_TICKS(mtick),
                                         pdTRUE,
                                         (void *)1,
                                         (TimerCallbackFunction_t)timeout_cb);
  return timhandle;
#endif
}

void os_timer_start(void *timer)
{
#if (OS_TYPE == FREERTOS)
  if (timer == NULL)
  {
    log_e("timer is NULL \r\n");
  }
  xTimerStart((TimerHandle_t)timer, 0); // 启动定时器
#endif
}

// 消息队列创建
void *os_msg_queue_create(const char *name, void *mqpool_buffer, unsigned int mqpool_size, int msg_size, int flg)
{
#if (OS_TYPE == FREERTOS)
  name = name;
  mqpool_buffer = mqpool_buffer;
  flg = flg;
  QueueHandle_t msg_queue = xQueueCreate(mqpool_size, msg_size);
  return msg_queue;
#endif
}

int os_msg_queue_send(void *msg_queue, void *msg, int msg_size, int timeout)
{
#if (OS_TYPE == FREERTOS)
  msg_size = msg_size;
  int delay = timeout < 0 ? portMAX_DELAY : timeout / portTICK_PERIOD_MS;
  return (int)xQueueSend((QueueHandle_t)msg_queue, msg, delay);
#endif
}

int os_msg_queue_recv(void *msg_queue, void *msg, int timeout)
{
#if (OS_TYPE == FREERTOS)
  int delay = timeout < 0 ? portMAX_DELAY : timeout / portTICK_PERIOD_MS;
  return (int)xQueueReceive((QueueHandle_t)msg_queue, msg, delay);
#endif
}

#if 0
static portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;
void os_enter_critical(void)
{
    taskENTER_CRITICAL(&my_spinlock);
}

void os_exit_critical(void)
{
    taskEXIT_CRITICAL(&my_spinlock);
}
#else
void os_enter_critical(void)
{
#if (OS_TYPE == FREERTOS)
  taskENTER_CRITICAL();
#endif
}


void os_sart_scheduler(void)
{
#if (OS_TYPE == FREERTOS)
   vTaskStartScheduler();
#endif
  
}


void os_exit_critical(void)
{
#if (OS_TYPE == FREERTOS)
  taskEXIT_CRITICAL();
#endif
}
#endif

void os_free(void *ptr)
{

#ifndef SYS_USING_HEAP
  return;
#endif

  if (ptr != NULL)
  {
#if (OS_TYPE == FREERTOS)
    vPortFree(ptr);
#endif
    ptr = NULL;
  }
}

void *os_malloc(unsigned int size)
{
#ifndef SYS_USING_HEAP
  return NULL;
#endif

#if (OS_TYPE == FREERTOS)
  return pvPortMalloc(size);
#endif
}

size_t os_get_free_heapsize(void)
{

#ifndef SYS_USING_HEAP
  return 0;
#endif
#if (OS_TYPE == FREERTOS)
  return xPortGetFreeHeapSize();
#endif
}
