#ifndef _RTC_DRIVER_H_
#define _RTC_DRIVER_H_ 


void drv_rtc_init(void);
void drv_rtc_set_counter(uint32_t cnt);
uint32_t drv_rtc_get_counter(void);

#endif