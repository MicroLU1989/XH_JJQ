#include "uart_device.h"


void rs485_task(void)
{ 
   struct device_t *rs485_dev = device_create(DEVICE_TYPE_UART, "485uart");
   struct uart_config_t uart_cfg = {
       .baudrate = 9600,
       .id = UART_0,
       .parity = UART_PARITY_NONE,
       .stopbits = UART_STOPBITS_1,
       .rx_size = 512,
   };
   device_init(rs485_dev, &uart_cfg);
   while(1)
   {
         

   }
}