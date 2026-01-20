/**************************************************************************
 * @FilePath     : /software/components/user_log/user_log.h
 * @Description  :
 * @Author       : MicroLU1989 916739459@qq.com
 * @Version      : 0.0.1
 * @LastEditors  : MicroLU1989 916739459@qq.com
 * @LastEditTime : 2024-12-10 15:46:16
 * @Copyright    :
***************************************************************************/
#ifndef _USER_LOG_H_
#define _USER_LOG_H_

#include <string.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

    // 16进制打印宽度， 32表示每32字节打印一行
#define LOG_HEX_PRINT_LINE_WIDTH 16

//日志打印级别 0 最高
#define LOG_LVL_ERROR                       0
#define LOG_LVL_WARN                        1
#define LOG_LVL_DEBUG                       2
#define LOG_LVL_INFO                        3


//是否显示颜色
#define PRINT_COLOR   

//是否显示时间
//#define PRINT_TIME

//自动添加换行
#define PRINT_AUTO_ADD_NEWLINE



#ifdef  PRINT_COLOR
#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m"
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"

#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"25 
#define CLRLINE              "\r\e[K" //or "\e[1K\r"

//定义不同级别日志打印颜色
#define LOG_D_COLOR           CYAN
#define LOG_W_COLOR           YELLOW
#define LOG_I_COLOR           GREEN
#define LOG_E_COLOR           RED
#endif

// 更智能的定义，提取纯文件名
#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : \
                     (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))
#endif

extern void log_print(unsigned char level, const char* file, const long line, const char* format, ...);


#define log_e(...) log_print(LOG_LVL_ERROR,__FILENAME__,__LINE__,__VA_ARGS__)

#define log_w(...) log_print(LOG_LVL_WARN,__FILENAME__,__LINE__,__VA_ARGS__)

#define log_d(...) log_print(LOG_LVL_DEBUG,__FILENAME__,__LINE__,__VA_ARGS__)

#define log_i(...) log_print(LOG_LVL_INFO,__FILENAME__,__LINE__,__VA_ARGS__)


    extern void log_set_print_enable(void);
    extern void log_set_print_disable(void);
    extern  void log_set_hex_print_line_width(int width);
    extern  void log_set_print_level(int level);
    extern  void log_hexdump(const uint8_t* src, uint16_t len);
    extern void log_output(char* logstr, int len);
    extern void log_init(void);

#ifdef __cplusplus
}
#endif

#endif

