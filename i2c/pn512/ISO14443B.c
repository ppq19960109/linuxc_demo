
#include "pn512.h"
#include "ISO14443B.h"

unsigned char ATQB[2]={0};
unsigned char Attrib[2]={0};
unsigned char UUID[10]={0};
/****************************************************
二代身份证是应答标准的REQB指令，但是Attrib指令是非标准的
并且其他的数据都是加密的，需要公安部授权的加密模块才能读取
二代身份证的唯一UID可以通过下列步骤读取:

1: PCD发送REQB命令05 00 00
二代收到后会发送ATQB应答：0x6D, 0x00

2: PCD发送非标准Attrib命令
二代身份证返回应答：0x6D, 0x00

3: PCD发送读取UID命令
二代身份证返回UID应答（10个字节UID）：
****************************************************/
void PcdRequest_IDCard(void)
{
		unsigned char errorcode=0, datanum=0, i=0;
	  ///**********************************zz60754404**************************************/
		WriteRawRC(CommandReg, PCD_TRANSCEIVE);  //excute  Transceive  command
		///**********************************zz60754404**************************************/
		WriteRawRC(FIFOLevelReg, 0x80);
	
		WriteRawRC(FIFODataReg, 0x05);//Send REQB command
		WriteRawRC(FIFODataReg, 0x00);
		WriteRawRC(FIFODataReg, 0x00);
		//-------------------------------

		WriteRawRC(BitFramingReg, 0x80);
	
		delay_ms(10);
		errorcode= ReadRawRC(ErrorReg);   
		datanum=ReadRawRC(FIFOLevelReg); 
		for(i=0;i<datanum;i++)			ATQB[i]= ReadRawRC(FIFODataReg);
		printf("\r\n 1-1:ErrorCode=0x%02X, Len=0x%02X",errorcode,datanum);
		for(i=0;i<datanum;i++)			ATQB[i]= ReadRawRC(FIFODataReg);
		printf("\r\n 1-2:ATQB:");
		for(i=0;i<datanum;i++)		printf(" 0x%02X ", ATQB[i]);printf("\r\n");
		//=======================================================================================
		WriteRawRC(FIFOLevelReg,0x80);
		WriteRawRC(FIFODataReg,0x1d);//Send Attrib command
		WriteRawRC(FIFODataReg,0x00);
		WriteRawRC(FIFODataReg,0x00);
		WriteRawRC(FIFODataReg,0x00);
		WriteRawRC(FIFODataReg,0x00);
		WriteRawRC(FIFODataReg,0x00);
		WriteRawRC(FIFODataReg,0x08);
		WriteRawRC(FIFODataReg,0x01);
		WriteRawRC(FIFODataReg,0x08);

		WriteRawRC(BitFramingReg,0x80);
		
		delay_ms(10);
		errorcode= ReadRawRC(ErrorReg);   
		datanum=ReadRawRC(FIFOLevelReg);
		printf("\r\n 2-1:ErrorCode = 0x%02X, Len=0x%02X",errorcode,datanum);
		for(i=0;i<datanum;i++)	Attrib[i]= ReadRawRC(FIFODataReg);
		printf("\r\n 2-2:Attrib:");
		for(i=0;i<datanum;i++)	printf(" 0x%02X  ", Attrib[i]);printf("\r\n");
	//=======================================================================================
		WriteRawRC(FIFOLevelReg, 0x80);
		WriteRawRC(FIFODataReg, 0x00);   //Send UUID command
		WriteRawRC(FIFODataReg, 0x36);
		WriteRawRC(FIFODataReg, 0x00);
		WriteRawRC(FIFODataReg, 0x00);
		WriteRawRC(FIFODataReg, 0x08);

		WriteRawRC(BitFramingReg, 0x80);
		
		delay_ms(10);
		errorcode= ReadRawRC(ErrorReg);   
		datanum=ReadRawRC(FIFOLevelReg); 
		printf("\r\n 3-1:ErrorCode=0x%02X, Len=0x%02X",errorcode,datanum);
		for(i=0;i<datanum;i++)		UUID[i]= ReadRawRC(FIFODataReg);
		printf("\r\n 3-2:UUID:");
		for(i=0;i<datanum;i++)		printf(" 0x%02X  ", UUID[i]); printf("\r\n");
		printf("\r\n 3-2:UUID:");
		//===============================================================================
}



//unsigned char PcdRequest_IDCard(void)
//{
//	char status;  
//  unsigned short  unLen;
//  unsigned char ucComMF522Buf[64]; 
//  ClearBitMask(Status2Reg,0x08);
//  WriteRawRC(BitFramingReg,0x07);
//	
//	ucComMF522Buf[0] = 0x05;
//	ucComMF522Buf[1] = 0x00;
//	ucComMF522Buf[2] = 0x00;
//	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,3,ucComMF522Buf,&unLen);
//	printf("\r\nID111:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
//	
//	ucComMF522Buf[0],ucComMF522Buf[1],ucComMF522Buf[2],ucComMF522Buf[3],
//	ucComMF522Buf[4],ucComMF522Buf[5],ucComMF522Buf[6],ucComMF522Buf[7],
//	ucComMF522Buf[8],ucComMF522Buf[9],ucComMF522Buf[10],ucComMF522Buf[11]);
//	
//	ClearBitMask(Status2Reg,0x08);
//  WriteRawRC(BitFramingReg,0x07);
//	ucComMF522Buf[0] = 0x1D;
//	ucComMF522Buf[1] = 0x00;
//	ucComMF522Buf[2] = 0x00;
//	ucComMF522Buf[3] = 0x00;
//	ucComMF522Buf[4] = 0x00;
//	ucComMF522Buf[5] = 0x00;
//	ucComMF522Buf[6] = 0x08;
//	ucComMF522Buf[7] = 0x01;
//	ucComMF522Buf[8] = 0x08;
//	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

//	printf("\r\nID222:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
//	ucComMF522Buf[0],ucComMF522Buf[1],ucComMF522Buf[2],ucComMF522Buf[3],
//	ucComMF522Buf[4],ucComMF522Buf[5],ucComMF522Buf[6],ucComMF522Buf[7],
//	ucComMF522Buf[8],ucComMF522Buf[9],ucComMF522Buf[10],ucComMF522Buf[11]);
//	
//	
//	ClearBitMask(Status2Reg,0x08);
//  WriteRawRC(BitFramingReg,0x07);
//	ucComMF522Buf[0] = 0x00;
//	ucComMF522Buf[1] = 0x36;
//	ucComMF522Buf[2] = 0x00;
//	ucComMF522Buf[3] = 0x00;
//	ucComMF522Buf[4] = 0x08;
//	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,5,ucComMF522Buf,&unLen);
//	
//	printf("\r\nID333:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
//	ucComMF522Buf[0],ucComMF522Buf[1],ucComMF522Buf[2],ucComMF522Buf[3],
//	ucComMF522Buf[4],ucComMF522Buf[5],ucComMF522Buf[6],ucComMF522Buf[7],
//	ucComMF522Buf[8],ucComMF522Buf[9],ucComMF522Buf[10],ucComMF522Buf[11]);
//}





