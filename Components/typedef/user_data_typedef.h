/************************************************************** 
 * @Author: JTX Software development team
 * @Date: 2023-08-21 14:22:10
 * @LastEditors: MicroLU1989 916739459@qq.com
 * @LastEditTime: 2023-12-25 15:47:53
 * @FilePath: \software_231110\components\user_typedef\user_typedef.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by www.jxjtx.com, All Rights Reserved. 
 **************************************************************/
#ifndef _USER_DATA_TYPEDEF_H_
#define _USER_DATA_TYPEDEF_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



typedef struct 
{
   uint8_t year;
   uint8_t month;
   uint8_t date;
   uint8_t hour;
   uint8_t min;
   uint8_t sec;
}time_s;


typedef struct
{
    uint32_t lati;       //纬度 Latitude (in degree * 10000000)
    uint32_t lngi;       //经度 Longitude(in degree * 10000000)
} gps_point_s;





#endif