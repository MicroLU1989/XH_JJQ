#include "buzzer.h"
#include "gpio_driver.h"
#include "os_port.h"
#include <stdio.h>

#define BUZZER_CTL_PORT GPIO_PORT_A
#define BUZZER_CTL_PIN GPIO_PIN_NUM(8)

#define BUZZER_ON() drv_gpio_write(BUZZER_CTL_PORT, BUZZER_CTL_PIN, 1)
#define BUZZER_OFF() drv_gpio_write(BUZZER_CTL_PORT, BUZZER_CTL_PIN, 0)

typedef void (*pfunc)(void);
typedef struct
{
    int index;
    pfunc hdl_func;

} buzzer_prompt_s;

buzzer_prompt_s buzzer_prompt_mng = {
        .index = 0,
        .hdl_func = NULL
};




static void buzzer_init(void)
{
    drv_gpio_init(BUZZER_CTL_PORT, BUZZER_CTL_PIN, PIN_MODE_OUT_PP, PIN_SPEED_50M);
}

static void buzzer_on(void)
{
    BUZZER_ON();
}

static void buzzer_off(void)
{
    BUZZER_OFF();
}


void buzzer_prompt_key_hdl(void)
{
    buzzer_on();
    os_task_sleep(50);
    buzzer_off();
    buzzer_prompt_mng.index = 0;
    buzzer_prompt_mng.hdl_func = NULL;
}

void buzzer_prompt_iccrad_hdl(void)
{
    buzzer_on();
    os_task_sleep(80);
    buzzer_off();
    buzzer_prompt_mng.index = 0;
    buzzer_prompt_mng.hdl_func = NULL;
}



buzzer_prompt_s buzzer_prompt_tab [] = 
{
    {BUZZER_PROMPT_NONE, NULL},
    {BUZZER_PROMPT_KEY, buzzer_prompt_key_hdl},
    {BUZZER_PROMPT_ICCARD, buzzer_prompt_iccrad_hdl},
};


// 加载蜂鸣器提示音
void buzzer_prompt_load(BUZZER_PROMPT_TYPE_E prompt_type)
{
    buzzer_prompt_mng.index = prompt_type;
    buzzer_prompt_mng.hdl_func = buzzer_prompt_tab[buzzer_prompt_mng.index].hdl_func;
}

void buzzer_task(void *param)
{
    buzzer_init();
    while (1)
    {
        if (buzzer_prompt_mng.hdl_func != NULL)
        {
            buzzer_prompt_mng.hdl_func();
        }
        else
        {
            os_task_sleep(10);
        }
    }
}