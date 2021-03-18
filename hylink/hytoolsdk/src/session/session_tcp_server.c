/***********************************************************
*文件名     : session_ser.c
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

#include <sys/types.h>
#include <sys/time.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <arpa/inet.h>
#include <sys/resource.h>

#include "session_base.h"
#include "session_tcp_server.h"
#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"
#include "cjson.h"


/*ioctl 指令类型*/
typedef enum
{
	/*设置串口波特率*/
	SESSION_UART_IOCTL_CMD_BAUDT_RATE_SET,
	/*设置串口数据位*/
	SESSION_UART_IOCTL_CMD_DATA_BITS_SET,
	/*设置串口校验位*/
	SESSION_UART_IOCTL_CMD_PARITY_SET,
	/*设置串口停止位*/
	SESSION_UART_IOCTL_CMD_STOP_BITS_SET,
	
	SESSION_UART_IOCTL_CMD_NB = 0xFF
}session_uart_ioctl_cmd_t;

/**************************************************************
*描述: 会话打开
*参数: pSessionDev:监听地址端口。格式“地址:端口”或“端口”
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_server_open(const char * pSessionDev, int iMode)
{
	int iRet = 0;
	int iFd = 0;
	char acSessionDev[TCP_SERVER_ADDR_MAX_LEN] = {0};
	char acAddr[TCP_SERVER_ADDR_MAX_LEN] = {0};
	short sPort = 0;
	char *pcTmp = NULL;
	in_addr_t inaddr;
	struct hostent *pstHostEnt = NULL;
	struct rlimit rt;
	struct sockaddr_in stServerAaddr;

	base_memset(&stServerAaddr, 0x0, sizeof(stServerAaddr));

	base_strncpy(acSessionDev, 
		(char *)pSessionDev,
		TCP_SERVER_ADDR_MAX_LEN);
	
	/*解析地址*/
	pcTmp = strchr(acSessionDev, ':');
	
	if(NULL == pcTmp)
	{
		base_strncpy(acAddr, "0.0.0.0", TCP_SERVER_ADDR_MAX_LEN);
		stServerAaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sPort = (short)base_atoi((char *)pSessionDev);

		stServerAaddr.sin_family = AF_INET;
		stServerAaddr.sin_port = htons(sPort);
	}
	else
	{
		base_strncpy(acAddr, acSessionDev, pcTmp - acSessionDev);

		sPort = (short)base_atoi(pcTmp + 1);

		if(INADDR_NONE == (inaddr = inet_addr(acAddr)))
		{
			/*域名*/
			if(NULL == (pstHostEnt = gethostbyname(acAddr))) /*是主机名*/
			{
				return -1;
			}
			base_memcpy((char *)&stServerAaddr.sin_addr,
				(char *)pstHostEnt->h_addr, 
				pstHostEnt->h_length);
		}
		else
		{
			/*ip*/
			stServerAaddr.sin_addr.s_addr = inaddr;
		}

		stServerAaddr.sin_family = AF_INET;/*TCP协议*/
		stServerAaddr.sin_port = htons(sPort);
	}
	

	HY_DEBUG("Addr: %s, Port: %d\n", acAddr, sPort);

	
	
	/*设置每个进程允许打开的最大文件数*/
	rt.rlim_max = rt.rlim_cur = 4096;
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
	{
		HY_ERROR("setrlimit error.\n");
		return SessionOpenErr;
	}

	/*创建监听套接字*/
	if((iFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		HY_ERROR("Create socket error.\n");
		return SessionOpenErr;
	}
	
	 /*设置socket属性，端口可以重用*/
	int iSocketOpt = SO_REUSEADDR;
	setsockopt(iFd, SOL_SOCKET, SO_REUSEADDR, &iSocketOpt, sizeof(iSocketOpt));

	/*设置socket为非阻塞模式*/
	fcntl(iFd, F_SETFL, fcntl(iFd, F_GETFL, 0) | O_NONBLOCK);

	/*绑定*/
	iRet = bind(iFd, (struct sockaddr*)&stServerAaddr, sizeof(stServerAaddr));
	if(iRet == -1)
	{
        HY_ERROR("Bind failed.\n");
        return SessionOpenErr;
    }

	/*监听*/
	iRet = listen(iFd, 10);
	if(iRet == -1)
	{
        HY_ERROR("Listen failed.\n");
        return SessionOpenErr;
    }
	
    HY_DEBUG("Open (%s) Success, SessionId: %d\n", pSessionDev, iFd);
	
	/*返回会话ID*/
	return iFd;
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_server_close(int iSessionId)
{
	return 0;
}

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_tcp_server_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	return 0;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存,
*			该数据格式为json，具体格式为{"Ip":"","Port":"","SessionId":""}
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
*说明：对应tcp服务器，该接口的作用是accept
**************************************************************/
int session_tcp_server_read(int iSessionId, void * pData, int iDataLen)
{
	int iClientFd = 0;
	socklen_t iAddrLen = sizeof(struct sockaddr_in);
	struct sockaddr_in stClientAddr;
	char acIp[TCP_IP_STR_MAX_LEN] = {0};
	char acPort[TCP_NUM_MAX_LEN] = {0};
	char acAddr[TCP_SERVER_ADDR_MAX_LEN] = {0};
	char acSessionId[TCP_NUM_MAX_LEN] = {0};
	cJSON *pstJson = NULL;
	char acDataJson[TCP_JSON_MAX_LEN] = {0};
	int iRecvDataLen = 0;
	
	iClientFd = accept(iSessionId,(struct sockaddr*)&stClientAddr, &iAddrLen);
	if(iClientFd < 0)
	{
		HY_ERROR("Accept failed.\n");
        return SessionRecvErr;
	}
	
	base_strcpy(acIp, inet_ntoa(stClientAddr.sin_addr));
	base_snprintf(acPort, 
		TCP_NUM_MAX_LEN, 
		"%d",
		ntohs(stClientAddr.sin_port));
	base_snprintf(acSessionId,
		TCP_NUM_MAX_LEN, 
		"%d", 
		iClientFd);
	base_snprintf(acAddr,
		TCP_SERVER_ADDR_MAX_LEN, 
		"%s:%s", 
		acIp, acPort);
	
	pstJson = cJSON_CreateObject();
	if(NULL == pstJson)
	{
		HY_ERROR("Failed to create json object.\n");
		base_close(iClientFd);
		return SessionRecvErr;
	}

	cJSON_AddStringToObject(pstJson, 
		SESSION_JSON_KEY_COMMAND, 
		SESSION_JSON_COMMAND_VALUE_CONNECT
	);
	
	cJSON_AddStringToObject(pstJson, 
		SESSION_JSON_KEY_ADDR, 
		acAddr
	);
		
	cJSON_AddStringToObject(pstJson, 
		SESSION_JSON_KEY_ID, 
		acSessionId
	);
	
	char *pstr = cJSON_Print(pstJson);
	base_strncpy(acDataJson, pstr, base_strlen(pstr) + 1);
	base_free(pstr);
	
	cJSON_Minify(acDataJson);
	cJSON_Delete(pstJson);

	iRecvDataLen = (base_strlen(acDataJson) + 1) > iDataLen ?
		iDataLen : 
		(base_strlen(acDataJson) + 1);

	base_strncpy(pData,
		acDataJson, iRecvDataLen);

	
	return iRecvDataLen;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_tcp_server_write(int iSessionId, const void * pData, int iDataLen)
{
	return 0;
}
