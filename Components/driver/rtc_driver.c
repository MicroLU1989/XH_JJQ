#include "gd32f30x_rtc.h"



static void rtc_configuration(void)
{
 /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32767);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

}


void drv_rtc_init(void)
{
    uint32_t RTCSRC_FLAG = 0;
    
    nvic_irq_enable(RTC_IRQn,6,0);

    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);

    /* allow access to BKP domain */
    pmu_backup_write_enable();
    if(RESET != (RCU_BDCTL & RCU_BDCTL_BKPRST)) {
        rcu_bkp_reset_disable();
    }

    /* get RTC clock entry selection */
    RTCSRC_FLAG = GET_BITS(RCU_BDCTL, 8, 9);

    if ((0xA5A5 != bkp_read_data(BKP_DATA_0)) || (0x00 == RTCSRC_FLAG)){
        /* backup data register value is not correct or not yet programmed
        or RTC clock source is not configured (when the first time the program 
        is executed or data in RCU_BDCTL is lost due to Vbat feeding) */

        /* RTC configuration */
        rtc_configuration();

        /* adjust time by values entred by the user on the hyperterminal */
        //time_adjust(); //设置时间

        bkp_write_data(BKP_DATA_0, 0xA5A5);  //保存数据
    }else{
        /* check if the power on reset flag is set */
        // if (rcu_flag_get(RCU_FLAG_PORRST) != RESET){
        //     log_d("\r\n\n Power On Reset occurred....");
        // }else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET){
        //     /* check if the pin reset flag is set */
        //     log_d("\r\n\n External Reset occurred....");
        // }

        /* allow access to BKP domain */
        rcu_periph_clock_enable(RCU_PMU);
        pmu_backup_write_enable();

        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();

        /* enable the RTC second */
        rtc_interrupt_enable(RTC_INT_SECOND);
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
    }
}

void drv_rtc_set_counter(uint32_t cnt)
{
    rtc_lwoff_wait();
    rtc_counter_set(cnt);
   // rtc_lwoff_wait();
}

uint32_t drv_rtc_get_counter(void)
{
    return  rtc_counter_get();
}