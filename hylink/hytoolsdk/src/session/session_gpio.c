/***********************************************************
*文件名     : session_gpio.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 会话服务
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "error_msg.h"
#include "base_api.h"
#include "link_list.h"
#include "log_api.h"
#include "session_gpio.h"

#define GPIO_PATH_MAX_LEN 128
#define GPIO_COMMAND_MAX_LEN 128
#define GPIO_VALUE_MAX_LEN 8

#define SESSION_GPIO_DIRECTION_IN_STR	"in"
#define SESSION_GPIO_DIRECTION_OUT_STR	"out"
#define SESSION_GPIO_EDGE_NONE_STR		"none"
#define SESSION_GPIO_EDGE_RISING_STR	"rising"
#define SESSION_GPIO_EDGE_FALLING_STR	"falling"
#define SESSION_GPIO_EDGE_BOTH_STR		"both"

/*会话工作模式*/
typedef enum
{
	/*可读可写*/
	SESSION_GPIO_MODE_RDWR,
	/*只读*/
	SESSION_GPIO_MODE_RDONLY,
	/*只写*/
	SESSION_GPIO_MODE_WRONLY,

	
	SESSION_GPIO_MODE_NB = 0xFF
}session_gpio_mode_t;
	
/*方向*/
typedef enum
{
	SESSION_GPIO_DIRECTION_IN = 0,
	SESSION_GPIO_DIRECTION_OUT = 1,
	
	SESSION_GPIO_DIRECTION_NB = 0xFF
} session_gpio_direction_t;

/*触发类型*/
typedef enum
{
	SESSION_GPIO_EDGE_NONE = 0,
	SESSION_GPIO_EDGE_RISING = 1,
	SESSION_GPIO_EDGE_FALLING = 2,
	SESSION_GPIO_EDGE_BOTH = 3,
	
	SESSION_GPIO_EDGE_NB = 0xFF
} session_gpio_edge_t;

/*ioctl 指令类型*/
typedef enum
{
	/*设置GPIO方向, 0-in 1-out*/
	SESSION_GPIO_IOCTL_CMD_DIRECTION_SET,
	/*获取GPIO方向*/
	SESSION_GPIO_IOCTL_CMD_DIRECTION_GET,
	
	/*设置GPIO触发方式, 0-none 1-rising(上升沿触发) 2-falling(下降沿触发) 3-both(边沿触发)*/
	SESSION_GPIO_IOCTL_CMD_EDGE_SET,
	/*获取GPIO触发方式*/
	SESSION_GPIO_IOCTL_CMD_EDGE_GET,

	SESSION_GPIO_IOCTL_CMD_NB = 0xFF
}session_gpio_ioctl_cmd_t;
	

typedef struct gpio_info_s
{
   int iFd;
   int iGpio;
}gpio_info_t;

simple_link_list_class_t *g_pstGpioList = NULL;


static int _session_gpio_cmp(void *pData1, int pData1Len, void *pData2, int pData2Len)
{
 	gpio_info_t *p1 = (gpio_info_t *)pData1;
	gpio_info_t *p2 = (gpio_info_t *)pData2;

	return p1->iFd - p2->iFd;
}

/**************************************************************
*描述: 会话打开
*参数: pSessionDev:gpio号，例如打开GPIO10，则改参数传入字符串“10”
*返回: 成功返回会话ID，失败返回-1；
*说明: gpio默认方向in
**************************************************************/
int session_gpio_open(const char * pSessionDev, int iMode)
{
	if(NULL == pSessionDev)
	{
		error_num = ParaErr;
		return GeneralErr;
	}
	
	if(NULL == g_pstGpioList)
	{
		g_pstGpioList = new_simple_link_list();
		if(NULL == g_pstGpioList)
		{
			return GeneralErr;
		}
	}

	int iFd = -1;
	char acGpioDir[GPIO_PATH_MAX_LEN] ={0};
	char acGpioPath[GPIO_PATH_MAX_LEN] ={0};
	char acCommand[GPIO_COMMAND_MAX_LEN] ={0};

	base_snprintf(acGpioDir, GPIO_PATH_MAX_LEN, "/sys/class/gpio/gpio%s", pSessionDev);
	if(base_access(acGpioDir, R_OK)!=0)
	{
		base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
			"echo %s > /sys/class/gpio/export", pSessionDev);
		base_system(acCommand, NULL, 0);
	}
	
	/*设置方向*/
	base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
		"echo '%s' > /sys/class/gpio/gpio%s/direction", 
		SESSION_GPIO_DIRECTION_IN_STR,
		pSessionDev);
	base_system(acCommand, NULL, 0);
	
	/*设置中断，边缘触发*/
	if(SESSION_GPIO_MODE_RDWR == iMode ||
		SESSION_GPIO_MODE_WRONLY == iMode)
	{
		base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
			"echo 'both' > /sys/class/gpio/gpio%s/edge",
			pSessionDev);
		base_system(acCommand, NULL, 0);
	}
	
	/*打开gpio驱动*/
	base_snprintf(acGpioPath, GPIO_PATH_MAX_LEN, "/sys/class/gpio/gpio%s/value", pSessionDev);
	if(SESSION_GPIO_MODE_RDWR == iMode)
	{
		iFd = base_open(acGpioPath, O_RDWR);
	}
	else if(SESSION_GPIO_MODE_RDONLY == iMode)
	{
		iFd = base_open(acGpioPath, O_RDONLY);
	}
	else if(SESSION_GPIO_MODE_WRONLY == iMode)
	{
		iFd = base_open(acGpioPath, O_WRONLY);
	}
	if(iFd < 0)
    {
        HY_ERROR("Open gpio%d dev failed!\n");
		error_num = FileOpenErr;
        return GeneralErr;
    }

	/*存储gpio信息*/
	gpio_info_t stGpioInfo = {0};
	stGpioInfo.iFd = iFd;
	stGpioInfo.iGpio = base_atoi((char *)pSessionDev);
	g_pstGpioList->headInsert(
		g_pstGpioList, &stGpioInfo, sizeof(gpio_info_t));
	
	return iFd;
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_gpio_close(int iSessionId)
{
	int iRet = 0;
	char acCommand[GPIO_COMMAND_MAX_LEN] ={0};
	
	if(NULL == g_pstGpioList)
	{
		g_pstGpioList = new_simple_link_list();
		if(NULL == g_pstGpioList)
		{
			return GeneralErr;
		}
	}
	
	/*查找gpio信息*/
	gpio_info_t stGpioInfoTmp = {0};
	stGpioInfoTmp.iFd = iSessionId;
	gpio_info_t *pstGpioInfo = 
		g_pstGpioList->find(
			g_pstGpioList, 
			_session_gpio_cmp,
			&stGpioInfoTmp,
			sizeof(gpio_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the gpio session.\n");
		return GeneralErr;
	}
	
	/*关闭驱动*/
	iRet = base_close(iSessionId);

	/*取消导出*/
	base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
			"echo %d > /sys/class/gpio/unexport", pstGpioInfo->iGpio);
	base_system(acCommand, NULL, 0);
	
	/**/
	g_pstGpioList->del(g_pstGpioList, pstGpioInfo);
	return iRet;
}

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_gpio_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	char acCommand[GPIO_COMMAND_MAX_LEN] ={0};
	char acCmdResult[GPIO_COMMAND_MAX_LEN] ={0};
	if(NULL == g_pstGpioList)
	{
		g_pstGpioList = new_simple_link_list();
		if(NULL == g_pstGpioList)
		{
			return GeneralErr;
		}
	}
	
	/*查找gpio信息*/
	gpio_info_t stGpioInfoTmp = {0};
	stGpioInfoTmp.iFd = iSessionId;
	gpio_info_t *pstGpioInfo = 
		g_pstGpioList->find(
			g_pstGpioList, 
			_session_gpio_cmp,
			&stGpioInfoTmp,
			sizeof(gpio_info_t)
		);
	if(NULL == pstGpioInfo)
	{
		HY_ERROR("Not found the gpio session.\n");
		return GeneralErr;
	}

	switch(iCmd)
	{
		case SESSION_GPIO_IOCTL_CMD_DIRECTION_SET:
			{
				if(SESSION_GPIO_DIRECTION_OUT == ulArg)
				{
					/*gpio反向设置为out，此时触发类型必须为none*/
					base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
						"echo '%s' > /sys/class/gpio/gpio%d/edge", 
						SESSION_GPIO_EDGE_NONE_STR,
						pstGpioInfo->iGpio);
					base_system(acCommand, NULL, 0);
				}
				base_memset(acCommand, 0x0, GPIO_COMMAND_MAX_LEN);
				base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
					"echo '%s' > /sys/class/gpio/gpio%d/direction", 
					ulArg == SESSION_GPIO_DIRECTION_OUT ? SESSION_GPIO_DIRECTION_OUT_STR : SESSION_GPIO_DIRECTION_IN_STR,
					pstGpioInfo->iGpio);
				base_system(acCommand, NULL, 0);
			}
			break;
		case SESSION_GPIO_IOCTL_CMD_DIRECTION_GET:
			{
				base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
					"cat /sys/class/gpio/gpio%d/direction", 
					pstGpioInfo->iGpio);
				base_system(acCommand, acCmdResult, GPIO_COMMAND_MAX_LEN);
				if(!base_strncmp(acCmdResult,
					SESSION_GPIO_DIRECTION_IN_STR, 
					base_strlen(SESSION_GPIO_DIRECTION_IN_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_DIRECTION_IN;
				}
				else if(!base_strncmp(acCmdResult,
					SESSION_GPIO_DIRECTION_OUT_STR, 
					base_strlen(SESSION_GPIO_DIRECTION_OUT_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_DIRECTION_OUT;
				}
				else
				{
					return GeneralErr;
				}
			}
			break;
		case SESSION_GPIO_IOCTL_CMD_EDGE_SET:
			{
				if(SESSION_GPIO_EDGE_NONE != ulArg)
				{
					/*如果开启中断触发，则gpio方向必须为in*/
					base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
						"echo '%s' > /sys/class/gpio/gpio%d/direction", 
						SESSION_GPIO_DIRECTION_IN_STR,
						pstGpioInfo->iGpio);
					base_system(acCommand, NULL, 0);
				}
				char acEdge[GPIO_VALUE_MAX_LEN] = {0};
				switch(ulArg)
				{
					case SESSION_GPIO_EDGE_NONE:
						base_strncpy(acEdge, SESSION_GPIO_EDGE_NONE_STR, GPIO_VALUE_MAX_LEN);
						break;
					case SESSION_GPIO_EDGE_RISING:
						base_strncpy(acEdge, SESSION_GPIO_EDGE_RISING_STR, GPIO_VALUE_MAX_LEN);
						break;
					case SESSION_GPIO_EDGE_FALLING:
						base_strncpy(acEdge, SESSION_GPIO_EDGE_FALLING_STR, GPIO_VALUE_MAX_LEN);
						break;
					case SESSION_GPIO_EDGE_BOTH:
						base_strncpy(acEdge, SESSION_GPIO_EDGE_BOTH_STR, GPIO_VALUE_MAX_LEN);
						break;
					default:
						error_num = ParaErr;
						return GeneralErr;
				}
				base_memset(acCommand, 0x0, GPIO_COMMAND_MAX_LEN);
				base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
					"echo '%s' > /sys/class/gpio/gpio%d/edge", 
					acEdge,
					pstGpioInfo->iGpio);
				base_system(acCommand, NULL, 0);
			}
			break;
		case SESSION_GPIO_IOCTL_CMD_EDGE_GET:
			{
				base_snprintf(acCommand, GPIO_COMMAND_MAX_LEN, 
					"cat /sys/class/gpio/gpio%d/edge", 
					pstGpioInfo->iGpio);
				base_system(acCommand, acCmdResult, GPIO_COMMAND_MAX_LEN);
				if(!base_strncmp(acCmdResult,
					SESSION_GPIO_EDGE_NONE_STR, 
					base_strlen(SESSION_GPIO_EDGE_NONE_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_EDGE_NONE;
				}
				else if(!base_strncmp(acCmdResult,
					SESSION_GPIO_EDGE_RISING_STR, 
					base_strlen(SESSION_GPIO_EDGE_RISING_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_EDGE_RISING;
				}
				else if(!base_strncmp(acCmdResult,
					SESSION_GPIO_EDGE_FALLING_STR, 
					base_strlen(SESSION_GPIO_EDGE_FALLING_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_EDGE_FALLING;
				}
				else if(!base_strncmp(acCmdResult,
					SESSION_GPIO_EDGE_BOTH_STR, 
					base_strlen(SESSION_GPIO_EDGE_BOTH_STR))
				)
				{
					*(int *)ulArg = SESSION_GPIO_EDGE_BOTH;
				}
				else
				{
					return GeneralErr;
				}
			}
			break;
		default:
			error_num = ParaErr;
			return GeneralErr;
	}

	return NoErr;
}

/**************************************************************
*描述: 定位数据读取位置
*参数: @int iSessionId: 会话ID
*	   @int iOffset: 偏移量
*	   @int iWhence: 起始位置
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_gpio_lseek(int iSessionId, int iOffset, int iWhence)
{
	return base_lseek(iSessionId, iOffset, iWhence);
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_gpio_read(int iSessionId, void * pData, int iDataLen)
{
	if(-1 == base_lseek(iSessionId, 0, SEEK_SET))
	{
		return GeneralErr;
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
int session_gpio_write(int iSessionId, void * pData, int iDataLen)
{
	if(-1 == base_lseek(iSessionId, 0, SEEK_SET))
	{
		return GeneralErr;
	}
	
	return base_write(iSessionId, pData, iDataLen);
}
