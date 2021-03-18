/***********************************************************
*文件名     : session_tcp_server.h
*版   本   : v1.0.0.0
*日   期   : 2019.07.03
*说   明   : tcp服务器会话基础接口
*修改记录: 
************************************************************/


#ifndef SESSION_TCP_SERVER_H
#define SESSION_TCP_SERVER_H

#define TCP_IP_STR_MAX_LEN					16
#define TCP_NUM_MAX_LEN						8
#define TCP_SERVER_ADDR_MAX_LEN				128
#define TCP_JSON_MAX_LEN					256

/**************************************************************
*描述: 会话打开
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_server_open(const char * pSessionDev, int iMode);

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_tcp_server_close(int iSessionId);

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_tcp_server_ioctl(int iSessionId, int iCmd, unsigned long ulArg);

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_tcp_server_read(int iSessionId, void * pData, int iDataLen);

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_tcp_server_write(int iSessionId, const void * pData, int iDataLen);

#endif /*SESSION_TCP_SERVER_H*/
