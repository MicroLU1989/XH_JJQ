#include <stdio.h>
#include "key.h"
#include "gpio_driver.h"
#include "user_log.h"
#include "os_port.h"
#include "my_queue.h"
#include "buzzer.h"

//外部按键事件处理函数
extern void menu_key_press_handle(uint8_t key_id);


// --- 配置宏 ---
#define PRESS_LONG_TICK (1500)      // 长按时间阈值 (ms)
#define DEBOUNCE_TICK (30)          // 消抖时间 (ms)
#define DOUBLE_CLICK_INTERVAL (100) // 两次单击间的最大间隔 (ms)

// --- 按键硬件定义 ---
#define KEY_GPIO_PORT   GPIO_PORT_B
#define KEY_1_PIN       GPIO_PIN_NUM(0)
#define KEY_2_PIN       GPIO_PIN_NUM(1)


// --- 类型与枚举定义 (增强可读性与类型安全) ---
typedef enum
{
    KEY_STATE_IDLE,                 // 空闲状态
    KEY_STATE_DEBOUNCING_PRESS,     // 按下消抖中
    KEY_STATE_PRESSED,              // 已确认按下
    KEY_STATE_DEBOUNCING_RELEASE,   // 松开消抖中
    KEY_STATE_WAIT_FOR_DOUBLE_CLICK // 等待双击中
} key_fsm_state_e;

typedef struct
{
    key_fsm_state_e state;           // 当前FSM状态
    uint32_t timestamp;              // 进入当前状态的时刻 (tick)
    uint32_t release_timestamp;      // 最近一次有效松开的时刻 (tick)，用于双击检测
    uint32_t press_start_timestamp;  // 按键最初按下的时刻 (用于计算总时长)
    uint8_t long_press_reported;     // 长按事件是否已报告标志 (0:未报告, 1:已报告)
    KEY_PRESS_STA_E event_to_report; // 待上报的事件
} key_fsm_s;

// --- 静态变量 ---
static void *key_queue = NULL;  // 按键事件队列
static void *key_sem = NULL;    // 按键事件信号量

static key_fsm_s key_fsm_tab[KEY_ID_MAX]; // 每个按键一个独立的FSM实例

// --- 私有函数声明 ---
static uint32_t key_1_scan_hdl(void);
static uint32_t key_2_scan_hdl(void);

// 按键扫描句柄表
key_scan_s key_scan_tab[KEY_ID_MAX] = {
    {0, 0, 0, 0, key_1_scan_hdl},
    {0, 0, 0, 0, key_2_scan_hdl},
};

// 对外暴露的按键数据（由FSM驱动）
key_data_s key_data_tab[KEY_ID_MAX] = {
    {KEY_UP, KEY_PRESS_NONE},
    {KEY_DOWN, KEY_PRESS_NONE},
};

// --- 函数实现 ---

/**
 * @brief 获取按键状态
 * @param key_id 按键ID
 * @return 按键状态
 */
KEY_PRESS_STA_E key_get_state(KEY_ID_E key_id)
{
    if (key_id >= KEY_ID_MAX)
        return KEY_PRESS_NONE;
    // 返回已确认的事件，调用者处理后应将状态清空
    KEY_PRESS_STA_E state = key_data_tab[key_id].press_state;
    // key_data_tab[key_id].press_state = KEY_PRESS_NONE; // 可选：获取后即清除
    return state;
}

/**
 * @brief 初始化按键GPIO
 */
static void key_init(void)
{
    drv_gpio_init(KEY_GPIO_PORT, KEY_1_PIN | KEY_2_PIN, PIN_MODE_IN_FLOATING, PIN_SPEED_10M);
}

/**
 * @brief 上键扫描 (假设低电平有效)
 */
static uint32_t key_1_scan_hdl(void)
{
    return drv_gpio_read(KEY_GPIO_PORT, KEY_1_PIN, PIN_DIRECTION_IN) ? 1 : 0;
}

/**
 * @brief 下键扫描 (假设低电平有效)
 */
static uint32_t key_2_scan_hdl(void)
{
    return drv_gpio_read(KEY_GPIO_PORT, KEY_2_PIN, PIN_DIRECTION_IN) ? 1 : 0;
}


bool kay_data_read(uint32_t timeout,key_data_s key_data)
{
    int ret = os_sem_take(key_sem,timeout);
    if (!queue_is_empty(key_queue))
    {
       return queue_pop(key_queue, &key_data);
    }
		return false;
}


/**
 * @brief 核心按键状态机处理函数
 * @param key_id 按键ID
 * @param current_level 当前扫描到的物理电平 (1:按下有效, 0:松开)
 */
static void key_fsm_process(KEY_ID_E key_id, uint8_t current_level)
{
    key_fsm_s *fsm = &key_fsm_tab[key_id];
    uint32_t current_tick = os_tick_ms_get();

    switch (fsm->state)
    {
    case KEY_STATE_IDLE:
        // 在空闲态，检测到按下
        if (current_level == 1)
        {
            fsm->timestamp = current_tick; // 记录进入消抖态的时间
            fsm->state = KEY_STATE_DEBOUNCING_PRESS;
            // log_d("KEY %d FSM: Enter DEBOUNCING_PRESS\n", key_id); // 调试日志，可按需开启
        }
        break;

    case KEY_STATE_DEBOUNCING_PRESS:
        // 在消抖态，检查是否稳定按下超过消抖时间
        if (current_level == 1)
        {
            if (current_tick - fsm->timestamp >= DEBOUNCE_TICK)
            {
                // 消抖完成，确认按下
                fsm->state = KEY_STATE_PRESSED;
                fsm->timestamp = current_tick;             // 重置为连发计时或长按计时起点
                fsm->press_start_timestamp = current_tick; // 【关键】记录按键最初按下的绝对时间
                fsm->long_press_reported = 0;              // 【关键】重置长按已报告标志
                // log_d("KEY %d FSM: Press Confirmed. Start timers.\n", key_id);
            }
        }
        else
        {
            // 在消抖期间松开了，视为抖动，回到空闲
            fsm->state = KEY_STATE_IDLE;
            // log_d("KEY %d FSM: Debounce failed, back to IDLE.\n", key_id);
        }
        break;

    case KEY_STATE_PRESSED:
        // 在按下态，检测松开或长按
        if (current_level == 0)
        {
            // 松开了，进入松开消抖
            fsm->timestamp = current_tick; // 记录进入松开消抖的时间
            fsm->state = KEY_STATE_DEBOUNCING_RELEASE;
            // log_d("KEY %d FSM: Enter DEBOUNCING_RELEASE\n", key_id);
        }
        else
        {
            // 仍然按下，检查是否达到长按时间
            // 使用 press_start_timestamp 判断是否达到长按，并用 long_press_reported 控制是否重复触发
            if (!fsm->long_press_reported && (current_tick - fsm->press_start_timestamp >= PRESS_LONG_TICK))
            {

                // --- 长按事件处理 (单次触发) ---
                key_data_tab[key_id].press_state = KEY_PRESS_LONG;
                key_data_tab[key_id].id = key_id;
                queue_push(key_queue, &key_data_tab[key_id]);
                os_sem_release(key_sem); // 通知按键处理线程
                log_i("KEY %d 长按触发\n", key_id);
                // 设置标志位，防止在持续按下的过程中重复触发长按事件
                fsm->long_press_reported = 1;
                // 【可选】如果需求是长按连发，则在这里更新timestamp，并移除上面的标志位逻辑
                // fsm->timestamp = current_tick; // 重置计时器，准备下次连发
                // log_i("KEY %d 长按连发...\n", key_id);
            }
        }
        break;

    case KEY_STATE_DEBOUNCING_RELEASE:
        // 在松开消抖态，检查是否稳定松开
        if (current_level == 0)
        {
            if (current_tick - fsm->timestamp >= DEBOUNCE_TICK)
            {
                // --- 松开消抖完成，根据总按下时长判断事件类型 ---

                // 【关键修复】使用 press_start_timestamp 计算总的按下时长
                uint32_t total_press_duration = current_tick - fsm->press_start_timestamp;

                // 检查这次按下是否满足单击条件 (非长按)
                if (total_press_duration < PRESS_LONG_TICK)
                {
                    // 是有效的单击，进入等待双击状态
                    fsm->release_timestamp = current_tick; // 记录本次松开的时间
                    fsm->state = KEY_STATE_WAIT_FOR_DOUBLE_CLICK;
                    // log_d("KEY %d FSM: Release Confirmed. Enter WAIT_FOR_DOUBLE_CLICK\n", key_id);
                }
                else
                {
                    // 这是一个长按后的松开，直接回到空闲
                    fsm->state = KEY_STATE_IDLE;
                    log_i("KEY %d 长按结束\n", key_id); // 这条日志现在会被正确打印
                }
            }
        }
        else
        {
            // 在松开消抖期间又按下了，说明之前的松开是抖动，回到按下态
            fsm->state = KEY_STATE_PRESSED;
            // 注意：这里不需要重置 press_start_timestamp，因为它是按键总的开始时间
            // 但可能需要重置长按计时器，取决于具体需求
            // fsm->timestamp = current_tick;
            // log_d("KEY %d FSM: Release debounce failed, back to PRESSED\n", key_id);
        }
        break;

    case KEY_STATE_WAIT_FOR_DOUBLE_CLICK:
        // 在等待双击态，检测新的按下或超时
        if (current_level == 1)
        {
            // 在规定时间内再次按下，触发双击
            if (current_tick - fsm->release_timestamp <= DOUBLE_CLICK_INTERVAL)
            {
                key_data_tab[key_id].press_state = KEY_PRESS_DOUBLE_CLICK;
                key_data_tab[key_id].id = key_id;
                queue_push(key_queue, &key_data_tab[key_id]);
                os_sem_release(key_sem); // 通知按键处理线程
                log_i("KEY %d 双击触发\n", key_id);
                // 进入新的按下消抖流程
                fsm->state = KEY_STATE_DEBOUNCING_PRESS;
                fsm->timestamp = current_tick;
            }
            else
            {
                // 超时了才按，这是一次新的单击的开始。我们先把旧的单击确认掉。
                key_data_tab[key_id].press_state = KEY_PRESS_CLICK;
                key_data_tab[key_id].id = key_id;
                queue_push(key_queue, &key_data_tab[key_id]);
                menu_key_press_handle(key_id);
                // os_sem_release(key_sem); // 通知按键处理线程
                buzzer_prompt_load(BUZZER_PROMPT_KEY);
                log_i("KEY %d 单击确认 (new press after timeout)\n", key_id);
                // 然后立即处理新的按下事件，进入消抖
                fsm->state = KEY_STATE_DEBOUNCING_PRESS;
                fsm->timestamp = current_tick;
            }
        }
        else
        {
            // 仍然没有按下，检查是否超时
            if (current_tick - fsm->release_timestamp >= DOUBLE_CLICK_INTERVAL)
            {
                // 双击超时，确认单击
                key_data_tab[key_id].press_state = KEY_PRESS_CLICK;
                key_data_tab[key_id].id = key_id;
                queue_push(key_queue, &key_data_tab[key_id]);
                menu_key_press_handle(key_id);
                //  os_sem_release(key_sem); // 通知按键处理线程
                buzzer_prompt_load(BUZZER_PROMPT_KEY);
                log_i("KEY %d 单击确认 (timeout)\n", key_id);
                fsm->state = KEY_STATE_IDLE;
            }
        }
        break;
    }
}

/**
 * @brief 按键扫描任务 (主循环)
 */
void key_scan_task(void *param)
{
    key_init();
    key_queue = queue_create(KEY_ID_MAX * 3, sizeof(key_data_s));
    key_sem = os_sem_create("keysem"); // 如果需要事件组可以保留
    // 初始化所有按键的FSM
    for (int i = 0; i < KEY_ID_MAX; i++)
    {
        key_fsm_tab[i].state = KEY_STATE_IDLE;
        key_fsm_tab[i].event_to_report = KEY_PRESS_NONE;
        key_fsm_tab[i].long_press_reported = 0;   // 【新增】初始化标志位
        key_fsm_tab[i].press_start_timestamp = 0; // 【新增】初始化时间戳
    }

    while (1)
    {
        os_task_sleep(10); // 10ms扫描周期，足够快以捕获消抖

        for (uint16_t index = 0; index < KEY_ID_MAX; index++)
        {
            if (key_scan_tab[index].key_scan_handle == NULL)
            {
                continue;
            }
            uint8_t current_level = key_scan_tab[index].key_scan_handle();
            key_fsm_process((KEY_ID_E)index, current_level);
        }
    }
}