#include "gpio_driver.h"
#include "gd32f30x_gpio.h"


static uint32_t pin_num_list[16] =
{
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
};


static uint32_t pin_port_list[16] =
{
    [GPIO_PORT_A] = GPIOA,
    [GPIO_PORT_B] = GPIOB,
	[GPIO_PORT_C] = GPIOC,
    [GPIO_PORT_D] = GPIOD,
    [GPIO_PORT_E] = GPIOE,
};


static uint32_t pin_clk_list[5] =
{
    [GPIO_PORT_A] = RCU_GPIOA,
    [GPIO_PORT_B] = RCU_GPIOB,
	  [GPIO_PORT_C] = RCU_GPIOC,
    [GPIO_PORT_D] = RCU_GPIOD,
    [GPIO_PORT_E] = RCU_GPIOE,
};



void drv_gpio_init(uint32_t port, uint32_t num, uint32_t mode, uint32_t speed)
{
    rcu_periph_clock_enable(pin_clk_list[port]);
    gpio_init(pin_port_list[port],mode,speed,num);
}

void drv_gpio_af_rcu_enable(void)
{
    rcu_periph_clock_enable(RCU_AF);
}

void drv_gpio_write(uint32_t port, uint32_t pin_num, uint32_t val)
{
	  if(RESET != val){
        GPIO_BOP(pin_port_list[port]) = (uint32_t)pin_num;
    }else{
        GPIO_BC(pin_port_list[port]) = (uint32_t)pin_num;
    }
}


uint32_t drv_gpio_read(uint32_t port, uint32_t num,uint32_t pin_direction)
{
	  uint32_t bit_read_val = 0;
		if(pin_direction == PIN_DIRECTION_IN)
			bit_read_val = (uint32_t)gpio_input_bit_get(pin_port_list[port],num);
		else
		   bit_read_val = (uint32_t)gpio_output_bit_get(pin_port_list[port],num);
		return bit_read_val;
}

