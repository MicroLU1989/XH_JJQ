#ifndef _GPS_H_
#define _GPS_H_

#include "user_data_typedef.h"

typedef struct
{
    time_s      time;       	//GPS 时间,已转换成北京时间
    uint8_t   ant_sta;    	//天线状态
    bool        fixed;      	//是否定位标志
    bool        lost_fixed; 	//定位丢失 
    bool        moving_flg;     //车辆行驶标志
    uint8_t          if3d;       	//是否为3D定位
    uint8_t			lati_dir;		//纬度方向,0无效,1南纬,2北纬
    uint8_t			lngi_dir;		//经度方向,0无效,1东经,2西经
    gps_point_s     locate;     	//经纬度数据
    uint16_t         speed;      	//速度，0.1km/h
    uint16_t         direction;  	//方向
    uint16_t         heigh;      	//高度
    uint8_t          fix_num;    	//定位时，卫星颗数
    uint32_t         milage;         //里程单位米
    uint32_t         pnumber;    	// Packet number(0 ~ 4.2E9)
    uint32_t         up_jifs;    	//GPS 更新所对应的时间(单位ms );   //GPS 更新所对应的时间(单位ms )
    
} gps_base_info_s;

typedef struct 
{
   uint16_t crc;
   uint16_t flag;
   gps_point_s  locate;       //经纬度数据
   uint32_t      milage;       //里程 
   time_s   time; 
   uint32_t      res;          //保留       
}gps_save_data_s;      



typedef struct 
{
    bool is_valid;
    uint32_t tick;

} gps_ref_tick_s;

typedef struct 
{
   uint8_t valid_flg;
   time_s time;
   gps_point_s locate;

}gps_lost_info_s;


typedef struct 
{
    uint8_t rmc_xor_err_cnt;     
    uint32_t rmc_monitor_tick_s;
    uint32_t gga_monitor_tick_s;
    uint32_t fixed_monitor_tick_s;  
    
} gps_exception_hdl_s;

void gps_task(void *param);

#endif