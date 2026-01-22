

#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include"user_data_typedef.h"

#define uint_test_bit(val, bit) (val & ((1 << (bit)))) 


#define uint_set_bit(val, bit) \
    do                         \
    {                          \
        (val |= (1 << (bit)));   \
    }while(0)


#define uint_reset_bit(val, bit) \
    do                         \
    {                          \
        (val &= ~(1 << (bit)));   \
    }while(0)

#define  IP_SIZE  32

typedef struct
{
    uint8_t tcp_mode;
    uint8_t ip_buf[IP_SIZE];
    uint32_t port;
} server_analysis_s;




extern bool add_struct_crc(uint8_t * p, uint16_t len);
extern bool verify_struct_crc(uint8_t * p, uint16_t len);
extern uint16_t jt808_add_filt_char(uint32_t len, uint8_t* chr);
extern uint16_t jt808_del_filt_char(uint16_t len, uint8_t* chr);
extern uint16_t _str2tobcd(uint8_t* bByte, uint8_t* str);
extern uint8_t _strcpy_len(uint8_t* str1, uint8_t*  str2);
extern uint8_t _sw_endian(uint8_t *cp1, uint8_t *cp2, uint8_t n);
extern uint8_t date_to_week(uint16_t year, uint8_t month, uint8_t date);
extern uint8_t hex_to_bcd(uint8_t data);
extern uint8_t bcd_to_hex(uint8_t data);
extern uint32_t time_to_timestamp(time_s t);
extern time_s timestamp_to_time(uint32_t timestamp);
extern void utc_to_bj_time(time_s *utc,time_s *bj_time);
extern void get_clock_time(time_s *old_time, time_s *new_time, uint16_t n);
extern bool time_check(time_s t);

extern float gps_cal_distance(int32_t lati_1, int32_t lati_2, int32_t lngi_1, int32_t lngi_2);
extern uint8_t check_xor_val(uint8_t *src, uint16_t len);
extern uint32_t check_sum_val(uint8_t *src, uint16_t len);
extern uint16_t _memcpy_len(uint8_t* cp1, uint8_t* cp2, uint16_t n);
extern uint8_t* _strcat(uint8_t* str1, uint8_t* str2);
extern uint8_t* _strncat(uint8_t* str1, uint8_t* str2, uint8_t n);
extern uint8_t* _strncpy(uint8_t* dst, uint8_t* sur, uint8_t n);
extern int8_t get_flag_cnt(uint8_t *flag, uint8_t *src, uint16_t src_len);
extern bool check_is_hex_str(uint8_t *src, uint8_t len);
extern uint8_t _chartohex(uint8_t uChar_H, uint8_t uChar_L);
extern uint8_t *get_chr_field(uint8_t *pf, uint16_t rlen, uint8_t *chr, uint8_t n);
extern uint8_t get_check_xor(uint8_t * str, uint16_t start_addr, uint16_t end_addr);
extern uint8_t get_digst(char *str, char *dest, int dsize);
extern int string_split(char *src, const char *separator, char **dest, unsigned char limit);
extern void ascii_to_hex(uint8_t *dest, uint8_t *src, uint16_t len);
extern uint8_t *get_net_7e_index(uint8_t *src, uint16_t src_len);
extern double myln(double a);
bool server_param_analysis(char *src, bool tcp_mode_parse, char separator, server_analysis_s *dest_param);
#endif


