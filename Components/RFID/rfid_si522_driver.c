#include <stdio.h>
#include "rfid_si522_driver.h"
#include "i2c_device.h"
#include "gpio_driver.h"
#include "si522.h"
#include "user_log.h"
#include "systick.h"


#define RFID_NRST_PIN GPIO_PIN_NUM(15)
#define RFID_NRST_PORT GPIO_PORT_A

#define RFID_NIRQ_PIN GPIO_PIN_NUM(6)
#define RFID_NIRQ_PORT GPIO_PORT_C

#define RFID_PWR_PORT GPIO_PORT_C
#define RFID_PWR_PIN GPIO_PIN_NUM(7)

static void rfid_si522_gpio_init(void)
{
    drv_gpio_init(RFID_NRST_PORT,RFID_NRST_PIN,PIN_MODE_OUT_PP,PIN_SPEED_10M);
    drv_gpio_init(RFID_NIRQ_PORT,RFID_NIRQ_PIN,PIN_MODE_IN_FLOATING,PIN_SPEED_10M);
    //电源配置
    drv_gpio_init(RFID_PWR_PORT,RFID_PWR_PIN,PIN_MODE_OUT_PP,PIN_SPEED_10M);
    drv_gpio_write(RFID_PWR_PORT,RFID_PWR_PIN,1); //开启RFID电源
}


static struct device_t *si522_i2c_dev = NULL;

int rfid_si522_i2c_init(void)
{
    si522_i2c_dev = device_create(DEVICE_TYPE_I2C,"i2c0");
    struct i2c_device_cfg_t cfg = {
        .dev_addr = 0x50,
        .i2c_speed = 100000,
        .scl_pin_num = GPIO_PIN_NUM(14),
        .scl_port_num = GPIO_PORT_B,
        .sda_pin_num = GPIO_PIN_NUM(15),
        .sda_port_num = GPIO_PORT_B
    };
    device_init(si522_i2c_dev,&cfg);
		return 0;
}

void si522_read(uint8_t reg_addr,uint8_t *buf,uint32_t read_len)
{
    device_read(si522_i2c_dev,&reg_addr,buf,read_len);
}

void si522_write(uint8_t reg_addr,uint8_t *buf,uint32_t write_len)
{
    device_write(si522_i2c_dev,&reg_addr,buf,write_len);
}

uint8_t si522_read_reg(uint8_t reg_addr)
{
     uint8_t data = 0;
     device_read(si522_i2c_dev,&reg_addr,&data,1);
     return data;
}

void si522_write_reg(uint8_t reg_addr,uint8_t data)
{
    uint8_t rval = data;
    device_write(si522_i2c_dev,&reg_addr,&rval,1);
}


static void rfid_si522_reset(void)
{
	
	  delay_1us(5000);
	  drv_gpio_write(RFID_NRST_PORT,RFID_NRST_PIN,0);
	  delay_1us(200);
	  drv_gpio_write(RFID_NRST_PORT,RFID_NRST_PIN,1); 
	  delay_1us(200);
    uint8_t val ;
    si522_write_reg(CommandReg, 0x0f); //向CommandReg 写入0x0f作用是使Si522复位
    uint32_t timeout = 1000;
	  while(timeout --){
         val = si522_read_reg(CommandReg);
         if(val == 0x00) break;
    };
    if(timeout == 0){
        log_e("si522 reset timeout");
    }else{
        log_i("si522 reset ok");
    }	
	delay_1us(200);
}


//清寄存器
void ClearBitMask(uint8_t reg,uint8_t mask)  
{
    uint8_t tmp = 0x00;
    tmp = si522_read_reg(reg);
    si522_write_reg(reg, tmp & ~mask);  // clear bit mask
} 

//设置寄存器
void SetBitMask(uint8_t reg,uint8_t mask)  
{
    char tmp = 0x00;
    tmp = si522_read_reg(reg);
    si522_write_reg(reg,tmp | mask);  // set bit mask
}

/**
 * Change some bits of the register
 */
void SiModifyReg(uint8_t RegAddr, uint8_t ModifyVal, uint8_t MaskByte)
{
    uint8_t RegVal;
    RegVal = si522_read_reg(RegAddr);
    if(ModifyVal)
    {
        RegVal |= MaskByte;
    }
    else
    {
        RegVal &= (~MaskByte);
    }
    si522_write_reg(RegAddr, RegVal);
}



//开启天线
static void si522_antenna_on(void)
{
    uint8_t val = si522_read_reg(TxControlReg);
    if(!(val & 0x03)){
        val |= 0x03;
        si522_write_reg(TxControlReg, val);
    }
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    si522_write_reg(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   si522_write_reg(FIFODataReg, *(pIndata+i));   }
    si522_write_reg(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = si522_read_reg(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = si522_read_reg(CRCResultRegL);
    pOutData[1] = si522_read_reg(CRCResultRegH);
}


/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
//status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

unsigned char aaa = 0;

char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
        case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
    }
   
    si522_write_reg(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    si522_write_reg(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
		si522_write_reg(FIFODataReg, pInData[i]);    
	}
    si522_write_reg(CommandReg, Command);
   
    if (Command == PCD_TRANSCEIVE)
    {    
		SetBitMask(BitFramingReg,0x80);  
	}
    
    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
	//i = 2000;
    do 
    {
        n = si522_read_reg(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {   
		aaa = si522_read_reg(ErrorReg);
		
        if(!(si522_read_reg(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = si522_read_reg(FIFOLevelReg);
              	lastBits = si522_read_reg(ControlReg) & 0x07;
                if (lastBits)
                {   
					*pOutLenBit = (n-1)*8 + lastBits;   
				}
                else
                {   
					*pOutLenBit = n*8;   
				}
                if (n == 0)
                {   
					n = 1;    
				}
                if (n > MAXRLEN)
                {   
					n = MAXRLEN;   
				}
                for (i=0; i<n; i++)
                {   
					pOutData[i] = si522_read_reg(FIFODataReg);    
				}
            }
        }
        else
        {   
			status = MI_ERR;   
		}
        
    }
   
    SetBitMask(ControlReg,0x80);           // stop timer now
    si522_write_reg(CommandReg,PCD_IDLE); 
    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}



/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
	char status;  
	unsigned int unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);
	si522_write_reg(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   
		status = MI_ERR;   
	}
   
	return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////
char PcdSelect (unsigned char * pSnr, unsigned char *sak)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		*sak = ucComMF522Buf[0];
		status = MI_OK;  
	}
    else
    {   
		status = MI_ERR;    
	}

    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(unsigned char *pSnr, unsigned char anticollision_level)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    si522_write_reg(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = anticollision_level;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucComMF522Buf[i];
			snr_check ^= ucComMF522Buf[i];
		}
		if (snr_check != ucComMF522Buf[i])
   		{   
			status = MI_ERR;    
		}
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}


char Test_Si522_GetUID(void)
{
	unsigned char ATQA[2];
	unsigned char UID[12];
	unsigned char SAK = 0;
	unsigned char CardReadBuf[16] = {0};
	unsigned char CardWriteBuf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	unsigned char CardKeyABuf[6] = { 0x20, 0x12, 0x10, 0x01, 0x00, 0x00 };
	unsigned char DefaultKeyABuf[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char CardRevBuf[16] = { 0 };

	//log_i("Test_Si522_GetUID");
	
	//request 寻卡
	if( PcdRequest( PICC_REQIDL, ATQA) != MI_OK )  //寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		//log_i("Request:fail");
		return 1;		
	}
	else
	{
		log_i("Request:ok  ATQA:%02x %02x",ATQA[0],ATQA[1]);
	}
	
	//Anticoll 冲突检测 level1
	if(PcdAnticoll(UID, PICC_ANTICOLL1)!= MI_OK)
	{
		log_i("\r\nAnticoll:fail");
		return 1;		
	}
	else
	{
		log_i("\r\nAnticoll:ok  UID:%02x %02x %02x %02x",UID[0],UID[1],UID[2],UID[3]);
	}
	
	//Select 选卡
	if(PcdSelect(UID,&SAK)!= MI_OK)
	{
		log_i("\r\nSelect:fail");
		return 1;		
	}
	else
	{
		log_i("\r\nSelect:ok  SAK:%02x",SAK);
	}
	
	
	//Halt
	if(PcdHalt() != MI_OK)
	{
		log_i("\r\nHalt:fail");
		return 1;		
	}
	else
	{
		log_i("\r\nHalt:ok");
	}	
	return 0;
}


void rfid_si522_init(void)
{
    rfid_si522_gpio_init();
    rfid_si522_i2c_init();
    // 复位
    rfid_si522_reset();
    // 初始化
    	// Reset baud rates
	si522_write_reg(TxModeReg, TxModeReg_Val);
	si522_write_reg(RxModeReg, RxModeReg_Val);
	// Reset ModWidthReg
	si522_write_reg(ModWidthReg, ModWidthReg_Val);

	// RxGain:110,43dB by default
	si522_write_reg(RFCfgReg, RFCfgReg_Val);
	
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	si522_write_reg(TModeReg, TModeReg_Val);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	si522_write_reg(TPrescalerReg, TPrescalerReg_Val);	// TPreScaler = TModeReg[3..0]:TPrescalerReg
	si522_write_reg(TReloadRegH, TReloadRegH_Val);		// Reload timer 
	si522_write_reg(TReloadRegL, TReloadRegL_Val);
	
	si522_write_reg(TxASKReg, TxASKReg_Val);			// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	si522_write_reg(ModeReg, ModeReg_Val);				// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	
	// Turn on the analog part of receiver 
	si522_write_reg(CommandReg, PCD_IDLE);
	
	si522_antenna_on();			// Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
}