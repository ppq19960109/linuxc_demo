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

#include "session_tcp_client.h"
#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"
#include "cjson.h"

/*ioctl 指令类型*/
typedef enum
{
	/*设置该客户端指向的服务器会话*/
	SESSION_TCP_CLIENT_IOCTL_CMD_SERVER_ID_SET,
	
	SESSION_TCP_CLIENT_IOCTL_CMD_NB = 0xFF
}session_tcp_client_ioctl_cmd_t;

/**************************************************************
*描述: 会话打开
*参数: pSessionDev:tcp客户端地址，格式“地址:端口”
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_client_open(const char * pSessionDev, int iMode)
{
	int iRet = 0;
	int iFd = 0;
	char acSessionDev[TCP_CLIENT_ADDR_MAX_LEN] = {0};
	char acAddr[TCP_CLIENT_ADDR_MAX_LEN] = {0};
	short sPort = 0;
	char *pcTmp = NULL;
	char *pStrtokPtr = NULL;
	struct sockaddr_in stServrAddr;
	in_addr_t inaddr;
	struct hostent *pstHostEnt = NULL;
	struct timeval timeout;
	fd_set fdr;
	fd_set fdw;
	int iError = -1;
	int iErrorLen = sizeof(iError);
	
	memset(&stServrAddr, 0x0, sizeof(struct sockaddr_in));
	
	base_strncpy(acSessionDev, 
		(char *)pSessionDev, TCP_CLIENT_ADDR_MAX_LEN);
	
	/*解析地址*/
	pcTmp = acSessionDev;
	pcTmp = base_strtok_r(pcTmp, ":", &pStrtokPtr);
	if(NULL == pcTmp)
	{
		HY_ERROR("Address format error.\n");
		return SessionOpenErr;
	}
	
	base_strncpy(acAddr, pcTmp, TCP_CLIENT_ADDR_MAX_LEN);

	pcTmp = base_strtok_r(NULL, ":", &pStrtokPtr);
	if(NULL == pcTmp)
	{
		HY_ERROR("Address format error.\n");
		return SessionOpenErr;
	}

	sPort = (short)base_atoi(pcTmp);

	HY_DEBUG("Addr: %s, Port: %d\n", acAddr, sPort);

	if(INADDR_NONE == (inaddr = inet_addr(acAddr)))
	{
		/*域名*/
		if(NULL == (pstHostEnt = gethostbyname(acAddr))) /*是主机名*/
		{
			return -1;
		}
		base_memcpy((char *)&stServrAddr.sin_addr,
			(char *)pstHostEnt->h_addr, 
			pstHostEnt->h_length);
	}
	else
	{
		/*ip*/
		stServrAddr.sin_addr.s_addr = inaddr;
	}
	
	stServrAddr.sin_family = AF_INET;/*TCP协议*/
	stServrAddr.sin_port = htons(sPort);
	
	/*创建套接字*/
	if((iFd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		HY_ERROR("Create socket error.\n");
		return SessionOpenErr;
	}
	//将套接字设置为非阻塞模式
	fcntl(iFd, F_SETFL, fcntl(iFd, F_GETFL, 0) | O_NONBLOCK);
	
	/*连接服务器*/
	iRet = connect(iFd, (struct sockaddr *)&stServrAddr, sizeof(stServrAddr));
	if(-1 == iRet)
	{
		if(errno == EINPROGRESS)
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
			FD_ZERO(&fdr);
			FD_SET(iFd, &fdr);
			FD_ZERO(&fdw);
			FD_SET(iFd, &fdw);
			iRet = select(iFd+1, &fdr, &fdw, NULL, &timeout);
			if(iRet < 0)
			{
				HY_ERROR("Select error.\n");
				base_close(iFd);
				return SessionOpenErr;
			}
			/*连接超时*/
			else if(iRet == 0)
			{
				HY_ERROR("Connect error.\n");
				base_close(iFd);
				return SessionOpenErr;
			}
			/*[1] 当连接成功建立时,描述符变成只可写,iRet = 1*/
			else if(iRet == 1 && FD_ISSET(iFd, &fdw))
			{
				/*连接成功*/
			}
			/*[2] 当连接建立遇到错误时,描述符变为即可读也可写，iRet = 2 遇到这种情况，可调用getsockopt函数*/
			else if(iRet == 2)
			{
				if(getsockopt(iFd, SOL_SOCKET, SO_ERROR, &iError, (socklen_t *)&iErrorLen) < 0)
				{
					HY_ERROR("Getsockopt error.\n");
					base_close(iFd);
					return SessionOpenErr;
				}
				if(iError == 0)
				{
					/*连接成功*/
				}
				else
				{
					HY_ERROR("Connect error.\n");
					base_close(iFd);
					return SessionOpenErr;
				}
			}
			else
			{
				HY_ERROR("Connect error.\n");
				base_close(iFd);
				return SessionOpenErr;
			}
		}
		else
		{
			HY_ERROR("Connect error.\n");
			base_close(iFd);
			return SessionOpenErr;
		}
	}

	/*连接成功*/
	//将套接字恢复为阻塞模式
	fcntl(iFd,F_SETFL,fcntl(iFd,F_GETFL,0) & ~O_NONBLOCK);

	HY_DEBUG("Open (%s) Success, SessionId: %d\n", pSessionDev, iFd);
	
	/*返回会话ID*/
	return iFd;
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_client_close(int iSessionId)
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
int session_tcp_client_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	return 0;
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_tcp_client_read(int iSessionId, void * pData, int iDataLen)
{
	return base_read (iSessionId, pData, iDataLen);
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_tcp_client_write(int iSessionId, const void * pData, int iDataLen)
{
	return base_write(iSessionId, (void *)pData, iDataLen);
}