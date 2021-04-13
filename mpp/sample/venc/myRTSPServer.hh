#ifndef _MYRTSPSERVER_HH
#define _MYRTSPSERVER_HH

#ifdef __cplusplus
extern "C" {
#endif
#include "sample_comm.h"
int getVencFrame(int chId,int srcId,unsigned char* buf,int size);
HI_S32 SAMPLE_VENC_H265_H264(int flag);
#ifdef __cplusplus
}
#endif

#endif