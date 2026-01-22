

#include "common_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define STRUCT_MAX_SIZE (1024)


const uint8_t mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
/***************************************************************************
* 名称:    add_struct_crc ()
* 功能：添加结构体校验
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool add_struct_crc(uint8_t *p, uint16_t len)
{
    uint8_t buf[STRUCT_MAX_SIZE];
    uint16_t crc;
    uint16_t i;

    if (len < 4 || len > STRUCT_MAX_SIZE)
    {
        return false;
    }

    memset(buf, 0x00, STRUCT_MAX_SIZE);
    memcpy(buf, p, len);

    buf[2] = 0xaa;
    buf[3] = 0x55;

    crc = 0;
    for (i = 2; i < len; i++)
    {
        crc += buf[i];
    }

    buf[0] = (crc & 0xff00) >> 8; //加入CRC
    buf[1] = (uint8_t)crc; //加入CRC

    memcpy(p, buf, len);
    return true;
}

/****************************************************************************
* 名称:    verify_struct_crc ()
* 功能：校验结构体信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool verify_struct_crc(uint8_t *p, uint16_t len)
{
    uint16_t crc;
    uint16_t crc1;
    uint16_t flag;
    uint16_t i;

    crc = 0;
    crc1 = 0;
    flag    = 0;

    if (len < 4 || len > STRUCT_MAX_SIZE)
    {
        return false;
    }

    crc1 = p[0];
    crc1 <<= 8;
    crc1 += p[1];

    flag = p[2];
    flag <<= 8;
    flag += p[3];

    crc = 0;
    for (i = 2; i < len; i++)
    {
        crc += p[i];
    }

    if (crc1 == crc && flag == 0xaa55)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/****************************************************************************
* 名称：_sw_endian
* 功能：大小端模式转换
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t _sw_endian(uint8_t *cp1, uint8_t *cp2, uint8_t n)
{
    uint8_t ret_len = 0;

    ret_len = n;

    if ((cp1 == NULL) || (cp2 == NULL))
        return ret_len;
    cp2 += n - 1;
    while (n--)
    {
        *cp1 = *cp2;
        cp1 ++;
        cp2 --;
    }
    return ret_len;
}


/***************************************************************************
* 名称：_str2tobcd
* 功能：2个字符转换成BCD 格式
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint16_t _str2tobcd(uint8_t *bByte, uint8_t *str)
{
    *bByte  = (*str++ - 0x30) << 4;
    *bByte |= (*str++ - 0x30) & 0x0f;

    return 0;
}


/***************************************************************************
* 名     称：hex_to_bcd
* 功     能：bcd码转十进制
* 入口参数：无
* 出口参数：无
****************************************************************************/

uint8_t bcd_to_hex(uint8_t data)
{
    uint8_t temp;

    temp = ((data >> 4) * 10 + (data & 0x0f));
    return temp;
}


/***************************************************************************
* 名     称：hex_to_bcd
* 功     能：十进制转bcd码
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t hex_to_bcd(uint8_t data)
{
    uint8_t temp;
    temp = (((data / 10) << 4) + (data % 10));
    return temp;
}



/******************************************************************
 *函 数 名: date_to_week
 *函数功能: 根据年月日,计算星期
 *入口参数: 无
 *返 回 值: 无
******************************************************************/
uint8_t date_to_week(uint16_t year, uint8_t month, uint8_t date)
{
    uint8_t week, c, temp_y ;

    if (month <= 2)
    {
        month |= 4 ;  //1月2月同5月6月表
        year--;
    }
    c = year / 100 ;
    c &= 0x03 ;   //百年%4
    temp_y = (char)(year % 100);
	
	//星期=(百年%4*5+年+年/4+(13*月+8)/5+日)%7
    week = ((c | (c << 2)) + (temp_y + (temp_y >> 2)) + (13 * month + 8) / 5 + date) % 7; 
    return week ;    //返回星期
}



static uint8_t get_monsize(uint8_t year, uint8_t month)
{
    const uint8_t month_days[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    
    if (month < 1 || month > 12) {
        return 0;
    }
    
    uint8_t days = month_days[month - 1];
    
    if (month == 2) {
        uint16_t full_year = 2000 + year;
        if ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) {
            days = 29;
        }
    }
    
    return days;
}

/****************************************************************************
* 名称:    time_to_timestamp
* 功能：时间转换成秒，2000/01/01 00:00:00 为基准时间
****************************************************************************/
uint32_t time_to_timestamp(time_s t1) {
    // 验证输入有效性
    if (t1.year > 99 ||                     // 年份范围限制（2000-2099）
        t1.month == 0 || t1.month > 12 ||  // 月份范围限制
        t1.date == 0 || t1.date > 31 ||    // 日期范围限制
        t1.hour > 23 ||                    // 小时范围限制
        t1.min > 59 || t1.sec > 59)        // 分钟秒范围限制
    {
        return 0;
    }
    
    uint16_t full_year = 2000 + t1.year;
    
    // 计算从 2000 年到目标年份前一年的总天数
    uint32_t total_days = 0;
    for (uint16_t year = 2000; year < full_year; year++) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            total_days += 366;
        } else {
            total_days += 365;
        }
    }
    
    // 计算目标年份中到指定日期的天数
    for (uint8_t month = 1; month < t1.month; month++) {
        total_days += get_monsize(t1.year, month);
    }
    total_days += (t1.date - 1); // 1号是第0天
    
    // 计算总秒数
    uint32_t total_seconds = total_days * 86400;
    total_seconds += t1.hour * 3600;
    total_seconds += t1.min * 60;
    total_seconds += t1.sec;
    
    return total_seconds;
}

/****************************************************************************
* 名称:    timestamp_to_time
* 功能：stamp 转 time，从 2000年1月1日00:00:00 开始
****************************************************************************/
time_s timestamp_to_time(uint32_t timestamp) 
{
    time_s result = {0};
    
    // 验证时间戳有效性（这里不设上限，只设下限为0）
    if (timestamp > (uint32_t)(UINT32_MAX / 2)) { // 简单防止溢出
        return result;
    }
    
    uint32_t total_days = timestamp / 86400;
    uint32_t seconds_left = timestamp % 86400;
    
    // 时间部分
    result.hour = seconds_left / 3600;
    result.min  = (seconds_left % 3600) / 60;
    result.sec  = seconds_left % 60;
    
    // 从 2000 年开始计算年份
    uint16_t current_year = 2000;
    uint32_t remaining_days = total_days;
    
    while (remaining_days >= 365) {
        uint16_t days_in_year = 365;
        if ((current_year % 4 == 0 && current_year % 100 != 0) || (current_year % 400 == 0)) {
            days_in_year = 366;
        }
        
        if (remaining_days >= days_in_year) {
            remaining_days -= days_in_year;
            current_year++;
        } else {
            break;
        }
    }
    
    // 年份转成两位数表示
    result.year = current_year - 2000;
    
    // 月份和日期
    uint8_t current_month = 1;
    uint32_t days_passed_this_year = remaining_days;
    
    while (current_month <= 12) {
        uint8_t days_in_month = get_monsize(result.year, current_month);
        
        if (days_passed_this_year < days_in_month) {
            result.date = days_passed_this_year + 1;
            result.month = current_month;
            break;
        }
        days_passed_this_year -= days_in_month;
        current_month++;
    }
    
    return result;
}

/****************************************************************************
* 名     称:  utc_to_bj_time
* 功     能：将UTC时间转成北京时间(UTC+8)
* 入口参数：utc - UTC时间结构体指针
*           bj_time - 存储转换结果的北京时间结构体指针
* 出口参数：无
****************************************************************************/
void utc_to_bj_time(time_s *utc, time_s *bj_time)
{
    // 复制UTC时间到北京时间
    memcpy(bj_time, utc, sizeof(time_s));
    
    // 北京时间 = UTC时间 + 8小时
    bj_time->hour += 8;
    
    // 处理小时进位
    if (bj_time->hour >= 24) {
        bj_time->hour -= 24;  // 减去24小时
        bj_time->date += 1;    // 日期加1
        
        // 获取当前月份的天数
        uint8_t days_in_month = get_monsize(bj_time->year, bj_time->month);
        
        // 处理日期进位
        if (bj_time->date > days_in_month) {
            bj_time->date = 1;  // 日期重置为1
            bj_time->month += 1; // 月份加1
            
            // 处理月份进位
            if (bj_time->month > 12) {
                bj_time->month = 1;  // 月份重置为1
                bj_time->year += 1;  // 年份加1

                if (bj_time->year > 99) {
                    bj_time->year = 0;
                }
            }
        }
    }
}


/****************************************************************************
* 名称：jt808_del_filt_char
* 功能：字符转义
* 入口参数：数据长度，待转移数据
* 出口参数：转义后的数据实际长度
****************************************************************************/
uint16_t jt808_del_filt_char(uint16_t len, uint8_t *chr)
{
    uint16_t  i, j, k ;
    uint8_t  *move;

    j = 0 ;
    for (i = 0; i < len; i++)
    {
        if (*chr++ == 0x7d)
        {
            j++;
            if (*chr == 0x02)
                *(chr - 1) = 0x7e;

            move = chr;
            for (k = 0; k < (len - i - 2); k++)
            {
                *move = *(move + 1);
                move++;
            }
            *move = '\0';
        }
    }
    return (j) ;
}

/****************************************************************************
* 名称：jt808_add_filt_char
* 功能：字符转义
* 入口参数：数据长度，待转移数据
* 出口参数：转义后的数据实际长度
****************************************************************************/
uint16_t jt808_add_filt_char(uint32_t len, uint8_t *chr)
{
    uint16_t  i, j, k ;
    uint8_t  *move;

    j = 0 ;
    for (i = 0; i < len; i++)
    {
        if (*chr == 0x7e || *chr == 0x7d)
        {
            j++;
            move = chr + (len - i);
            *(move + 1) = '\0';
            for (k = 0; k < (len - i); k++)
            {
                *move = *(move - 1);
                move--;
            }
            move++;
            if (*chr == 0x7d)
                *move = 0x01;
            else *move = 0x02;

            *chr = 0x7d;
            chr++;
        }
        chr ++;
    }
    return (j);
}



/****************************************************************************
* 名称:    _strcpy_len ()
* 功能：带有结束符的字符串拷贝
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t _strcpy_len(uint8_t *str1, uint8_t  *str2)
{
    uint8_t i = 0;

    while (*str2 != '\0')
    {
        *str1++ = *str2++;
        i++;
    }
    *str1 = '\0';
    i++;
    return i;
}




/****************************************************************************
* 名称：gps_cal_distance
* 功能：近似计算两点间的距离
* 入口参数：无
* 出口参数：无
****************************************************************************/

float gps_cal_distance(int32_t lati_1, int32_t lati_2, int32_t lngi_1, int32_t lngi_2)
{
    float x, y;

    x = 60 * 1851.5 * labs(lati_1 - lati_2);
    x /= (float)1000000;

    y = 60 * 1422.2 * labs(lngi_1 - lngi_2);
    y /= (float)1000000;

    return (x * x + y * y);
}



//异或校验
uint8_t check_xor_val(uint8_t *src, uint16_t len)
{
    uint8_t xor = 0x00;
    for (int i = 0; i < len; i++)
        xor ^= src[i];
    return xor;
}


//加和校验
uint32_t check_sum_val(uint8_t *src, uint16_t len)
{
    uint32_t sum = 0x00;
    for (int i = 0; i < len; i++)
        sum += src[i];
    return sum;
}


/****************************************************************************
* 名称:	  _memcpy ()
* 功能：带长度，数据拷贝
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint16_t _memcpy_len(uint8_t *cp1, uint8_t *cp2, uint16_t n)
{
    uint16_t ret_len = 0;

    if (n == 0)
        return 0;

    ret_len = n;
    while (n--)
        *cp1++ = *cp2++;

    return ret_len;
}



/****************************************************************************
* 名称:	  _strcpy ()
* 功能：字符串截取并拷贝
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t *_strcat(uint8_t *str1, uint8_t *str2)
{
    uint8_t *ret = str1;

    while (*str1 != '\0')
        str1++;
    while ((*str1++ = *str2++) != '\0');
    return ret;
}

/****************************************************************************
* 名称:	  _strncat ()
* 功能：字符串截取并拷贝
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t *_strncat(uint8_t *str1, uint8_t *str2, uint8_t n)
{
    uint8_t *ret = str1;

    while (*str1 != '\0')
        str1++;
    while (n && ((*str1++ = *str2++) != '\0'))
        n--;
    if (n == 0)
        *str1 = '\0';
    return ret;
}

/****************************************************************************
* 名称:	  _strncpy ()
* 功能：字符拷贝
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t *_strncpy(uint8_t *dst, uint8_t *sur, uint8_t n)
{
    *(dst) = '\0';
    return _strncat(dst, sur, n);
}



//获取标志字符个数
int8_t get_flag_cnt(uint8_t *flag, uint8_t *src, uint16_t src_len)
{
    int8_t flag_cnt = 0;
    uint16_t i;
    if ((NULL == flag) || (NULL == src) || (0 == src_len))
        return flag_cnt;

    for (i = 0; i < src_len; i++)
    {
        if (*flag == *(src + i))
            flag_cnt ++;
    }
    return flag_cnt;
}



/******************************************************************
 *函 数 名: check_is_hex_str
 *函数功能: 检测是否为16进制字符串
 *入口参数: src 待检测的字符传, len 长度
 *返 回 值: true 是16进制字符串, flase 非16进制字符串
******************************************************************/
bool check_is_hex_str(uint8_t *src, uint8_t len)
{
    uint8_t i ;
    if (src == NULL || len == 0)
        return false;
    for (i = 0; i < len; i++)
    {
        if ((src[i] >= '0' && src[i] <= '9') || (src[i] >= 'A' && src[i] <= 'F') || (src[i] >= 'a' && src[i] <= 'f'))
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}


/****************************************************************************
* 名	   称：_chartohex
* 功	   能：单字节的字符转换成HEX格式
* 入口参数：无
* 出口参数：无
****************************************************************************/
uint8_t _chartohex(uint8_t uChar_H, uint8_t uChar_L)
{
    if ((uChar_H >= '0') && (uChar_H <= '9'))
    {
        uChar_H -= 0x30 ;
    }
    else
    {
        if ((uChar_H >= 'A') && (uChar_H <= 'F'))
        {
            uChar_H -= 0x37 ;
        }
        else
        {
            if ((uChar_H >= 'a') && (uChar_H <= 'f'))
                uChar_H -= 0x57 ;
            else
                return (0) ;
        }
    }

    if ((uChar_L >= '0') && (uChar_L <= '9'))
    {
        uChar_L -= 0x30 ;
    }
    else
    {
        if ((uChar_L >= 'A') && (uChar_L <= 'F'))
        {
            uChar_L -= 0x37 ;
        }
        else
        {
            if ((uChar_L >= 'a') && (uChar_L <= 'f'))
                uChar_L -= 0x57 ;
            else
                return (0) ;
        }
    }

    return (16 * uChar_H + uChar_L);
}


/****************************************************************************
* 名		 称:	 get_chr_field ()
* 功		 能：在pf指向的字符串中查找第n个chr标志字符,并返回第n个chr字符的地址
* 入 口 参	 数：@pf		 源字符串
			 @len	 源字符串长度
			 @chr	 查找的标志字符
			 @n 	 查找第几个标志字符
* 出 口 参	 数：成功返回第n个chr字符的地址,否则返回NULL
****************************************************************************/
uint8_t *get_chr_field(uint8_t *pf, uint16_t rlen, uint8_t *chr, uint8_t n)
{
    uint8_t *pt = pf;
    uint16_t  len = rlen;

    if (pf == NULL || len == 0  || chr == NULL || n == 0)
        return NULL;

    while ((len != 0) && (*pt != 0))
    {
        if (*pt == *chr)
            n--;
        pt++;
        if (n == 0)
            break;
        if (len > 0)
            len--;
    }
    return pt;
}

/*********************************************************
名	称：get_check_xor
功	能：计算校验值
参	数：*str 指针，st_addr 开始地址，end_addr 接收地址
输	出：返回校验值
编写日期：2011-01-18
*********************************************************/
uint8_t get_check_xor(uint8_t *str, uint16_t start_addr, uint16_t end_addr)
{
    uint16_t i;
    uint16_t j;
    uint8_t xvalue;

    if (str == NULL)
    {
        return 0xff;
    }
    j = end_addr;
    xvalue = 0x00;
    for (i = start_addr; i < j; i++)
    {
        xvalue = xvalue ^ str[i];
    }
    return xvalue;
}




/******************************************************************
 *函 数 名: get_digst
 *函数功能: 取出字符串中的连续的数字字符,
 *入口参数: @str        源字符串
            @dest   解析后的数字字符串
            @dsize  数字字符长度,源字符串中的数字字符串长度不得
                    超过dsize

 *返 回 值: 解析后的数字字符串长度
******************************************************************/
uint8_t get_digst(char *str, char *dest, int dsize)
{
    uint8_t ret = 0;
    char *psrc = NULL;

    for (; str && *str != '\0'; str++)
    {
        if ((*str >= '0' && *str <= '9'))
        {
            if (++ret < dsize)
            {
                psrc = str;
                *dest++ = *str;
            }
            else
            {
                *dest = '\0';
                break;
            }
        }
        else
        {
            if (psrc)
            {
                *dest = '\0';
                break;
            }
        }
    }
    return ret;
}




/*******************************************************
* 函 数 名：get_clock_time
* 函数功能：将给定的RCT时间，加n分钟后，返回新的RTC时间
* 入口参数：@old_time 给定的RTC时间, @new_time 加n分钟后的时间,
            @ n 分钟数,
* 返 回 值：无
********************************************************/

void get_clock_time(time_s *old_time, time_s *new_time, uint16_t n)
{

    uint16_t sec_t = 0 ;
    uint16_t min_t = 0 ;
    
    min_t = n / 60 + (n % 60 + old_time->sec) / 60;
    
    sec_t = (old_time->sec + n) % 60;

    if ((old_time->min + min_t) > 59) //超过一个小时
    {
        new_time->min = old_time->min + min_t - 60;
        if ((old_time->hour + 1) == 24) //超过一天
        {
            new_time->hour    = (old_time->hour + 1) - 24;
            if ((old_time->year % 400 == 0) || ((old_time->year % 4 == 0) && (old_time->year % 100 != 0))) //闰年
            {
                if ((old_time->month == 2) && (old_time->date == 29)) //如果是闰年2月29日
                {
                    new_time->date    = 1;
                    new_time->month   = 3;
                    new_time->year    = old_time->year;
                }
                else // 不是闰年2月29日
                {
                    if (old_time->date == 31) //如果是31号
                    {
                        if (old_time->month == 12) //如果是12月
                        {
                            new_time->date    = 1;
                            new_time->month   = 1;
                            new_time->year    = old_time->year + 1;
                        }
                        else  //不是12月
                        {
                            new_time->date    = 1;
                            new_time->month   = old_time->month + 1;
                            new_time->year    = old_time->year;
                        }
                    }
                    else if (old_time->date == 30) //如果是30号
                    {
                        if ((old_time->month == 4) || (old_time->month == 6) || (old_time->month == 9) || (old_time->month == 11))
                        {
                            new_time->date    = 1;
                            new_time->month   = old_time->month + 1;
                            new_time->year    = old_time->year;
                        }
                        else //1 3 5 7 8 10 12 月30号
                        {
                            new_time->date    = old_time->date + 1;
                            new_time->month   = old_time->month;
                            new_time->year    = old_time->year;
                        }
                    }
                    else
                    {
                        new_time->date    = old_time->date + 1;
                        new_time->month   = old_time->month;
                        new_time->year    = old_time->year;
                    }
                }
            }
            else //非闰年
            {
                if ((old_time->month == 2) && (old_time->date == 28)) //如果是非闰年2月28日
                {
                    new_time->date    = 1;
                    new_time->month   = 3;
                    new_time->year    = old_time->year;
                }
                else // 不是2月28日
                {
                    if (old_time->date == 31) //如果是31号
                    {
                        if (old_time->month == 12) //如果是12月
                        {
                            new_time->date    = 1;
                            new_time->month   = 1;
                            new_time->year    = old_time->year + 1;
                        }
                        else  //不是12月
                        {
                            new_time->date    = 1;
                            new_time->month   = old_time->month + 1;
                            new_time->year    = old_time->year;
                        }
                    }
                    else if (old_time->date == 30) //如果是30号
                    {
                        if ((old_time->month == 4) || (old_time->month == 6) || (old_time->month == 9) || (old_time->month == 11))
                        {
                            new_time->date    = 1;
                            new_time->month   = old_time->month + 1;
                            new_time->year    = old_time->year;
                        }
                        else //1 3 5 7 8 10 12 月30号
                        {
                            new_time->date    = old_time->date + 1;
                            new_time->month   = old_time->month;
                            new_time->year    = old_time->year;
                        }
                    }
                    else
                    {
                        new_time->date    = old_time->date + 1;
                        new_time->month   = old_time->month;
                        new_time->year    = old_time->year;
                    }
                }
            }
        }
        else  //如果没有超过一天,但是超过一个小时
        {
            new_time->hour    = old_time->hour + 1;
            new_time->date    = old_time->date;
            new_time->month   = old_time->month;
            new_time->year    = old_time->year;
        }
    }
    else //没有超过一个小时
    {
        new_time->min     = old_time->min + min_t;
        new_time->hour    = old_time->hour;
        new_time->date    = old_time->date;
        new_time->month   = old_time->month;
        new_time->year    = old_time->year;
    }
    new_time->sec     = sec_t;
}




//校验时间有效性
bool verify_time(time_s *time)
{
    if ((time->year < 22) || (time->year > 99)) //校验时间限制在2022 ~ 2099
        return false;
    if ((time->month < 1) || (time->month > 12))
        return false;
    if ((time->date < 1) || (time->date > 31))
        return false;
   // if ((time->week < 1) || (time->week > 7))
   //     return false;
    if (time->hour > 24)
        return false;
    if (time->min > 59)
        return false;
    if (time->sec > 59)
        return false;
    return true;
}


bool time_check(time_s t)
{
    if ((t.year < 13) || (t.year > 43)) //校验时间限制在2013 ~ 2043
        return false;
    if ((t.month < 1) || (t.month > 12))
        return false;
    if ((t.date < 1) || (t.date > 31))
        return false;
    if (t.hour > 24)
        return false;
    if (t.min > 59)
        return false;
    if (t.sec > 59)
        return false;
    return true;
}




/**************************************************************************
*函 数 名 ：string_split
*功     能 ：字符串分割函数
*输入参数 ：src 				需要分割的字符串
            separator	分割符 如空格,逗号等等
            dest 		分割完后字符串的存储地址,二级指针
            limit       最大分割字符串数量限制
*返 回 值 ：	分割后的字符串数量
**************************************************************************/
int string_split(char *src, const char *separator, char **dest, unsigned char limit)
{
    int cnt = 0; //记录分割后字符串数量
    char *p_next = NULL;
    if (src == NULL || separator == NULL || dest == NULL || limit == 0 ||  strlen(src) == 0 || strlen(separator) == 0)
    {
        return 0;
    }
    p_next = strtok(src, separator);
    while (p_next != NULL && limit)
    {
        *dest ++ = p_next;
        limit --;
        cnt ++;
        p_next = strtok(NULL, separator);
    }
    return cnt;
}


/**************************************************************************
*函 数 名 ：ascii_to_hex(uint8_t* dest, uint8_t* src, uint16_t len)
*功     能 ：将ASCII码转换成HEX
*输入参数 ：dest，存储转换后数据的缓存  				src，存储需要转换数据的缓存   			  len，数据长度
*返 回 值 ：	无
*
**************************************************************************/

void ascii_to_hex(uint8_t *dest, uint8_t *src, uint16_t len)
{
    uint8_t   dh, dl;

    while (len)
    {
        if (*src <= '9')
            dh = (*(src++) - '0');
        else
            dh = (*(src++) - 0x37);

        if (*src <= '9')
            dl = (*(src++) - '0');
        else
            dl = (*(src++) - 0x37);

        *(dest++) = ((dh << 4) + dl);
        len -= 2;
    }
}




/**********************************************
* 函 数 名：get_net_7e_index
* 函数功能：获取0x7e在指定缓存中位置
* 入口参数：@src 数据
            @src_len 数据长度

* 返 回 值：0x7e在指定缓存中地址
***********************************************/
uint8_t *get_net_7e_index(uint8_t *src, uint16_t src_len)
{
    uint8_t *p = NULL;
    if (src == NULL || src_len == 0 || src_len > 2048)
        return p;
    while (src_len)
    {
        if (*src == 0x7e)
        {
            return src;
        }
        src ++;
        src_len --;
    }
    return p;
}



//自然对数运算
double myln(double a)
{
   int N = 15;//我们取了前15+1项来估算
   int k,nk;
   double x,xx,y;
   x = (a-1)/(a+1);
   xx = x*x;
   nk = 2*N+1;
   y = 1.0/nk;
   for(k=N;k>0;k--)
   {
     nk = nk - 2;
     y = 1.0/nk+xx*y;
     
   }
   return 2.0*x*y;
}





//校验IP端口合法性
static bool check_server_ip_port(uint8_t *ip_src, uint32_t port)
{
    uint8_t flg = '.', i = 0;
    int8_t flg_cnt = 0;
    uint8_t domain_flg = 0;
    int a = 0, b = 0, c = 0, d = 0, ret = 0;
    int len = strlen((const char *)ip_src); //IP长度,
    if (len < 5)
    {
        return false;
    }
    //判断IP中的 .
    flg_cnt = get_flag_cnt(&flg, ip_src, len); //点分格式有3个'.',域名格式至少有1个'.',最多有3个'.'
    if (flg_cnt == 0 || flg_cnt > 3)
    {
        return false;
    }

    for (i = 0; i < len; i++)
    {
        if (ip_src[i] == '.' || (ip_src[i] >= '0' && ip_src[i] <= '9'))
        {
            continue;
        }
        else
        {
            domain_flg = 1; //域名格式IP端口
        }
    }

    if (domain_flg == 0) //点分格式
    {
        if (flg_cnt != 3) //点分格式最少有3个'.'
        {
            return false;
        }
        ret = sscanf((const char *)ip_src, "%d.%d.%d.%d", &a, &b, &c, &d);
        if (ret != 4)
        {
            return false;
        }
        if (a < 0 || b < 0 || c < 0 || d < 0)
        {
            return false;
        }
        if (a > 255 || b > 255 || c > 255 || d > 255)
        {
            return false;
        }
    }

    if (port > 65535 || port < 100) //端口限制在 100~65535
    {
        return false;
    }
    return true;
}


/************************************************************** 
 * @description: 解析ip端口
 * @param {uint8_t} *src  待解析的字符串 
 * @param {bool} tcp_mode_parse 是否带连接模式解析, true  1,203.86.8.5分隔符9999 
 *                                                false  203.86.8.5分隔符9999 
 * @param {char} separator IP端口分隔符
 * @param {server_analysis_s} *dest_param 解析后的IP端口信息
 * @return {*} 成功 true false 失败
 **************************************************************/
bool server_param_analysis(char *src, bool tcp_mode_parse, char separator, server_analysis_s *dest_param)
{
    unsigned char i = 0;
    memset(dest_param->ip_buf, 0, sizeof(dest_param->ip_buf));
    if(tcp_mode_parse)
    {
        dest_param->tcp_mode = src[0] - 0x30;
        if (dest_param->tcp_mode != 1 && dest_param->tcp_mode != 0)
        {
            return false;
        }
        src += 2;  
    }
    while (src[i] != separator && i < IP_SIZE)
    {
        dest_param->ip_buf[i] = src[i];
        i ++;
    }
    src += (i + 1);
    dest_param->port = atoi((const char *)src);
    return check_server_ip_port(dest_param->ip_buf, dest_param->port);
}
