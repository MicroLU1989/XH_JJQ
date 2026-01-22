#ifndef _RTC_DEVICE_H_
#define _RTC_DEVICE_H_ 

#include "device.h"

#include "user_data_typedef.h"

#define RTC_CMD_SET_TIME 0x01



struct rtc_device_t
{
    struct device_t parent;
    time_s time;
};

struct rtc_config_t
{
    time_s time;
};

extern struct device_t *rtc_device_get(const char *dev_name);
extern struct device_t *rtc_device_create(const char *dev_name);


#endif