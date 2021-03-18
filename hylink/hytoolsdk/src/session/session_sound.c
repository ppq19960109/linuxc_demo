/***********************************************************
*文件名     : session_sound.c
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

#include "error_msg.h"
#include "base_api.h"
#include "link_list.h"
#include "log_api.h"

/*ioctl 指令类型*/
typedef enum
{
	/*获取语音忙碌状态*/
	SESSION_SOUND_IOCTL_CMD_BUSY_GET,


	SESSION_SOUND_IOCTL_CMD_NB = 0xFF
}session_sound_ioctl_cmd_t;


/**************************************************************
*描述: 会话打开
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_sound_open(const char * pSessionDev, int iMode)
{
	if(NULL == pSessionDev)
	{
		error_num = ParaErr;
		return GeneralErr;
	}
	return base_open(pSessionDev, O_WRONLY);
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_sound_close(int iSessionId)
{
	return base_close(iSessionId);
}

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_sound_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	
	return NoErr;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_sound_read(int iSessionId, void * pData, int iDataLen)
{
	/*暂不支持read接口*/
	return NoErr;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_sound_write(int iSessionId, void * pData, int iDataLen)
{
	return base_write(iSessionId, pData, iDataLen);
}
