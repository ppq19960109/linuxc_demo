/***********************************************************
*文件名     : session_i2c.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 会话服务
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "error_msg.h"
#include "base_api.h"
#include "link_list.h"
#include "log_api.h"

/*ioctl 指令类型*/
typedef enum
{
	/*设置i2c客户端地址*/
	SESSION_I2C_IOCTL_CMD_SLAVE_ADDR_SET,
	/*设置i2c寄存器地址*/
	SESSION_I2C_IOCTL_CMD_REG_ADDR_SET,

	SESSION_I2C_IOCTL_CMD_NB = 0xFF
}session_i2c_ioctl_cmd_t;

typedef struct i2c_info_s
{
   int iFd;
   int iSlaveAddr;
   int iRegAddr;
}i2c_info_t;

simple_link_list_class_t *g_pstI2cList = NULL;


static int _session_i2c_cmp(void *pData1, int pData1Len, void *pData2, int pData2Len)
{
 	i2c_info_t *p1 = (i2c_info_t *)pData1;
	i2c_info_t *p2 = (i2c_info_t *)pData2;

	return p1->iFd - p2->iFd;
}


/**************************************************************
*描述: 会话打开
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_i2c_open(const char * pSessionDev, int iMode)
{
	if(NULL == pSessionDev)
	{
		error_num = ParaErr;
		return GeneralErr;
	}
	
	int iFd;
	
	if(NULL == g_pstI2cList)
	{
		g_pstI2cList = new_simple_link_list();
		if(NULL == g_pstI2cList)
		{
			return GeneralErr;
		}
	}
	
	iFd = base_open(pSessionDev, O_RDWR);
	if(iFd > 0)
	{
		/*存储gpio信息*/
		i2c_info_t stI2cInfo = {0};
		stI2cInfo.iFd = iFd;

		g_pstI2cList->headInsert(
			g_pstI2cList, &stI2cInfo, sizeof(i2c_info_t));
	}
	return iFd;
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_i2c_close(int iSessionId)
{
	int iRet = 0;
	/*查找gpio信息*/
	i2c_info_t stI2cInfoTmp = {0};
	stI2cInfoTmp.iFd = iSessionId;
	i2c_info_t *pstGpioInfo = 
		g_pstI2cList->find(
			g_pstI2cList, 
			_session_i2c_cmp,
			&stI2cInfoTmp,
			sizeof(i2c_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the I2c session.\n");
		return GeneralErr;
	}
	iRet = base_close(iSessionId);

	g_pstI2cList->del(g_pstI2cList, pstGpioInfo);
	
	return iRet;
}

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_i2c_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	/*查找gpio信息*/
	i2c_info_t stI2cInfoTmp = {0};
	stI2cInfoTmp.iFd = iSessionId;
	i2c_info_t *pstGpioInfo = 
		g_pstI2cList->find(
			g_pstI2cList, 
			_session_i2c_cmp,
			&stI2cInfoTmp,
			sizeof(i2c_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the I2c session.\n");
		return GeneralErr;
	}

	if(SESSION_I2C_IOCTL_CMD_SLAVE_ADDR_SET == iCmd)
	{
		pstGpioInfo->iSlaveAddr = ulArg;
	}
	else if(SESSION_I2C_IOCTL_CMD_REG_ADDR_SET == iCmd)
	{
		pstGpioInfo->iRegAddr = ulArg;
	}

	return NoErr;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_i2c_read(int iSessionId, void * pData, int iDataLen)
{
	int iRet = 0;
	unsigned char ucI2cAddr = 0;
	unsigned char ucRegAddr = 0;
	/*查找gpio信息*/
	i2c_info_t stI2cInfoTmp = {0};
	stI2cInfoTmp.iFd = iSessionId;
	i2c_info_t *pstGpioInfo = 
		g_pstI2cList->find(
			g_pstI2cList, 
			_session_i2c_cmp,
			&stI2cInfoTmp,
			sizeof(i2c_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the I2c session.\n");
		return GeneralErr;
	}
	ucI2cAddr = (unsigned char)pstGpioInfo->iSlaveAddr;
	ucRegAddr = (unsigned char)pstGpioInfo->iRegAddr;
	
	base_ioctl(iSessionId, I2C_SLAVE, ucI2cAddr >> 1);
	base_ioctl(iSessionId, I2C_TIMEOUT, 1);
	base_ioctl(iSessionId, I2C_RETRIES, 1);

	iRet = base_write(iSessionId, &ucRegAddr, 1);
	if(1 != iRet)
	{
		HY_ERROR("Set RegAddr error.\n");
		return iRet;
	}
	return base_read(iSessionId, pData, iDataLen); 
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_i2c_write(int iSessionId, const void * pData, int iDataLen)
{
	int iRet = 0;
	unsigned char ucI2cAddr = 0;
	unsigned char ucRegAddr = 0;
	unsigned char* pWriteData = base_calloc(1, iDataLen + 1);
	if(NULL == pWriteData)
	{
		return GeneralErr;
	}
	
	/*查找gpio信息*/
	i2c_info_t stI2cInfoTmp = {0};
	stI2cInfoTmp.iFd = iSessionId;
	i2c_info_t *pstGpioInfo = 
		g_pstI2cList->find(
			g_pstI2cList, 
			_session_i2c_cmp,
			&stI2cInfoTmp,
			sizeof(i2c_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the I2c session.\n");
		base_free(pWriteData);
		return GeneralErr;
	}
	ucI2cAddr = (unsigned char)pstGpioInfo->iSlaveAddr;
	ucRegAddr = (unsigned char)pstGpioInfo->iRegAddr;

	pWriteData[0] = ucRegAddr;
	
	base_memcpy(&pWriteData[1], (void*)pData, iDataLen);

	base_ioctl(iSessionId, I2C_SLAVE, ucI2cAddr >> 1);
	base_ioctl(iSessionId, I2C_TIMEOUT, 1);
	base_ioctl(iSessionId, I2C_RETRIES, 1);

	iRet = base_write(iSessionId, pWriteData, iDataLen + 1);
	
	base_free(pWriteData); 

	return iRet;
}
