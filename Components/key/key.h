#ifndef _KEY_H_
#define _KEY_H_

#include <stdint.h>

/**
 * @brief 按键ID枚举
 */
typedef enum
{ 
   KEY_UP 	 = 0,  //上键
   KEY_DOWN     ,  //下键
   KEY_ID_MAX
}KEY_ID_E;

/**
 * @brief 按键状态枚举
 */
typedef enum 
{
   KEY_PRESS_NONE = 0,      //按键没有动作
   KEY_PRESS_CLICK ,        //单击
   KEY_PRESS_DOUBLE_CLICK,  //双击
   KEY_PRESS_LONG,          //长按

}KEY_PRESS_STA_E;

/**
 * @brief 按键扫描结构体
 */
typedef struct
{
    uint8_t is_press;               //按键按下标识
    uint32_t press_s_tick;          //按键按下开始时间戳
    uint32_t press_r_tick;          //按键释放时间戳（用于双击检测）
    uint32_t scan_step;
    uint32_t (*key_scan_handle)(void); //按键扫描函数指针
} key_scan_s;  

/**
 * @brief 按键数据结构体
 */
typedef struct 
{
   uint16_t id;                       //按键ID 
   KEY_PRESS_STA_E press_state;  //按键状态  
}key_data_s;

/**
 * @brief 按键扫描任务
 * @param param 任务参数
 */
extern void key_scan_task(void *param);
KEY_PRESS_STA_E key_get_state(KEY_ID_E key_id);


#endif