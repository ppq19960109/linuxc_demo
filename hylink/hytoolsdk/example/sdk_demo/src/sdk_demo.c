/***********************************************************
*文件名     : sdk_demo.c
*版   本   : v1.0.0.0
*日   期   : 2019.11.12
*说   明   : sdk_demo
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <dirent.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "log_api.h"
#include "base_api.h"
#include "sdk_demo.h"
#include "session_api.h"

int dataRecvFun(session_ser_class_t *_this, void *pData, int iDataLen, void *pUserData)
{
	HY_DEBUG("iDataLen = %d\n", iDataLen);
	return 0;
}


int main(int argc, char *argv[])
{
	int iCount = 0;
	/*日志初始化*/
	InitLog(LOG_DEBUG, NULL);

	/*初始化会话*/
	session_ser_class_t *pstSessionSer =
		new_session_ser();
	if(NULL == pstSessionSer)
	{
		HY_ERROR("pstSessionSer error.\n");
		return -1;
	}
	session_info_t stSessionInfo = {0};

	/*会话类型*/
	stSessionInfo.iSessionType = SESSION_TYPE_I2C;

	/*会话地址*/
	base_strcpy(stSessionInfo.acSessionAddr, "/dev/i2c-0");
	
	pstSessionSer->open(pstSessionSer, 
		&stSessionInfo);

	//pstSessionSer->ioctl(pstSessionSer,
	//	stSessionInfo.iSessionId,
	//	SESSION_GPIO_IOCTL_CMD_DIRECTION_SET,
	//	SESSION_GPIO_DIRECTION_OUT);

	//pstSessionSer->callback_reg(pstSessionSer, 
	//	stSessionInfo.iSessionId,
	//	SESSION_CB_TYPE_DATA_RECV,
	//	dataRecvFun,
	//	NULL);

	/*设置I2C地址*/
	pstSessionSer->ioctl(
		pstSessionSer,
		stSessionInfo.iSessionId,
		SESSION_I2C_IOCTL_CMD_SLAVE_ADDR_SET,
		0xa8);
	session_data_t SendData = {0};
	/*初始化*/
	SendData.iSessionType = SESSION_TYPE_I2C;
	SendData.iSessionId = stSessionInfo.iSessionId;

	SendData.aucData[0] = (iCount++) % 4;
	SendData.iDataLen = 1;
	HY_DEBUG("\n");
	pstSessionSer->send (pstSessionSer, 
		&SendData,
		NULL,
		0
	);
	
	while(1)
	{
		
		sleep(10);
	}
	
	return 0;
}
