#include "gps.h"
#include "uart_device.h"
#include "os_port.h"
#include "gpio_driver.h"
#include "user_log.h"
#include <stdio.h>
#include "common_utils.h"
#include <math.h>

#define gps_tick_get() os_tick_ms_get()

#define GPS_RMC_INT_VAL (0x00524d43)
#define GPS_GGA_INT_VAL (0x00474741)
#define GPS_GSA_INT_VAL (0x00475341)

#define GPS_PWR_PORT GPIO_PORT_C
#define GPS_PWR_PIN GPIO_PIN_NUM(4)
 
static bool gps_debug_enable = false;
static bool car_is_moving = true;
static uint8_t gps_data_buf[128];

gps_exception_hdl_s gps_exception_hdl = {0x00};
gps_lost_info_s gps_lost_info = {0};
gps_base_info_s gps_base_info;
gps_save_data_s gps_save_data; // 保存参数
static void gps_pwr_gpio_init(void)
{
   drv_gpio_init(GPS_PWR_PORT, GPS_PWR_PIN, PIN_MODE_OUT_PP, PIN_SPEED_2M);
}

static void gps_pwr_on(void)
{
   drv_gpio_write(GPS_PWR_PORT, GPS_PWR_PIN, 1);
}

static void gps_pwr_off(void)
{
   drv_gpio_write(GPS_PWR_PORT, GPS_PWR_PIN, 0);
}

/****************************************************************************
 * 名称:    get_gps_field ()
 * 功能：计算数据帧中标志位(,)的个数
 * 入口参数：无
 * 出口参数：无
 ****************************************************************************/
uint8_t *get_gps_field(uint8_t *pf, uint8_t n)
{
   uint8_t *pt = pf;
   uint8_t len = 99;
   if (n == 0)
      return pf;

   while ((len != 0) && (*pt != 0))
   {
      if (*pt == ',')
         n--;
      pt++;
      if (n == 0)
         break;
      if (len > 0)
         len--;
   }
   return pt;
}

/****************************************************************************
 * 名称:    read_gps ()
 * 功能：串口接收，单个字节
 * 入口参数：无
 * 出口参数：无
 ****************************************************************************/
void read_gps(uint8_t *buf_s, uint8_t *rbuf_ptr, uint16_t n)
{
   while (n--)
   {
      *buf_s++ = *rbuf_ptr;
      rbuf_ptr++;
   }
}

/****************************************************************************
 * 名称:    get_gps_field ()
 * 功能：找到对应标志位，并计算之间间隔的长度
 * 入口参数：无
 * 出口参数：无
 ****************************************************************************/
static uint8_t get_gps_flen(uint8_t *pf)
{
   uint8_t len = 0;
   uint8_t *pt = pf;

   while ((*pt != ',') && (*pt != 0))
   {
      len++;
      pt++;
   }
   return len;
}

// GPS数据校验
static bool gps_data_check(uint8_t *src, uint16_t len)
{
   uint8_t *pt = NULL;
   uint8_t xor_val = 0; // 计算出来的异或值
   int src_xor_val = 0; // 原始数据串中的异或值

   if (len == 0 || len > 200)
   {
      return false;
   }

   if (src[0] != '$')
   {
      return false;
   }
   pt = src;
   pt++; // 跳过 '$'
   while (*pt != '*' && len)
   {
      xor_val ^= *pt; // 异或校验, '$'与'*'之间的数据异或值
      pt++;
      len--;
   }
   pt++; // 跳过'*'
   if (check_is_hex_str(pt, 2))
   {
      sscanf((char *)pt, "%x", &src_xor_val);
      if (xor_val == src_xor_val)
      {
         return true;
      }
   }
   return false;
}

/****************************************************************************
 * 名		称:  lock_gps_posiont ()
 * 功		能：GPS锁定，防止静态漂移
 * 入口参数：无
 * 出口参数：无
 ****************************************************************************/
static void lock_gps_posiont(gps_point_s *p)
{
   static gps_point_s lock_locate = {0, 0};
   static uint32_t lock_tick = 0;
   gps_point_s cur;

   if (gps_tick_get() - lock_tick >= 5000) // 第一次定位，或从不定位到定位，
   {
      memset((uint8_t *)&lock_locate, 0x00, sizeof(lock_locate)); // 清除lock_locate
   }

   lock_tick = gps_tick_get();
   memcpy((uint8_t *)&cur, p, sizeof(gps_point_s)); // 获取最新的坐标

   if (car_is_moving == false) // 车辆静止,锁定位置
   {
      if ((gps_base_info.fix_num >= 5) && (!lock_locate.lati || !lock_locate.lngi)) // 初次执行
      {
         // 第一次定位，或从不定位到定位，更新lock_locate
         memcpy((uint8_t *)&lock_locate, (uint8_t *)&cur, sizeof(gps_point_s));
      }
      else if (lock_locate.lati || lock_locate.lngi)
      {
         memcpy(p, (uint8_t *)&lock_locate, sizeof(gps_point_s)); // 车辆静止时，坐标信息保持不变
      }
   }
   else // 车辆处于移动状态
   {
      memset((uint8_t *)&lock_locate, 0x00, sizeof(lock_locate)); // 不锁定坐标信息
   }
}

/********************************************************************
 *函 数 名: rmc_data_prease
 *函数功能: 解析RMC数据 #GNRMC,075051.00,A,2237.18718,N,11355.26236,E,26.518,1.65,100519,,,D,V*3C
 *入口参数: 无
 *返 回 值: 无
 *********************************************************************/
static void rmc_data_parser(uint8_t *rmc_data, uint16_t len)
{
   uint8_t *pt;
   uint8_t str[10];
   uint8_t buf[32];
   volatile uint16_t tnew;
   volatile int vnew = 0;
   volatile uint16_t dt;
   gps_point_s pnew;
   volatile float dist;
   double v_t;
   uint32_t dgr;
   uint32_t min;
   static uint16_t told = 0;
   static int vold = 0;
   static gps_point_s pold = {0, 0};
   time_s gps_time;
   time_s bj_time = {0x00};
   float d; // 方向
   static uint8_t err_cnt = 0;

   if (len > 100 || len < 20)
      return;

   pt = get_gps_field(rmc_data, 2);
   if (*pt != 'A') // 不定位
   {
      return;
   }
   gps_exception_hdl.fixed_monitor_tick_s = gps_tick_get(); // 更新定位时间
   // UTC日期
   pt = get_gps_field(rmc_data, 9);
   read_gps(buf, pt, 6);
   gps_time.date = atoi((const char *)_strncpy(str, buf, 2));
   gps_time.month = atoi((const char *)_strncpy(str, buf + 2, 2));
   gps_time.year = atoi((const char *)_strncpy(str, buf + 4, 2));

   // UTC时间
   pt = get_gps_field(rmc_data, 1);
   read_gps(buf, pt, 6);
   gps_time.hour = atoi((const char *)_strncpy(str, buf, 2));
   gps_time.min = atoi((const char *)_strncpy(str, buf + 2, 2));
   gps_time.sec = atoi((const char *)_strncpy(str, buf + 4, 2));

   utc_to_bj_time(&gps_time, &bj_time); // 将UTC时间转成北京时间
   gps_base_info.time = bj_time;
   // log_i("时间%02d:%02d:%02d \r\n",bj_time.hour,bj_time.min,bj_time.sec);
   // 计算接收的两次GPS数据之间的时间间隔
   tnew = (gps_time.min * 60) + gps_time.sec;
   if (tnew > told)
      dt = tnew - told;
   else if (tnew < told)
      dt = tnew + 3600 - told; // 下一个小时
   else if (tnew == told)
      dt = 1; // 如果两帧数据间隔时间不到1秒，默认为1秒 2019-01-16
   else
      dt = 1;

   if (gps_base_info.lost_fixed) // 定位丢失,再次定位之后,要用上一次定位时间与当前定位时间间隔计算加速度与距离
   {
      dt = time_to_timestamp(gps_base_info.time) - time_to_timestamp(gps_lost_info.time);
   }
   told = tnew;
   /***********************************************************************************/
   // 速度相关
   // 地面速率 000.0~999.9节
   pt = get_gps_field(rmc_data, 7);
   len = get_gps_flen(pt);

   if (len > 7)
      goto will_abort;

   read_gps(buf, pt, len);
   buf[len] = '\0';
   // 获取到速度值
   v_t = (atof((const char *)&buf)) * 1.85;

   vnew = floor(v_t);
   if (vnew > vold)
   {
      // 设定加速度 10m/s*s作为判断速度漂移的依据
      // 以百公里加速时间10秒来计算，加速度约为3m/s*s，此处设定加速度为 10m/s*s，余量足够大
      if ((vnew - vold) >= (10 * dt))
      {
         if (gps_base_info.fixed)
            goto will_abort;
      }
   }
   if (vnew < 5) // 速度小于5公里,置为0,防止静态漂移
   {
      vnew = 0;
   }
   else if (vnew > 180)
   {
      vnew = 180;
   }
   gps_base_info.speed = vnew;
   /***********************************************************************************/
   // 航向(000.0~359.9度，以正北为参考基准，前面的0也将被传输)
   pt = get_gps_field(rmc_data, 8);
   len = get_gps_flen(pt);
   read_gps(buf, pt, len);
   buf[len] = '\0';
   if (len > 0)
   {
      d = (atof((const char *)&buf));
      if (d >= 0 && d < 360)
      {
         gps_base_info.direction = floor(d);
      }
   }
   // 纬度ddmm.mmmm(度分)格式(前面的0也将被传输)
   pt = get_gps_field(rmc_data, 3);
   read_gps(buf, pt, 10);
   dgr = atoi((const char *)_strncpy(str, buf, 2)) * 1000000uL;
   min = atoi((const char *)_strncpy(str, buf + 2, 2)) * 10000uL;
   min += atoi((const char *)_strncpy(str, buf + 5, 4)) * 1uL;
   dgr += (min * 100 / 60);

   pt = get_gps_field(rmc_data, 4);
   pnew.lati = (*pt == 'N') ? dgr : -dgr;
   gps_base_info.lati_dir = (*pt == 'N') ? 2 : 1; // 纬度方向
   // 经度dddmm.mmmm(度分)格式(前面的0也将被传输)
   pt = get_gps_field(rmc_data, 5);
   read_gps(buf, pt, 11);
   dgr = atoi((const char *)_strncpy(str, buf, 3)) * 1000000uL;
   min = atoi((const char *)_strncpy(str, buf + 3, 2)) * 10000uL;
   min += atoi((const char *)_strncpy(str, buf + 6, 4)) * 1uL;
   dgr += (min * 100 / 60);
   pt = get_gps_field(rmc_data, 6);
   pnew.lngi = (*pt == 'E') ? dgr : -dgr;
   gps_base_info.lngi_dir = (*pt == 'E') ? 1 : 2; // 经度方向
   // 计算两点之间的近似距离
   dist = gps_cal_distance(pnew.lati, pold.lati, pnew.lngi, pold.lngi);
   if (dist >= (float)(45 * 45 * dt * dt) && (pold.lati))
   {
   will_abort:
      if (++err_cnt > 5)
      {
         err_cnt = 0;
         vold = vnew;
         pold = pnew;
      }
      gps_base_info.pnumber++;
      return;
   }
   vold = vnew;
   pold = pnew;
   gps_base_info.pnumber++;
   gps_base_info.up_jifs = gps_tick_get();
   gps_base_info.fixed = true;
   // 更新基本位置信息
   gps_base_info.locate = pnew;
   lock_gps_posiont(&gps_base_info.locate);
   // log_i("lati = %d ln = %d \n",gps_base_info.locate.lati,gps_base_info.locate.lngi);
}

/*********************************************************************
 *函 数 名: gga_data_prease
 *函数功能: 解析GGA数据 $GNGGA,035255.00,2238.81358,N,11354.99347,E,1,10,2.24,82.5,M,-2.6,M,,*6B
 *入口参数: 无
 *返 回 值: 无
 *********************************************************************/
static void gga_data_parser(uint8_t *gga_data, uint16_t len)
{
   uint8_t *pt;
   uint8_t buf[32] = {0};
   float heigh_t = 0;

   pt = get_gps_field(gga_data, 7);
   read_gps(buf, pt, 2);
   gps_base_info.fix_num = atoi((const char *)&buf);
   pt = get_gps_field(gga_data, 9);
   len = get_gps_flen(pt);
   read_gps(buf, pt, len);
   buf[len] = '\0';
   heigh_t = (atof((const char *)&buf));
   if (heigh_t >= 0 && heigh_t < 5000)
   {
      gps_base_info.heigh = floor(heigh_t);
   }
}

static void gsa_data_parser(uint8_t *str, uint16_t s_len)
{
   uint8_t *pt = NULL;
   uint8_t dat[10] = {0x00};
   uint8_t len = 0;
   int tmp = 0;
   // 获取卫星定位模式
   pt = get_gps_field(str, 2); // 第2个逗号的地址
   len = get_gps_flen(pt);

   if (len != 1)
      return;
   strncpy((char *)dat, (char *)pt, len); // 卫星定位模式，1 = 未定位， 2 = 二维定位， 3 = 三维定位
   dat[len] = '\0';
   tmp = atoi((const char *)&dat);
   if (tmp == 3 || tmp == 2)
   {
      gps_base_info.if3d = (uint8_t)tmp;
   }
   else
   {
      gps_base_info.if3d = 1;
   }
   pt = NULL;
}

// GPS里程计算,注意GPS里程计算的时间间隔为1s
void calculation_sum_mile(void)
{
   uint32_t temp_mil_value = 0;
   float dist = 0;
   static uint16_t mil_mod = 0;
   static uint32_t nold = 0;
   uint16_t cal_speed = 0;
   uint32_t interval_second;
   gps_point_s locate_tmp;
   time_s time_tmp;
   uint8_t lost_flg = 0;

   if (gps_base_info.fixed && nold != gps_base_info.pnumber)
   {
      nold = gps_base_info.pnumber;
      // gps_debug_print("\r\n calculation_sum_mile %d \r\n",nold);
      if (car_is_moving) // 在定位和ACC开或处于运动状态的时候计算里程
      {
         /*********************************************************
         gps_base.void_speed 单位1000m/h
         temp_mil_value , mil_mod单位0.01m
         速度系数为1.03倍
         **********************************************************/
         if (gps_base_info.speed > 200) // 如果速度超过200KM/H , 退出计算
            return;
         cal_speed = gps_base_info.speed;
         temp_mil_value = (cal_speed * 1000) / 36;
         temp_mil_value = (temp_mil_value * 103) / 100; // 速度*1.03系数
         gps_save_data.milage += (temp_mil_value / 100);
         mil_mod += (temp_mil_value % 100);
         gps_save_data.milage += (mil_mod / 100);
         mil_mod %= 100;
         if (gps_base_info.lost_fixed)
         {
            gps_base_info.lost_fixed = false;
            if (gps_lost_info.valid_flg)
            {
               time_tmp = gps_lost_info.time;
               locate_tmp = gps_lost_info.locate;
            }
            else
            {
               time_tmp = gps_save_data.time;
               locate_tmp = gps_save_data.locate;
            }

            dist = gps_cal_distance(locate_tmp.lati, gps_base_info.locate.lati, locate_tmp.lngi, gps_base_info.locate.lngi);
            interval_second = time_to_timestamp(gps_base_info.time) - time_to_timestamp(time_tmp);
            temp_mil_value = sqrt(dist);
            /*********************************************************
                限制条件
                1.时间不超过30分钟
                2.30分钟内的距离不会行驶60公里
            **********************************************************/
            temp_mil_value = (temp_mil_value * 103) / 100;     
            if (temp_mil_value > 100 && temp_mil_value < 60000 && interval_second > 5 && interval_second < 1800) // 60KM*1000
            {
               gps_save_data.milage += temp_mil_value;
            }
         }
         gps_base_info.milage = gps_save_data.milage;
      }
   }
}

void gps_task(void *param)
{
   uint8_t ch = 0, last_ch = 0;
   uint16_t index = 0;
   uint32_t gps_format_val;
   bool xor_flg = false;
   gps_pwr_gpio_init();
   gps_pwr_on();
   struct device_t *uart_dev = device_create(DEVICE_TYPE_UART, "gpsuart");
   struct uart_config_t uart_cfg = {
       .baudrate = 115200,
       .id = UART_1,
       .parity = UART_PARITY_NONE,
       .stopbits = UART_STOPBITS_1,
       .rx_size = 512,
   };
   device_init(uart_dev, &uart_cfg);
   while (1)
   {
      int len = device_read(uart_dev, NULL, &ch, 1);
      if (len > 0 && index < sizeof(gps_data_buf) - 1)
      {
         gps_data_buf[index++] = ch;
         if (ch == '\n' && last_ch == '\r') // 收到完整的一帧数据
         {
            gps_data_buf[index] = '\0'; // 字符串结束符
            uint16_t gps_recv_len = index;
            index = 0; // 重置索引，准备接收下一帧数据
            gps_format_val = 0;
            gps_format_val = gps_data_buf[3] << 16 | gps_data_buf[4] << 8 | gps_data_buf[5];
            if (gps_format_val == GPS_RMC_INT_VAL || gps_format_val == GPS_GGA_INT_VAL || gps_format_val == GPS_GSA_INT_VAL)
            {
               if (gps_debug_enable)
               {
                  log_d("%s", gps_data_buf);
               }
               xor_flg = gps_data_check(gps_data_buf, gps_recv_len); // 数据校验
               if (!xor_flg)
               {
                  log_e("GPS 数据校验失败");
                  continue;
               }
               switch (gps_format_val)
               {
               case GPS_RMC_INT_VAL:
                  rmc_data_parser(gps_data_buf, gps_recv_len);
                  break;

               case GPS_GGA_INT_VAL:
                  gga_data_parser(gps_data_buf, gps_recv_len);
                  break;

               case GPS_GSA_INT_VAL:
                  gsa_data_parser(gps_data_buf, gps_recv_len);
                  break;
               default:
                  break;
               }
            }
         }
         else
         {
            last_ch = ch;
         }
      }
      else
      {
         calculation_sum_mile();
         os_task_sleep(10);
         continue;
      }
   }
}