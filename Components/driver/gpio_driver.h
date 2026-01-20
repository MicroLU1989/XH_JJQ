#ifndef _GPIO_DRIVER_H_
#define _GPIO_DRIVER_H_

#include <stdint.h>



#define PIN_SPEED_10M     ((uint8_t)0x01U) 
#define PIN_SPEED_2M      ((uint8_t)0x02U)
#define PIN_SPEED_50M     ((uint8_t)0x03U)
#define PIN_SPEED_MAX    ((uint8_t)0x04U) 




#define PIN_MODE_AIN                    ((uint8_t)0x00U)          /*!< 模拟输入*/
#define PIN_MODE_IN_FLOATING            ((uint8_t)0x04U)          /*!< 浮空输入 */
#define PIN_MODE_IPD                    ((uint8_t)0x28U)          /*!< 下拉输入 */
#define PIN_MODE_IPU                    ((uint8_t)0x48U)          /*!< 上拉输入*/
#define PIN_MODE_OUT_OD                 ((uint8_t)0x14U)          /*!<漏极开漏输出 */
#define PIN_MODE_OUT_PP                 ((uint8_t)0x10U)          /*!< 推挽输出 */
#define PIN_MODE_AF_OD                  ((uint8_t)0x1CU)          /*!< 复用开漏输出 */
#define PIN_MODE_AF_PP                  ((uint8_t)0x18U)          /*!< 复用推挽输出 */


#define PIN_DIRECTION_IN    (0)     
#define PIN_DIRECTION_OUT   (1)


#define GPIO_PORT_A   (0)
#define GPIO_PORT_B   (1)
#define GPIO_PORT_C   (2)
#define GPIO_PORT_D   (3)
#define GPIO_PORT_E   (4)


#define GPIO_PIN_NUM(x)  ((uint32_t)((uint32_t)0x01U<<(x)))   


extern void drv_gpio_af_rcu_enable(void);
extern void drv_gpio_init(uint32_t port, uint32_t num, uint32_t mode,uint32_t speed);
extern void drv_gpio_write(uint32_t port, uint32_t num, uint32_t val);
extern uint32_t drv_gpio_read(uint32_t port, uint32_t num,uint32_t pin_direction);















#endif