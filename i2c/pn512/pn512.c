#include "pn512.h"
#include "ISO14443A.h"
#include "hal_iic.h"
#include <unistd.h>
#define MAXRLEN 64
///////////////////////////////////////////////////////////////////////
// Delay 10ms
///////////////////////////////////////////////////////////////////////
void delay_Xms(unsigned int _ms) {
    unsigned int i, j;
    for (i = 0; i < _ms; i++) {
        for (j = 0; j < 6000; j++)
            ;
    }
}

void delay_ms(unsigned int nms) { usleep(nms * 1000); }

void _nop_() {
    unsigned int i = 0;
    ++i;
    ++i;
}

void PN512_GPIO_Confguration() {
     
}

/////////////////////////////////////////////////////////////////////
//功    能：读PN512寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address) {
    unsigned int val;
    I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = 0X07 ;
    i2c_data.reg_addr = Address;
    i2c_data.addr_byte_num = 1;
    i2c_data.data = 0;
    i2c_data.data_byte_num = 1;

    i2c_read(&i2c_data, &val, 1);
    return val;
}

/////////////////////////////////////////////////////////////////////
//功    能：写PN512寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value) {
     unsigned int val=value;
    I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = 0X07 ;
    i2c_data.reg_addr = Address;
    i2c_data.addr_byte_num = 1;
    i2c_data.data = 0;
    i2c_data.data_byte_num = 1;

    i2c_write(&i2c_data, &val, 1, 2000);
}

/////////////////////////////////////////////////////////////////////
//功    能：复位PN512
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void) {
    // RST_H;
    // delay_Xms(40);
    // RST_L;
    // delay_Xms(40);
    // RST_H;
    delay_Xms(40);
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    delay_Xms(20);

    WriteRawRC(ModeReg, 0x3D);  //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAutoReg, 0x40);
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char type) {
    // unsigned char temp = 0;
    if (type == 'A')  // ISO14443_A
    {
        WriteRawRC(Status2Reg, 0x08);
        WriteRawRC(ControlReg, 0x10);
        WriteRawRC(TxModeReg, 0x00);
        WriteRawRC(RxModeReg, 0x08);
        WriteRawRC(TxAutoReg, 0x40);
        WriteRawRC(DemodReg, 0x4D);
        WriteRawRC(GsNOnReg, 0xF4);
        // WriteRawRC(CWGsPReg,0x3f);
        WriteRawRC(RxThresholdReg, 0x75);
        WriteRawRC(RFCfgReg, 0x67);  //晋中参数：0x69 衢州 ：0x67
        WriteRawRC(ModeReg, 0x3D);
        WriteRawRC(TxControlReg, 0x00);
        WriteRawRC(CollReg, 0x80);
        WriteRawRC(TReloadRegL, 30);  // tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH, 0);
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);
    } else if (type == 'B')  // ISO14443_B//ID_Card
    {
        WriteRawRC(CommandReg, PCD_RESETPHASE);  // SoftReset
        WriteRawRC(ControlReg, 0x10);
        WriteRawRC(ModeReg, 0x3b);
        WriteRawRC(TxModeReg, 0x83);
        WriteRawRC(RxModeReg, 0x83);  // RxModeReg ----select type b
        //		 WriteRawRC(TxControlReg, 0x83);//2016--3--5  add
        WriteRawRC(TxAutoReg, 0x00);
        WriteRawRC(TxSelReg, 0x10);
        WriteRawRC(RxSelReg, 0x86);
        WriteRawRC(RxThresholdReg, 0x77);
        WriteRawRC(DemodReg, 0x4D);
        WriteRawRC(ManualRCVReg, 0x10);
        //		 WriteRawRC(TypeBReg, 0x93); //TypeBReg(1EH)-------AutoTestReg(36H)
        //		 WriteRawRC(TypeBReg, 0x83);
        WriteRawRC(TypeBReg, 0x90);
        WriteRawRC(GsNOffReg, 0x88);
        WriteRawRC(ModWidthReg, 0x26);
        WriteRawRC(RFCfgReg, 0x59);
        WriteRawRC(GsNOnReg, 0x88);
        WriteRawRC(CWGsPReg, 0x20);
        WriteRawRC(ModGsPReg, 0x06);
        ///*****************************///////////////////////////////////////
        WriteRawRC(TxControlReg, 0x00);
    } else {
        return (char)-1;
    }

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：置位寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg, unsigned char mask) {
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清零寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask) {
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned short *pOutLenBit) {
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    volatile unsigned char n;
    volatile unsigned short i;
    switch (Command) {
        case PCD_AUTHENT:
            irqEn = 0x12;
            waitFor = 0x10;
            break;
        case PCD_TRANSCEIVE:
            irqEn = 0x77;
            waitFor = 0x30;
            break;
        default:
            break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++) {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE) {
        SetBitMask(BitFramingReg, 0x80);
    }

    //    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 20000;
    do {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);
    if (i != 0) {
        if (!(ReadRawRC(ErrorReg) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE) {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits) {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                } else {
                    *pOutLenBit = n * 8;
                }
                if (n == 0) {
                    n = 1;
                }
                if (n > MAXRLEN) {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++) {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        } else {
            status = MI_ERR;
        }
    }
    SetBitMask(ControlReg, 0x80);  // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}

/////////////////////////////////////////////////////////////////////
//选择天线1或2  chn=0 时关闭天线
/////////////////////////////////////////////////////////////////////
void PcdSelAntenna(char chn) {
    if (chn)
        SetBitMask(TxControlReg, 0x83);
    else
        ClearBitMask(TxControlReg, 0x83);
}

void PN512_Init(unsigned char choose_mode) {
    PcdReset(); /*Reset PN512*/
    PcdSelAntenna(0);
    delay_ms(2);
    PcdSelAntenna(1); /*open wireless*/
    if (choose_mode == 0)
        M500PcdConfigISOType('A'); /* Configure PN512 Work at TYPEA Mode*/
    else
        M500PcdConfigISOType('B'); /* Configure PN512 Work at TYPEA Mode*/
    PcdSelAntenna(0);
    delay_ms(2);
    PcdSelAntenna(1); /*open wireless*/
    //	PcdHalt();
}
