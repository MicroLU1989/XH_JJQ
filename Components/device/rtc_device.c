#include "rtc_device.h"
#include "rtc_driver.h"
#include <string.h>
#include "os_port.h"
#include "common_utils.h"

#ifndef SYS_USING_HEAP
struct rtc_device_t rtc_dev_instance = {0};
#endif

struct rtc_device_t *rtc_device = NULL;

static int rtc_device_init(struct device_t *pdev, void *config)
{
    if (pdev == NULL)
    {
        return -1;
    }
    drv_rtc_init();
    return 0;
}

static int rtc_device_deinit(struct device_t *pdev)
{
    return 0;
}

static int rtc_device_read(struct device_t *pdev, void *exten_arg, uint8_t *buf, uint32_t size)
{
    if (buf == NULL || size == 0 || pdev == NULL)
    {
        return -1;
    }
    uint32_t timestamp = drv_rtc_get_counter();
    time_s time = timestamp_to_time(timestamp);
    memcpy(buf, &time, size);
    return size;
}

static int rtc_device_write(struct device_t *pdev, void *exten_arg, uint8_t *buf, uint32_t size)
{
    if (buf == NULL || size == 0 || pdev == NULL || exten_arg == NULL)
    {
        return -1;
    }
    struct rtc_device_t *rtc_dev = (struct rtc_device_t *)pdev;
    memcpy(&rtc_dev->time, buf, size);
    return size;
}

struct device_t *rtc_device_get(const char *dev_name)
{
    return (struct device_t *)rtc_device; // 目前只有一个flash设备
}

static int rtc_device_ctrl(struct device_t *pdev, uint32_t cmd, void *arg)
{
    int ret = -1;

    if (pdev == NULL || arg == NULL)
    {
        return ret;
    }
    struct rtc_device_t *rtc_dev = (struct rtc_device_t *)pdev;
    switch (cmd)
    {
    case RTC_CMD_SET_TIME:
    {
        uint32_t timestamp = time_to_timestamp(*((time_s *)arg));
        drv_rtc_set_counter(timestamp);
        ret = 0;
    }
    break;
    default:
        break;
    }
    return ret;
}

struct device_t *rtc_device_create(const char *dev_name)
{
    if (dev_name == NULL)
    {
        return NULL;
    }

#if defined(SYS_USING_HEAP)
    struct rtc_device_t *p_dev = (struct rtc_device_t *)os_malloc(sizeof(struct rtc_device_t));
    memset(p_dev, 0, sizeof(struct rtc_device_t));
#else
    p_dev = rtc_dev_instance;
    static rtc_adapter_t rtc_adp;
    p_dev->rtc_handle = &rtc_adp;
#endif
    p_dev->parent.type = DEVICE_TYPE_RTC;
    p_dev->parent.init = rtc_device_init;
    p_dev->parent.deinit = rtc_device_deinit;
    p_dev->parent.read = rtc_device_read;
    p_dev->parent.write = rtc_device_write;
    p_dev->parent.control = rtc_device_ctrl;
    int len = strlen(dev_name);
    if (len > (sizeof(p_dev->parent.name) - 1))
    {
        len = sizeof(p_dev->parent.name) - 1;
    }
    memcpy(p_dev->parent.name, dev_name, len);
    rtc_device = p_dev;
    return (struct device_t *)p_dev;
}
