
#include "pn512.h"
#include "ISO14443A.h"

#define MAXRLEN 64

unsigned char  SelectedSnr[4];
unsigned char  cosBuffer[200];
unsigned char  TagType[3];
unsigned char	keyPICC[6];
unsigned char	Default_KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};

/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          	  	pTagType[OUT]：卡片类型代码
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
  unsigned short  unLen;
  unsigned char ucComMF522Buf[MAXRLEN]; 
  ClearBitMask(Status2Reg,0x08);
  WriteRawRC(BitFramingReg,0x07);
  //SetBitMask(TxControlReg,0x03);
	ucComMF522Buf[0] = req_code;
	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

   if ((status == MI_OK) && (unLen == 0x10)){	*(pTagType+1) = ucComMF522Buf[0];	*(pTagType+2) = ucComMF522Buf[1];	}
   else{   status = MI_ERR;   }
   
   return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned short  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
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
			 {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned short  unLen;
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
    {   TagType[0] =ucComMF522Buf[0]; status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned short  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned short  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned short  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);

        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：扣款和充值
//参数说明dd_mode[IN]:命令字
//          0xc0--扣款
//          0xc1--充值
//          addr[IN]:钱包地址
//          *pValueIn[IN]:4字节增减值，低位在前，高位在后
/////////////////////////////////////////////////////////////////////
char PcdValue(unsigned char dd_mode,unsigned char  addr,unsigned char  *pValue)
{
    char status;
    unsigned short  unLen;
    unsigned char  ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
 
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        memcpy(ucComMF522Buf, pValue, 4);
       //  for (i=0; i<4; i++)
        // {    ucComMF522Buf[i] = *(pValue+i);   }
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_ERR)
        {    status = MI_OK;    }
    }
    
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]); 
   
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
 
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    return status;
}
////////////////////////////////////////////////////////////////////
//功    能：备份钱包
//参数说明sourceaddr[IN]:源地址
//          goaladdr[IN]:目标地址
//返回值    成功返回 MI_OK
/////////////////////////////////////////////////////////////////////
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
	  char status;
    unsigned short  unLen;
    unsigned char  ucComMF522Buf[MAXRLEN];
	
	  ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
	  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
	
	  if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
    
    if(status ==MI_OK)
		{
			ucComMF522Buf[0] =0;ucComMF522Buf[1] =0;ucComMF522Buf[2] =0;ucComMF522Buf[3] =0;
			CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
			status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
			if(status) return(MI_ERR);
		}
    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
	  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
		if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
     {   status = MI_ERR;   }
		 return(status);
}
/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    unsigned short unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++){	WriteRawRC(FIFODataReg, *(pIndata+i));	}
		WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}
/****************************************************************/
/*名称: HL_Active */
/*功能: 该函数实现高级MIFARE卡激活命令*/
/*输入: Block_Adr: 块地址*/
/*输出: 操作状态码*/
/* 读出数据存于RevBuffer中*/
/****************************************************************/
char HL_Active(unsigned char Block_Adr, unsigned char Mode)
{
		unsigned char  status;
    PcdHalt();	/* Halt */
    status = PcdRequest(PICC_REQALL,TagType);	
    if(status)
		{return(MI_NOTAGERR);}

    status = PcdAnticoll(SelectedSnr)	;		
    if(status)
     {return(MI_ERR );}

    status =PcdSelect(SelectedSnr);				
    if(status)
		{return(MI_ERR);}

    status  = PcdAuthState(Mode,Block_Adr,keyPICC,SelectedSnr);	
    if(status)
    {return(MI_ERR);}

    return status;
}
/****************************************************************/
/*名称: HL_Read */
/*功能: 该函数实现高级读命令*/
/*输入: Block_Adr: 块地址*/
/*      Mode     : 认证密码方式A或B*/
/*      buff     : 数据首地址      */
/*输出: 操作状态码*/
/* 读出数据存于RevBuffer中*/
/****************************************************************/
char HL_Read(unsigned char  *buff, unsigned char Block_Adr, unsigned char Mode)
{
	unsigned char	status;
	status = HL_Active(Block_Adr, Mode);
	if(status){	return MI_ERR;	}
	status= PcdRead(Block_Adr, buff);
	if(status){ return MI_ERR; }
	return MI_OK;
}
/****************************************************************/
/*名称: HL_Write */
/*输入: Block_Adr: 块地址*/
/*      Mode     : 认证密码方式A或B*/
/*      buff     : 待写入数据首地址      */
/*输出: 操作状态码*//*输出:操作状态码*/
/****************************************************************/
char  HL_Write(unsigned char  *buff, unsigned char Block_Adr, unsigned char Mode)
{
    unsigned char status;
    status = HL_Active(Block_Adr, Mode);
    if(status){return MI_ERR;}
    status = PcdWrite(Block_Adr,buff);
    if(status){return MI_ERR;}
		return MI_OK;
}

/****************************************************************/
/*名称: MIF_Initial */
/*功能: 该函数实现MIFARE卡初始化值操作*/
/*输入: buff: 四个字节初始化数值起始地址*/
/* Block_Adr: 块地址*/
/*输出: MI_OK: 初始成*/
/****************************************************************/
char MIF_Initial(unsigned char  *buff, unsigned char Block_Adr)
{
  unsigned char 	temp;
  unsigned char		i;
  for(i = 0; i < 4; i++){	*(buff + 4 + i) = ~(*(buff + i)); }
	for(i = 0; i < 4; i++){ *(buff + 8 + i) = *(buff + i); }
  *(buff + 12) = Block_Adr;
  *(buff + 13) = ~Block_Adr;
  *(buff + 14) = Block_Adr;
  *(buff + 15) = ~Block_Adr;
  temp =PcdWrite(Block_Adr,buff );
  return temp;
}
#if 1
/*************************************************
*Mifare_Pro复位
*input: para      = PCD BUFERR SIZE
*output: state 
         (if state is ok, store all the data to cosBuffer)
**************************************************/
unsigned char g_bIblock=0;

char PcdRats(void)
{
    char status;
    unsigned short  unLen;
	  unsigned char   len;
	  unsigned char *dp = cosBuffer+2;
   // ClearBitMask(Status2Reg,0x08);
    dp[0] = 0xe0;
    dp[1] = 0x51;
    CalulateCRC(dp,2,&dp[2]);
	  status =PcdComMF522(PCD_TRANSCEIVE,dp,4,dp,&unLen);

    if(status)
    {
      return MI_NOTAGERR;
    }
		len = unLen/8 -2;
    if(len<=62)
    {
     cosBuffer[0]=0;
     cosBuffer[1]=len;
     g_bIblock = 0; 
     return(MI_OK);
    }
    else 
    { return MI_ERR; }
    
}
/////////////////////////////////////////////////////////////////////
//向ISO14443-4卡发送COS命令
//input:CID_NAD  = 是否包含CID或NAD		CID卡片标识符 NAD节点
//       pLen     = 命令长度
//      pBase= COS命令
//ouput:pLen     = 返回数据长度
//      pBase+5= 返回数据
//P为机具，C为卡片，》表示数据发送方向）
//P》C    1A 01 00 A4
//C》P    AA 01 
//P》C    1B 01 00 00
//C》P    AB 01
//P》C    0A 01 02 3F 00
//C》P    0A 01 6F 15 84 0E 31 50 41 59 2E 53 59 53 2E 44 44 46 30 31 A5 03 88 01 01 90 00 

/////////////////////////////////////////////////////////////////////
char CPUCardCom(unsigned char CID_NAD,unsigned char *pLen,unsigned char *pBase)
{
    char   status =0;
	  unsigned short  timeout =30;
	  unsigned short  unLen;
	  unsigned char   retlen;
    unsigned char   CidNadLg,PCB_I,PCB_R;
    unsigned char   sendlgok,sendlgnow,sendlgsum,recelgnow,recelgsum;
    
    recelgnow = 0x00;
    recelgsum = 0x00;
    sendlgnow = 0x00;
    sendlgok  = 0x00;
    sendlgsum = *pLen;
    PCB_R     = 0xAA;

	  if(pBase[0] ==0x80 &&(pBase[1] ==0x54 || pBase[1] == 0xA8))
			timeout = 512;
		else if(pBase[0]== 0x80 && pBase[1]==0xdc)
      timeout = 150;
		else if(pBase[0]== 0x00 && pBase[1]==0xB2)
			timeout = 150;
		else
			timeout =30;
		
		WriteRawRC(TReloadRegL,timeout&0xff);
	  WriteRawRC(TReloadRegH,timeout>>8);
    if (CID_NAD & 0xF0)
    {   CidNadLg = 1;    }
    else
    {   CidNadLg = 0;    }    
    if (sendlgsum > 50-1-CidNadLg)
    {
        sendlgnow  = 50-1-CidNadLg;
        sendlgsum -= sendlgnow;
        PCB_I = 0x1A+g_bIblock;
    }    
    else
    {   
        sendlgnow  = sendlgsum;
        PCB_I =0x0A+g_bIblock;
    }
    g_bIblock ^=0x01;    
    cosBuffer[0] =PCB_I;
    if(CidNadLg)	cosBuffer[1]= 1;
    memcpy(&cosBuffer[CidNadLg+1], pBase, sendlgnow);   
    sendlgok += sendlgnow;
		CalulateCRC(&cosBuffer[0],(sendlgnow+CidNadLg+1),&cosBuffer[sendlgnow+CidNadLg+1]);
		status = PcdComMF522(PCD_TRANSCEIVE,cosBuffer,(sendlgnow+CidNadLg+3),cosBuffer,&unLen);
//////////////////////////////////////////////
    while (status == MI_OK)
    {
      retlen = unLen/8 -2;
   	  if (retlen > (60+1+CidNadLg))
   	  {   
            return(MI_ERR);
      }
		  if ((cosBuffer[0] & 0xF0) == 0x00)  //命令通讯结束
		  {
					recelgnow = retlen - 1 - CidNadLg;
					memcpy(pBase+3+recelgsum, &cosBuffer[1 + CidNadLg], recelgnow);    
					recelgsum += recelgnow;
					*(pLen+3) = recelgsum;
				  status =MI_OK;
					break;
			}  
		  if ((cosBuffer[0] & 0xF0) == 0xA0)
					//发送后续数据
			{              
            if(sendlgsum > 50-1-CidNadLg)
            {  
                sendlgnow  = 50-1-CidNadLg; 
                sendlgsum -= sendlgnow; 
                PCB_I =0x1A + g_bIblock;
            }    
            else
            {
            	sendlgnow = sendlgsum;
                PCB_I =0x0a+g_bIblock;
            }
            g_bIblock ^=0x01;
            cosBuffer[0] =PCB_I;
            if(CidNadLg) cosBuffer[1]= 1;
            memcpy(&cosBuffer[CidNadLg+1], pBase+sendlgok, sendlgnow); 
            sendlgok += sendlgnow;
						CalulateCRC(&cosBuffer[0],sendlgnow+CidNadLg+1,&cosBuffer[sendlgnow+CidNadLg+1]);
            status = PcdComMF522(PCD_TRANSCEIVE,cosBuffer,sendlgnow+CidNadLg+3,cosBuffer,&unLen);
            continue;
       }
       if ((cosBuffer[0] & 0xF0) == 0x10)
        //接收后续数据
        {
            recelgnow = retlen - 1 - CidNadLg;
            memcpy(pBase+3 + recelgsum, &cosBuffer[CidNadLg+1], recelgnow);  
            recelgsum += recelgnow;
            PCB_R =0xAA +g_bIblock;
            g_bIblock ^=0x01;
            cosBuffer[0] = PCB_R;
            if(CidNadLg) cosBuffer[1]= 1;
					  CalulateCRC(&cosBuffer[0],2,&cosBuffer[CidNadLg+1]);
					  status = PcdComMF522(PCD_TRANSCEIVE,cosBuffer,CidNadLg+3,cosBuffer,&unLen);
            continue;    
        }
				if(cosBuffer[0]==0xfa)
				{
					cosBuffer[1] =cosBuffer[2] =0x01;
					CalulateCRC(&cosBuffer[0],3,&cosBuffer[3]);
					status = PcdComMF522(PCD_TRANSCEIVE,cosBuffer,5,cosBuffer,&unLen);
				}
    }
    if(timeout>30)	
    {
			WriteRawRC(TReloadRegL,30);
	    WriteRawRC(TReloadRegH,0);
		}			
    return status;
}

//////////////////////////////////////////////////////////////////////
//ISO14443-4 DESELECT
//////////////////////////////////////////////////////////////////////
char Deselect(unsigned char CID)
{
    char status;
    unsigned short  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = 0xCA;
    ucComMF522Buf[1] = CID;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
	  status =PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    return status;
}
/////////////////////////////////////////////////////////////////////
//响应上位机TYPEA卡高级复位命令
/////////////////////////////////////////////////////////////////////
char  CPUCardRst(void)
{
    unsigned short t;
    unsigned char status;
    PcdSelAntenna(0);
    t=2000;
    while(t-->0);
    PcdSelAntenna(1);
    t=3000;
    while(t-->0);
    PcdHalt();	
    t=30000;
    while(t-->0);
    status = PcdRequest(PICC_REQALL,TagType);	
    if(status)
		{return(MI_NOTAGERR);}

    status = PcdAnticoll(SelectedSnr)	;		
    if(status)
     {return(MI_ERR );}

    status =PcdSelect(SelectedSnr);				
    if(status)
		{return(MI_ERR);}
   	
    status =  PcdRats();    
    return(status);
}

#endif
