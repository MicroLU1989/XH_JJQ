#ifndef _RFID_SI522_DRIVER_H_
#define _RFID_SI522_DRIVER_H_ 


#include<stdint.h>




extern char Test_Si522_GetUID(void);
extern void rfid_si522_init(void);
extern uint8_t si522_read_reg(uint8_t reg_addr);
extern void si522_write_reg(uint8_t reg_addr,uint8_t data);


#endif