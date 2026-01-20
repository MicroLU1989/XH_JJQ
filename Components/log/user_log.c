
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "user_log.h"
#include <time.h>
#include "device.h"




#define LOG_PRINT_DATA_SZIE  1000
#define LOG_PRINT_DEBUG_SZIE LOG_PRINT_DATA_SZIE + 96
static char data_buf[LOG_PRINT_DATA_SZIE];
static char debug_buf[LOG_PRINT_DEBUG_SZIE];


static int hex_print_line_width = LOG_HEX_PRINT_LINE_WIDTH;

static bool log_enable_flg = true;

static int log_print_level = LOG_LVL_INFO;  // 日志打印级别

static const char* level_output_info[] = {
    [LOG_LVL_ERROR] = "E",
    [LOG_LVL_WARN] = "W",
    [LOG_LVL_INFO] = "I",
    [LOG_LVL_DEBUG] = "D",
};



void log_enable(void)
{
    log_enable_flg = true;
}
void log_disable(void)
{
    log_enable_flg = false;
}


#ifdef PRINT_TIME
char time_buf[32];

static char* time_get(void)
{
    struct tm* tm_now;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tm_now = localtime(&tv.tv_sec);
    memset(time_buf, 0, sizeof(time_buf));
    sprintf(time_buf, "%04d-%02d-%02d %02d:%02d:%02d.%03ld", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
        tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tv.tv_usec / 1000);
    return time_buf;
}
#endif

void log_output(char* logstr, int len)
{
    // fwrite(logstr, 1, len, stdout);
    // fflush(stdout);
    static struct device_t* log_device = NULL;
    if(log_device == NULL)
    {
       log_device = device_get_obj(DEVICE_TYPE_USB, "USB0");
    }
    else
    {
        device_write(log_device,NULL, (uint8_t *)logstr, len);
    }
}

void log_print(unsigned char level, const char* file, const long line, const char* format, ...)
{
    int len = 0;
    int data_len = 0;
    int cnt = 0;
    va_list args;
    if (log_enable_flg == false)
    {
        return;
    }
    if (level > log_print_level)
    {
        return;
    }

#ifdef PRINT_TIME
    char* time = time_get();
#endif
    // memset(debug_buf, 0, LOG_PRINT_DEBUG_SZIE);
    // memset(data_buf, 0, LOG_PRINT_DATA_SZIE);
    va_start(args, format);
    data_len = vsnprintf(data_buf, (LOG_PRINT_DATA_SZIE - 1), format, args);
    va_end(args);
    for (int i = 0; i < data_len; i++)
    {
        if (data_buf[i] != '\n') //计算数据前面有几个\n
        {
            break;
        }
        cnt++;
    }
    len += cnt;
    switch (level)
    {
    case LOG_LVL_ERROR:
#ifdef PRINT_COLOR
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], LOG_E_COLOR "|%s|-%c-[%s:%ld]", time, level_output_info[level], file, line);
#else
        len += sprintf(&debug_buf[len], LOG_E_COLOR "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif

#else
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], "|%s|-%c-[%s:%ld]", time, level_output_info[level], file,line);
#else
        len += sprintf(&debug_buf[len], "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif
#endif
        break;

    case LOG_LVL_WARN:
#ifdef PRINT_COLOR
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], LOG_W_COLOR "|%s|-%c-[%s:%ld]", time, level_output_info[level], file, line);
#else
        len += sprintf(&debug_buf[len], LOG_W_COLOR "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif
#else
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], "|%s|-%c-[%s:%ld]", time, level_output_info[level], file, line);
#else
        len += sprintf(&debug_buf[len], "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif
#endif
        break;

    case LOG_LVL_DEBUG:
#ifdef PRINT_COLOR
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], LOG_D_COLOR "|%s|-%c-[%s:%ld]", time, level_output_info[level],file,line);
#else
        len += sprintf(&debug_buf[len], LOG_D_COLOR "-%s-[%s:%ld]", level_output_info[level],file, line);
#endif
#else

#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], "|%s|-%c-[%s:%ld]", time, level_output_info[level], file, line);
#else
        len += sprintf(&debug_buf[len], "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif

#endif
        break;

    case LOG_LVL_INFO:

#ifdef PRINT_COLOR
#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], LOG_I_COLOR "|%s|-%c-[%s:%ld]", time, level_output_info[level],file,line);
#else
        len += sprintf(&debug_buf[len], LOG_I_COLOR "-%s-[%s:%ld]", level_output_info[level],file, line);
#endif
#else

#ifdef PRINT_TIME
        len += sprintf(&debug_buf[len], "|%s|-%c-[%s:%ld]", time, level_output_info[level], file, line);
#else
        len += sprintf(&debug_buf[len], "-%s-[%s:%ld]", level_output_info[level], file, line);
#endif

#endif
        break;

    default:
        break;
    }
    // tmp_iindex = len;
    if (len + data_len > LOG_PRINT_DEBUG_SZIE - 1)
    {
        log_output("\n*debug*len*err*\n", 17);
        return;
    }
    if(cnt != 0)
    {
       memset(debug_buf,'\n',cnt);
    }
    memcpy(&debug_buf[len],&data_buf[cnt],data_len - cnt);
    len += data_len - cnt;
#ifdef PRINT_AUTO_ADD_NEWLINE
    if (debug_buf[len - 1] != '\n')
    {
        debug_buf[len++] = '\n';
    }
#endif
    debug_buf[len] = 0;
    log_output(debug_buf, len);
}

// 设置打印颜色
void log_set_print_color(char* color)
{
    if (color == NULL)
    {
        return;
    }
#ifdef PRINT_COLOR
    // fwrite(color, 1, strlen(color), stdout);
    // fflush(stdout);
#else
    color = color;
#endif
}


const unsigned char hex_str[] = "0123456789ABCDEF";
void log_hexdump(const uint8_t* src, uint16_t len)
{
    int i, j;
    int width;
#ifdef PRINT_COLOR
     log_set_print_color(L_BLUE);
#endif
  
    if (log_enable_flg == false)
    {
        return;
    }
    if (src == NULL || len <= 0)
    {
        return;
    }
    if (hex_print_line_width > 128)
    {
        hex_print_line_width = 128;
    }
    width = hex_print_line_width * 3;  // 每行打印的字符数
    for (i = 0,j = 0; i < len; i++)
    {
        debug_buf[j++] = hex_str[src[i] >> 4];
        debug_buf[j++] = hex_str[src[i] & 0x0f];
        debug_buf[j++] = ' ';  // 空格
        if (j == width)
        {
            debug_buf[j++] = '\n';
            log_output(debug_buf, j);  // 每width字节数据输出一次
            j = 0;
        }
    }
    debug_buf[j++] = '\n';
    log_output(debug_buf, j);  // 加上回车换行,输出剩余字节
}

void log_set_print_enable(void)
{
    log_enable_flg = true;
}

void log_set_print_disable(void)
{
    log_enable_flg = false;
}

void log_set_hex_print_line_width(int width)
{
    if (width > 128)
    {
        width = 128;
    }
    hex_print_line_width = width;
}

void log_set_print_level(int level)
{
    if (level < 0 || level > 4)
    {
        log_e("log level error");
    }
    log_print_level = level;
}

void log_init(void)
{
    
}


