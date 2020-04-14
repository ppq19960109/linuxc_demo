#ifndef _ISO14443A_H
#define _ISO14443A_H

#include"pn512.h"

extern unsigned char SelectedSnr[4];
extern unsigned char cosBuffer[200];
extern unsigned char TagType[3];
extern unsigned char	keyPICC[6];
extern unsigned char	Default_KEY[6];

/**Function Protype****************************************/
char PcdRequest(unsigned char req_code,unsigned char *pTagType);   
char PcdAnticoll(unsigned char *pSnr);
char PcdSelect(unsigned char *pSnr);         
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);     
char PcdRead(unsigned char addr,unsigned char *pData);     
char PcdWrite(unsigned char addr,unsigned char *pData);    
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);   
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);                                 
char PcdHalt(void);
/***Advanced Function*********************************************************/
char HL_Active(unsigned char Block_Adr, unsigned char Mode);
char HL_Read(unsigned char  *buff, unsigned char Block_Adr, unsigned char Mode);
char HL_Write(unsigned char *buff, unsigned char Block_Adr, unsigned char Mode);
char MIF_Initial(unsigned char  *buff, unsigned char Block_Adr);
char CPUCardRst(void);
char CPUCardCom(unsigned char CID_NAD,unsigned char *pLen,unsigned char *pBase);
#endif
