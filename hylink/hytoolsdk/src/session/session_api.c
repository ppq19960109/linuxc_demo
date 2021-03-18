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

#include "error_msg.h"
#include "cjson.h"
#include "log_api.h"
#include "error_msg.h"
#include "base_api.h"
#include "link_list.h"
#include "session_api.h"
#include "session_base.h"
#include "hashmap.h"
#include "msg_queue.h"
#include "epoll_api.h"
#include "session_gpio.h"
#include "session_i2c.h"
#include "session_uart.h"
#include "session_sound.h"
#include "session_tcp_client.h"
#include "session_tcp_server.h"


typedef int (*IoctlFun)(int fd, int cmd, ...);
typedef int (*OpenFun)(char * pathname,int flags, ...);
typedef int (*CloseFun)(int fd);
typedef int (*LseekFun)(int fd, int offset, int whence);
typedef int (*ReadableFun)(int fd, int iTimeOut);
typedef int (*ReadFun)(int fd, void * buf, int count);
typedef int (*WriteableFun)(int fd, int iTimeOut);
typedef int (*WriteFun)(int fd, void * buf, int count);

typedef int (*dataSplitCombineFun)(
	session_ser_class_t *_this, 
	unsigned char* pOriginalData, 
	int iOriginalDataLen, 
	unsigned char * pCompleteData, 
	int *piCompleteDataLen,
	void *pUserData
);
typedef int (*dataParserFun)(
	session_ser_class_t *_this,
	void *pData,
	int iDataLen, 
	void *pUserData
);
typedef int (*dataRecvFun)(
	session_ser_class_t *_this,
	void *pData, 
	int iDataLen, 
	void *pUserData
);

/*会话工作模式*/
typedef enum
{
	/*可读可写*/
	SESSION_WORK_MODE_RDWR,
	/*只读*/
	SESSION_WORK_MODE_RDONLY,
	/*只写*/
	SESSION_WORK_MODE_WRONLY,

	
	SESSION_WORK_MODE_NB = 0xFF
}session_work_mode_t;
	
/*会话基础操作接口*/
typedef struct session_base_op_s 
{
	/*会话控制接口*/
	//int ioctl(int fd, int cmd, ...);
	void *pIoctl;
	/*会话打开接口*/
	//int open(const char * pathname,int flags, .../*mode_t mode*/);
	void *pOpen;
	/*会话关闭接口*/
	//int close(int fd);
	void *pClose;
	/*会话数据读取位置设置接口,
	仅对字符类会话生效*/
	//int lseek(int fd, int offset, int whence);
	void *pLseek;
	/*会话可读判断接口，针对不支持系统级select、poll的会话*/
	//int readable(int fd, int iTimeOut);
	void *pReadable;
	/*会话读取接口*/
	//int read(int fd, void * buf, int count);
	void *pRead;
	/*会话可写判断接口，针对不支持系统级select、poll的会话*/
	//int writeable(int fd, int iTimeOut);
	void *pWriteable;
	/*会话写入接口*/
	//int write (int fd, const void * buf, int count);
	void *pWrite;	
}session_base_op_t;

/*会话协议回调接口*/
typedef struct session_cb_s 
{
	/*接收数据包黏连组合回调*/
	/*
	*该回调的作用将接收到断裂的数据组合在一起
	*接收到黏连的数据分割开
	*/
	//int (*dataSplitCombineFun)(session_ser_class_t *_this, unsigned char*, int, unsigned char *, int *, void *pUserData);
	void* pDataSplitCombineCb;
	void* pDataSplitCombineUserData;
	/*接收数据解析回调*/
	/*
	*该回调的作用是解析经过“接收数据包黏连组合回调”处理后的完整数据报文
	*/
	//int (*dataParserFun)(session_ser_class_t *_this, void *, int, void *pUserData);
	void* pDataParserCb;
	void* pDataParserUserData;

	/*接收数据与发送数据 匹配回调*/
	/*
	*该回调的作用是，判断回应报文与之匹配的发送报文
	*/
	//int (*DataMatchingFun)(data_repeater_t *_this, void *pSendData, int iSendDataLen, void *pRecvData, int iRecvDataLen, void *pUserData);
	void* pRespondMatchingCb;
	void* pRespondMatchingUserData;
	/*数据接收回调*/
	/*
	*该回调的作用是用于接收最终的会话报文。
	*/
	//int (*dataParserFun)(session_ser_class_t *_this, void *, int, void *pUserData);
	void* pDataRecvCb;
	void* pDataRecvUserData;
}session_cb_t;

/*回话类型信息*/
typedef struct session_type_info_s  
{  
	/*消息类型*/
	int iSessionType;
	/*是否支持异步处理*/
	int iAsyn;
	/*是否支持系统级的epoll机制*/
	int iSysPollSupport;
	/*工作模式，详见session_work_mode_t*/
	int iWorkMode;
	session_base_op_t stBaseOp;
}session_type_info_t;


/*会话信息*/
typedef struct session_conf_s
{
	/*会话类型*/
	int iSessionType;
	/*会话ID*/
	int iSessionId;
	/*会话地址*/
	char acSessionAddr[SESSION_ADDR_MAX_LEN];
	/*会话活跃*/
	int iActiveFlag;
	/*数据超时时间*/
	int iDataTimeOut;
	/*数据最大重发次数*/
	int iMaxResendTime;
	/*数据最小间隔*/
	int iDataSpacing;

	/*时间戳*/
	base_timeval_t stTimestamp;
	
	/*数据重发器*/
	data_repeater_t *pstDataRepeater;
	
	/*回调接口*/
	session_cb_t stCallbackList;

	/*客户端链表*/
	double_link_list_class_t *pstClientList;
	/*服务器客户端从属关系*/
	void *pParentSession;
	
}session_conf_t;

/*私有类型定义*/
typedef struct session_ser_private_s  
{  
	/*会话hash*/
	hash_map_t *pstSessionHash;
	
	thread_t sessionSysPid;

	/*epoll*/
	epoll_fd_t * pstEpollFd;
	
	/*发送队列*/	
	msg_queue_class_t *pstSendMsgQueue;
	/*异步接收队列*/
	msg_queue_class_t *pstRecvMsgQueue;
	/*同步接收队列*/
	msg_queue_class_t *pstSyncRecvMsgQueue;
	
}session_ser_private_t;


/*会话类型信息*/
session_type_info_t g_astSessionTypeInfo[SESSION_TYPE_MAX_NUM] = {{0}};


/*会话类型信息初始化*/
static int _session_type_init (void)
{
	base_memset(g_astSessionTypeInfo, 
		0x0, SESSION_TYPE_MAX_NUM * sizeof(session_type_info_t));
	/*uart*/
	g_astSessionTypeInfo[SESSION_TYPE_UART].iSessionType = 
		SESSION_TYPE_UART;
	g_astSessionTypeInfo[SESSION_TYPE_UART].iAsyn = 
		1;
	g_astSessionTypeInfo[SESSION_TYPE_UART].iSysPollSupport = 
		1;
	g_astSessionTypeInfo[SESSION_TYPE_UART].iWorkMode = 
		SESSION_WORK_MODE_RDWR;
	g_astSessionTypeInfo[SESSION_TYPE_UART].stBaseOp.pIoctl = 
		session_uart_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_UART].stBaseOp.pOpen =
		session_uart_open;
	g_astSessionTypeInfo[SESSION_TYPE_UART].stBaseOp.pClose 
		= session_uart_close;
	g_astSessionTypeInfo[SESSION_TYPE_UART].stBaseOp.pRead =
		session_uart_read;
	g_astSessionTypeInfo[SESSION_TYPE_UART].stBaseOp.pWrite = 
		session_uart_write;
	
	/*i2c*/
	g_astSessionTypeInfo[SESSION_TYPE_I2C].iSessionType =
	SESSION_TYPE_I2C;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].iAsyn =
		0;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].iSysPollSupport =
		0;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].iWorkMode = 
		SESSION_WORK_MODE_RDWR;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].stBaseOp.pIoctl = 
		session_i2c_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].stBaseOp.pOpen =
		session_i2c_open;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].stBaseOp.pClose =
		session_i2c_close;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].stBaseOp.pRead = 
		session_i2c_read;
	g_astSessionTypeInfo[SESSION_TYPE_I2C].stBaseOp.pWrite =
		session_i2c_write;
	
	/*gpio*/
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].iSessionType =
		SESSION_TYPE_GPIO;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].iAsyn = 
		1;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].iSysPollSupport =
		1;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].iWorkMode = 
		SESSION_WORK_MODE_RDWR;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pIoctl = 
		session_gpio_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pOpen = 
		session_gpio_open;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pClose =
		session_gpio_close;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pLseek =
		session_gpio_lseek;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pRead =
		session_gpio_read;
	g_astSessionTypeInfo[SESSION_TYPE_GPIO].stBaseOp.pWrite =
		session_gpio_write;
	/*声音*/
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].iSessionType =
		SESSION_TYPE_SOUND;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].iAsyn = 
		0;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].iSysPollSupport =
		0;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].iWorkMode = 
		SESSION_WORK_MODE_WRONLY;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].stBaseOp.pIoctl = 
		session_sound_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].stBaseOp.pOpen = 
		session_sound_open;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].stBaseOp.pClose =
		session_sound_close;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].stBaseOp.pRead =
		session_sound_read;
	g_astSessionTypeInfo[SESSION_TYPE_SOUND].stBaseOp.pWrite =
		session_sound_write;
	/*socket类*/
	/*pipe*/
	g_astSessionTypeInfo[SESSION_TYPE_PIPE].iSessionType = 
		SESSION_TYPE_PIPE;
	/*tcp server*/
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].iSessionType = 
		SESSION_TYPE_TCP_SERVER;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].iAsyn = 
		1;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].iSysPollSupport =
		1;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].iWorkMode = 
		SESSION_WORK_MODE_RDWR;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].stBaseOp.pIoctl = 
		session_tcp_server_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].stBaseOp.pOpen = 
		session_tcp_server_open;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].stBaseOp.pClose =
		session_tcp_server_close;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].stBaseOp.pRead =
		session_tcp_server_read;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_SERVER].stBaseOp.pWrite =
		session_tcp_server_write;

	/*tcp client*/
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].iSessionType = 
		SESSION_TYPE_TCP_CLIENT;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].iAsyn = 
		1;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].iSysPollSupport =
		1;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].iWorkMode = 
		SESSION_WORK_MODE_RDWR;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].stBaseOp.pIoctl = 
		session_tcp_client_ioctl;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].stBaseOp.pOpen = 
		session_tcp_client_open;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].stBaseOp.pClose =
		session_tcp_client_close;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].stBaseOp.pRead =
		session_tcp_client_read;
	g_astSessionTypeInfo[SESSION_TYPE_TCP_CLIENT].stBaseOp.pWrite =
		session_tcp_client_write;

	/*udp*/
	g_astSessionTypeInfo[SESSION_TYPE_UDP].iSessionType = 
		SESSION_TYPE_UDP;
	
	/*应用协议类*/
	/*mqtt*/
	g_astSessionTypeInfo[SESSION_TYPE_MQTT].iSessionType =
		SESSION_TYPE_MQTT;
	/*COAP*/
	g_astSessionTypeInfo[SESSION_TYPE_COAP].iSessionType = 
		SESSION_TYPE_COAP;
	
	/*接口模拟类*/
	g_astSessionTypeInfo[SESSION_TYPE_SOFTWARE_INTERFACE].iSessionType =
		SESSION_TYPE_SOFTWARE_INTERFACE;
	
	return NoErr;
}

/*会话配置信息比较回调*/
int _session_config_cmp_handle(
	void *pData1, int iData1Len, 
	void *pData2, int iData2Len
)
{
	session_conf_t *pstSessionInfo1 = (session_conf_t *)pData1;
	session_conf_t *pstSessionInfo2 = (session_conf_t *)pData2;
	
	return pstSessionInfo1->iSessionId - \
		pstSessionInfo2->iSessionId;
}

int _session_config_print_handle(
	void *pData, int iDataLen)
{
	session_conf_t *pstSessionInfo = (session_conf_t *)pData;

	HY_INFO("%s:%d\n", pstSessionInfo->acSessionAddr, pstSessionInfo->iSessionId);

	return 0;
	
}

/*重发器的重发回调*/
static int 
_session_data_repeater_resend_cb_handle(
	data_repeater_t *_this,
	void *pData, 
	int iDataLen,
	void *pUserData)
{
	session_data_t *pstSessionData = 
		(session_data_t *)pData;
	int iRet = 0;

	iRet = ((WriteFun)(
				g_astSessionTypeInfo[pstSessionData->iSessionType].stBaseOp.pWrite
			)
		)
	(
		pstSessionData->iSessionId,
		pstSessionData->aucData,
		pstSessionData->iDataLen
	);
	return iRet;
}

/*重发器的数据域释放回调*/
static int 
_session_data_repeater_free_data_cb_handle(
	data_repeater_t *_this,
	void *pData, 
	int iDataLen
)
{
	session_data_t *pstSessionData = (session_data_t *)pData;
	if(pstSessionData->pProcessedData)
	{
		base_free(pstSessionData->pProcessedData);
	}

	return NoErr;
}

/*查找会话*/
session_conf_t* _session_find (
	session_ser_class_t* _this, 
	int iSessionId)
{
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)_this->acPrivateParam;
	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(acHashKey, HASH_KEY_MAX_LEN, "%d", iSessionId);
	session_conf_t *pstSession = NULL;
	void *ptr = pPrivate->pstSessionHash->find(
			pPrivate->pstSessionHash,
			acHashKey
		);
	
	if(NULL == ptr)
	{
		HY_ERROR("The SessionId not found.\n");
		return NULL;
	}
	else
	{
		base_memcpy(&pstSession, ptr, sizeof(session_conf_t *));
		return pstSession;
	}
}

	
/*会话被动保活*/
void _session_passive_keepalive(
	session_conf_t *pstSession)
{
#if 0
		int iSessionTmp = pstSession->iSessionId;
		/*删除epoll事件监控*/
		if(1 == g_astSessionTypeInfo[pstSession->iSessionType].iSysPollSupport || 
			SESSION_WORK_MODE_WRONLY != \
			g_astSessionTypeInfo[pstSession->iSessionType].iWorkMode)
		{
			pPrivate->pstEpollFd->del_event(
				pPrivate->pstEpollFd,
				pstSession->iSessionId
			);
		}
		
		/*关闭原会话*/
		((CloseFun)(
				g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pClose
			)
		)
		(
			pstSession->iSessionId
		);
		
		/*重新打开会话*/
		pstSession->iSessionId = 
			((OpenFun)(
					g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pOpen
				)
			)
			(
				pstSession->acSessionAddr,
				g_astSessionTypeInfo[pstSession->iSessionType].iWorkMode
			);
		if(pstSession->iSessionId < 0)
		{
			HY_ERROR("Session(%s) reopen failed.\n", 
				pstSession->acSessionAddr);
			continue;
		}
		/*将会话添加到epoll中监控IO事件*/
		if(1 == g_astSessionTypeInfo[pstSession->iSessionType].iSysPollSupport || 
			SESSION_WORK_MODE_WRONLY != \
			g_astSessionTypeInfo[pstSession->iSessionType].iWorkMode)
		{
			if(0 != 
				pPrivate->pstEpollFd->init_event(
					pPrivate->pstEpollFd, 
					pstSession->iSessionId
				)
			)
			{
				HY_ERROR("Init event failed\n");
				continue;
			}
		}
		if(iSessionTmp != pstSession->iSessionId)
		{
			char acHashKey[HASH_KEY_MAX_LEN] = {0};
			base_snprintf(
				acHashKey, 
				HASH_KEY_MAX_LEN,
				"%d", 
				iSessionTmp
			);
			pPrivate->pstSessionHash->del(
				pPrivate->pstSessionHash,
				acHashKey
			);

			base_snprintf(
				acHashKey, 
				HASH_KEY_MAX_LEN, 
				"%d", 
				pstSession->iSessionId
			);
			pPrivate->pstSessionHash->inst(
				pPrivate->pstSessionHash,
				acHashKey,
				pstSession,
				sizeof(session_conf_t*)
			);
		}
#endif
}

/*会话主动保活*/
void _session_initiative_keepalive(
	session_conf_t *pstSession)
{
	
}

/*从发送队列中取出数据并发送*/
void _session_send_data(session_ser_class_t *pstSessionSer)
{
	
}

/*新客户端连接处理*/
int _session_client_connect(
	session_ser_class_t *pstSessionSer,
	session_conf_t *pstServerSession,
	char *pServerReturnJson)
{
	int iRet = 0;
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pstSessionSer->acPrivateParam;

	char acAddr[SESSION_ADDR_MAX_LEN] = {0};
	char acSessionId[SESSION_NUM_MAX_LEN] = {0};
	int iSessionId = {0};
	
	cJSON *pstJson = NULL;
	int iValueType;
	pstJson = cJSON_Parse(pServerReturnJson);
	if(NULL == pstJson)
	{
		HY_ERROR("The json format error.\n");
		iRet = -1;
		goto _session_client_connect_end;
	}

	if(0 != JSON_value_get(
		SESSION_JSON_KEY_ADDR, 
		acAddr, 
		SESSION_ADDR_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		HY_ERROR("The json format error(%s not found).\n",
			SESSION_JSON_KEY_ADDR);
		iRet = -1;
		goto _session_client_connect_end;
	}

	if(0 != JSON_value_get(
		SESSION_JSON_KEY_ID, 
		acSessionId, 
		SESSION_NUM_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		HY_ERROR("The json format error(%s not found).\n",
			SESSION_JSON_KEY_ID);
		iRet = -1;
		goto _session_client_connect_end;
	}
	iSessionId = base_atoi(acSessionId);

	/*创建客户端会话*/
	session_conf_t stSession = {0};
	stSession.iSessionId = iSessionId;
	stSession.iSessionType = SESSION_TYPE_TCP_CLIENT;
	base_snprintf(stSession.acSessionAddr, 
		SESSION_ADDR_MAX_LEN, "%s", acAddr);
	stSession.iDataTimeOut = 200;
	stSession.iMaxResendTime = 3;
	stSession.iDataSpacing = 10;
	stSession.pParentSession = (void *)pstServerSession;
	stSession.iActiveFlag = 1;
	
	/*初始化重发器*/
	stSession.pstDataRepeater = 
		new_data_repeater(
			stSession.iDataTimeOut,
			stSession.iMaxResendTime
		);
	if(NULL != stSession.pstDataRepeater)
	{
		stSession.pstDataRepeater->resendCbReg(
			stSession.pstDataRepeater,
			_session_data_repeater_resend_cb_handle,
			NULL
		);
		stSession.pstDataRepeater->freeDataCbReg(
			stSession.pstDataRepeater,
			_session_data_repeater_free_data_cb_handle
		);
	}

	/*将会话添加到epoll中监控IO事件*/
	if(0 != pPrivate->pstEpollFd->init_event(
			pPrivate->pstEpollFd,
			iSessionId
		)
	)
	{
		HY_ERROR("Init event failed\n");
		iRet = -1;
		goto _session_client_connect_end;
	}

	/*将客户端会话ID，添加到会话列表中*/
	/*将会话信息，添加到链表中*/
	if(NULL == pstServerSession->pstClientList)
	{
		pstServerSession->pstClientList = 
			new_double_link_list();
		if(NULL == pstServerSession->pstClientList)
		{
			HY_ERROR("List creation failed.\n");
			pPrivate->pstEpollFd->del_event(
				pPrivate->pstEpollFd,
				iSessionId);
			iRet = -1;
			goto _session_client_connect_end;
		}
	}
	
	iRet = pstServerSession->pstClientList->tailInsert(
		pstServerSession->pstClientList,
		(void *)&stSession, 
		sizeof(session_conf_t)
	);
	if(iRet < 0)
	{
		HY_ERROR("Session info add list failed.\n");
		pPrivate->pstEpollFd->del_event(
			pPrivate->pstEpollFd,
			iSessionId);
		iRet = -1;
		goto _session_client_connect_end;
	}
	/*将会话信息指针，添加到hash表中*/
	session_conf_t stFindSession = {0};
	stFindSession.iSessionId = iSessionId;
	
	session_conf_t *pstFindSession = (session_conf_t *)(
		pstServerSession->pstClientList->find(
			pstServerSession->pstClientList,
			(void *)_session_config_cmp_handle, 
			(void *) &stFindSession, 
			sizeof(session_conf_t)
		)
	);
	if(NULL == pstFindSession)
	{
		HY_ERROR("The SessionId not found.\n");
		iRet = -1;
		pPrivate->pstEpollFd->del_event(
			pPrivate->pstEpollFd,
			iSessionId);
		goto _session_client_connect_end;
	}
	
	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(
		acHashKey,
		HASH_KEY_MAX_LEN,
		"%d", 
		iSessionId
	);
	iRet = pPrivate->pstSessionHash->inst(
		pPrivate->pstSessionHash,
		acHashKey, 
		(void *)&pstFindSession,
		sizeof(session_conf_t *)
	);
	if(0 != iRet)
	{
		HY_ERROR("Session join hash failed.\n");
		iRet = -1;
		pPrivate->pstEpollFd->del_event(
			pPrivate->pstEpollFd,
			iSessionId);
		pstServerSession->pstClientList->del(
			pstServerSession->pstClientList,
			pstFindSession);
		goto _session_client_connect_end;
	}
	
_session_client_connect_end:
	if(iRet < 0)
	{
		base_close(iSessionId);
		if(NULL != stSession.pstDataRepeater)
		{
			destroy_data_repeater(stSession.pstDataRepeater);
		}
	}
	return 0;
}

/*上报数据*/
void _session_report_original_data(
	session_ser_class_t *pstSessionSer,
	session_conf_t *pstSession,
	unsigned char *pData,
	int iDataLen)
{
	base_timeval_t stNew;
	/*获取当前时间*/
	base_time_get(&stNew);
	
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pstSessionSer->acPrivateParam;
	
	/*向服务器端推送新客户端连接信息*/
	session_data_t stSessionRecvData = {0};
	stSessionRecvData.iSessionId = 
		pstSession->iSessionId;
	stSessionRecvData.iSessionType =
		pstSession->iSessionType;
	base_memcpy(&stSessionRecvData.stTimestamp,
		&stNew, sizeof(base_timeval_t));
	stSessionRecvData.iDataLen = 
		iDataLen;
	base_memcpy(stSessionRecvData.aucData,
		pData, 
		iDataLen);
	if(pstSession->stCallbackList.pDataRecvCb)
	{
		/*定义接收数据回调，则从回调返回数据*/
		((dataRecvFun)(
				pstSession->stCallbackList.pDataRecvCb
			)
		)
		(
			pstSessionSer,
			(void*)&stSessionRecvData, 
			sizeof(session_data_t),
			pstSession->stCallbackList.pDataRecvUserData
		);
	}
	else
	{
		/*未定义回调，则将数据加入到接收缓存*/
		pPrivate->pstRecvMsgQueue->push(
			pPrivate->pstRecvMsgQueue,
			pstSession->iSessionId,
			&stSessionRecvData,
			sizeof(session_data_t)
		);
	}

	return;
}

void __session_report_session_data(
	session_ser_class_t *pstSessionSer,
	session_conf_t *pstSession,
	session_data_t *pstData)
{
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pstSessionSer->acPrivateParam;
	
	if(pstSession->stCallbackList.pDataRecvCb)
	{
		/*注册了数据回调，则回应从回调直接返回*/
		((dataRecvFun)(
				pstSession->stCallbackList.pDataRecvCb
			)
		)
		(
			pstSessionSer,
			(void*)pstData, 
			sizeof(session_data_t),
			pstSession->stCallbackList.pDataRecvUserData
		);
	}
	else
	{
		/*未注册接收回调，将该回应报文添加到异步接收队列*/
		pPrivate->pstRecvMsgQueue->push(
			pPrivate->pstRecvMsgQueue,
			pstSession->iSessionId,
			pstData,
			sizeof(session_data_t)
		);
	}
}
	
/*上报数据,经过协议回调处理*/
void _session_report_protocol_data(
	session_ser_class_t *pstSessionSer,
	session_conf_t *pstSession,
	unsigned char *pData,
	int iDataLen)
{
	int iRet = 0;
	base_timeval_t stNew;
	/*获取当前时间*/
	base_time_get(&stNew);
	
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pstSessionSer->acPrivateParam;
	
	int iRemainingDataLen = iDataLen;
	unsigned char *pucRecvOriginalDataCurr = 
		pData;
	unsigned char uacRecvData[SESSION_DATA_MAX_LEN] = {0};
	int iRecvDataLen = SESSION_DATA_MAX_LEN;
	while(
		iRemainingDataLen > 0
	)
	{
		/*原始数据的分割组装*/
		iRemainingDataLen	= 
			((dataSplitCombineFun)(
					pstSession->stCallbackList.pDataSplitCombineCb
				)
			)
			(
				pstSessionSer,
				pucRecvOriginalDataCurr,
				iRemainingDataLen,
				uacRecvData,
				&iRecvDataLen,
				pstSession->stCallbackList.pDataSplitCombineUserData
			);

		if(iRecvDataLen > 0)
		{
			HY_INFO("Recv Len : %d\n", iRecvDataLen);
			LOG_HEX (LOG_INFO, 
				uacRecvData, 
				0, 
				iRecvDataLen
			);
			
			/*分割组包后的数据*/
			session_data_t stSessionRecvData = {0};
			stSessionRecvData.iSessionId = 
				pstSession->iSessionId;
			stSessionRecvData.iSessionType =
				pstSession->iSessionType;
			base_memcpy(&stSessionRecvData.stTimestamp,
				&stNew, sizeof(base_timeval_t));
			stSessionRecvData.iDataLen = 
				iRecvDataLen;
			base_memcpy(stSessionRecvData.aucData,
				uacRecvData, 
				iRecvDataLen);
			/*根据协议解析数据*/
			if(NULL != pstSession->stCallbackList.pDataParserCb)
			{
				iRet = 
					((dataParserFun)(
						pstSession->stCallbackList.pDataParserCb
					)
				)
				(
					pstSessionSer,
					(void*)&stSessionRecvData, 
					sizeof(session_data_t),
					pstSession->stCallbackList.pDataParserUserData
				);

				if(iRet < 0)
				{
					/*无效数据，直接丢弃，继续处理下面的数据*/
					HY_ERROR("invalid data.\n");
					iRecvDataLen = SESSION_DATA_MAX_LEN;
					pucRecvOriginalDataCurr = pData +
						(iDataLen - iRemainingDataLen);
					return;
				}
			}
			
			if(NULL != pstSession->pstDataRepeater &&
				pstSession->stCallbackList.pRespondMatchingCb)
			{
				/*根据接收数据，匹配发送数据，
				*以便确定该接收报文是异步请求的回应、
				*还是同步请求的回应，或者是主动上报*/
				session_data_t stSessionSendData = {0};
				int iSessionSendDataLen = 
					sizeof(session_data_t);
				iRet = 
					pstSession->pstDataRepeater->respondPush(
						pstSession->pstDataRepeater,
						(void*)&stSessionRecvData,
						sizeof(session_data_t),
						(void*)&stSessionSendData,
						&iSessionSendDataLen
					);
				if(0 == iRet)
				{
					/*匹配到发送报文*/
					if(0 == stSessionSendData.iSync)
					{
						/*异步报文回应*/
						__session_report_session_data(
							pstSessionSer,
							pstSession,
							&stSessionRecvData);
					}
					else if(1 == stSessionSendData.iSync)
					{
						/*同步报文回应，
						将该回应报文添加到同步接收队列*/
						pPrivate->pstSyncRecvMsgQueue->push(
							pPrivate->pstSyncRecvMsgQueue,
							pstSession->iSessionId,
							&stSessionRecvData,
							sizeof(session_data_t)
						);
					}
					/*释放匹配到的发送数据的pProcessedData域*/
					if(stSessionSendData.pProcessedData)
					{
						base_free(
							stSessionSendData.pProcessedData
						);
					}
				}
				else
				{
					/*未匹配到发送报文，此时认为该报文为主动上报*/
					__session_report_session_data(
						pstSessionSer,
						pstSession,
						&stSessionRecvData);
				}
			}
			else
			{
				/*异步报文回应*/
				__session_report_session_data(
					pstSessionSer,
					pstSession,
					&stSessionRecvData);
			}
			
		}
		iRecvDataLen = SESSION_DATA_MAX_LEN;
		pucRecvOriginalDataCurr = pData +
			(iDataLen - iRemainingDataLen);
	}
}


/**************************************************************
*描述: 创建会话
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @void* pSessionInfo: 会话属性
*返回: 成功返回SessionId，失败返回-1；
**************************************************************/
int session_open ( session_ser_class_t* _this, 
	session_info_t* pSessionInfo)
{
	if(NULL == _this || 
		NULL == pSessionInfo)
	{
		error_num = ParaErr;
		return ParaErr;
	}


	int iRet = 0;

	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)_this->acPrivateParam;
		
	session_conf_t stSession = {0};

	stSession.iSessionType = pSessionInfo->iSessionType;
	base_strncpy(stSession.acSessionAddr, 
		pSessionInfo->acSessionAddr, SESSION_ADDR_MAX_LEN);
	stSession.iDataTimeOut = pSessionInfo->iDataTimeOut;
	stSession.iMaxResendTime = pSessionInfo->iMaxResendTime;
	stSession.iDataSpacing = pSessionInfo->iDataSpacing;
	
	/*初始化重发器,只有支持异步的会话类型才需要*/
	if(g_astSessionTypeInfo[stSession.iSessionType].iAsyn)
	{
		stSession.pstDataRepeater = 
			new_data_repeater(
				stSession.iDataTimeOut,
				stSession.iMaxResendTime
			);
		if(NULL != stSession.pstDataRepeater)
		{
			stSession.pstDataRepeater->resendCbReg(
				stSession.pstDataRepeater,
				_session_data_repeater_resend_cb_handle,
				NULL
			);
			stSession.pstDataRepeater->freeDataCbReg(
				stSession.pstDataRepeater,
				_session_data_repeater_free_data_cb_handle
			);
		}
	}
	
	/*开启会话*/
	iRet = ((OpenFun)(
					g_astSessionTypeInfo[stSession.iSessionType].stBaseOp.pOpen
				)
			)
			(
				stSession.acSessionAddr,
				g_astSessionTypeInfo[stSession.iSessionType].iWorkMode
			);
	if(iRet < 0)
	{
		HY_ERROR("Session(%s) open failed.\n", stSession.acSessionAddr);
		error_num = SessionOpenErr;
		iRet =  GeneralErr;
		goto session_open_end;
	}

	stSession.iSessionId = pSessionInfo->iSessionId = iRet;
	stSession.iActiveFlag = 1;
	/*将会话添加到epoll中监控IO事件*/
	if(1 == g_astSessionTypeInfo[stSession.iSessionType].iSysPollSupport && 
		SESSION_WORK_MODE_WRONLY != \
		g_astSessionTypeInfo[stSession.iSessionType].iWorkMode)
	{
		if(0 != pPrivate->pstEpollFd->init_event(
				pPrivate->pstEpollFd,
				stSession.iSessionId
			)
		)
		{
			HY_ERROR("Init event failed\n");
			iRet =  GeneralErr;
			goto session_open_end;
		}
	}
		
	/*将会话信息，添加到链表中*/
	iRet = _this->stLinkList.tailInsert(
		(double_link_list_class_t*)_this,
		(void *)&stSession, 
		sizeof(session_conf_t)
	);
	if(iRet < 0)
	{
		HY_ERROR("Session info all list failed.\n");
		iRet =  GeneralErr;
		goto session_open_end;
	}

	/*将会话信息指针，添加到hash表中*/
	session_conf_t stFindSession = {0};
	stFindSession.iSessionId = pSessionInfo->iSessionId;
	
	session_conf_t *pstSession = (session_conf_t *)(
		_this->stLinkList.find(
			(double_link_list_class_t *)_this,
			(void *)_session_config_cmp_handle, 
			(void *) &stFindSession, 
			sizeof(session_conf_t)
		)
	);
	if(NULL == pstSession)
	{
		HY_ERROR("The SessionId not found.\n");
		iRet =  GeneralErr;
		goto session_open_end;
	}
	
	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(
		acHashKey,
		HASH_KEY_MAX_LEN,
		"%d", 
		pSessionInfo->iSessionId
	);
	iRet = pPrivate->pstSessionHash->inst(
		pPrivate->pstSessionHash,
		acHashKey, 
		(void *)&pstSession,
		sizeof(session_conf_t *)
	);
	if(0 != iRet)
	{
		HY_ERROR("Session join hash failed.\n");
		iRet =  GeneralErr;
		goto session_open_end;
	}
	
session_open_end:
	if(iRet < 0)
	{
		if(NULL != stSession.pstDataRepeater)
		{
			destroy_data_repeater(stSession.pstDataRepeater);
		}
		if(stSession.iSessionId > 0)
		{
			if(1 == g_astSessionTypeInfo[stSession.iSessionType].iSysPollSupport || 
				SESSION_WORK_MODE_WRONLY != \
					g_astSessionTypeInfo[stSession.iSessionType].iWorkMode)
			{
				pPrivate->pstEpollFd->del_event(
					pPrivate->pstEpollFd,
					stSession.iSessionId
				);
			}
			((CloseFun)(
					g_astSessionTypeInfo[stSession.iSessionType].stBaseOp.pClose
				)
			)
			(
				stSession.iSessionId
			);
		}
		return iRet;
	}
	else
	{
		
		return stSession.iSessionId;
	}
}

/**************************************************************
*描述: 关闭会话
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @int iSessionId: 会话ID
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_close ( session_ser_class_t* _this, 
	int iSessionId)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	if(iSessionId <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)_this->acPrivateParam;
	
	/*查找会话*/
	session_conf_t *pstSession = 
		_session_find (_this, iSessionId);
	if(NULL == pstSession)
	{
		HY_ERROR("The SessionId not found.\n");
		return GeneralErr;
	}

	/*如果是客户端连接，则需要告知服务会话*/
	if(NULL != pstSession->pParentSession)
	{
		cJSON *pstJson = NULL;
		unsigned char uacRecvData[SESSION_DATA_MAX_LEN] = {0};
		int iRecvDataLen = SESSION_DATA_MAX_LEN;
		char acSessionId[SESSION_NUM_MAX_LEN] = {0};
		pstJson = cJSON_CreateObject();
		if(NULL == pstJson)
		{
			HY_ERROR("Failed to create json object.\n");
			return GeneralErr;
		}

		cJSON_AddStringToObject(pstJson, 
			SESSION_JSON_KEY_COMMAND, 
			SESSION_JSON_COMMAND_VALUE_DISCONNECT
		);

		cJSON_AddStringToObject(pstJson, 
			SESSION_JSON_KEY_ADDR, 
			pstSession->acSessionAddr
		);
		
		base_snprintf(acSessionId, 
			SESSION_NUM_MAX_LEN, 
			"%d",
			pstSession->iSessionId);
		
		cJSON_AddStringToObject(pstJson, 
			SESSION_JSON_KEY_ID, 
			acSessionId
		);
		
		char *pstr = cJSON_Print(pstJson);
		base_strncpy((char *)uacRecvData, pstr, base_strlen(pstr) + 1);
		base_free(pstr);
		
		cJSON_Minify((char *)uacRecvData);
		cJSON_Delete(pstJson);

		iRecvDataLen = (base_strlen((char *)uacRecvData) + 1);


		/*向服务器端推送客户端连接断开*/
		_session_report_original_data(
			_this,
			(session_conf_t *)pstSession->pParentSession,
			uacRecvData,
			iRecvDataLen);
	}
	
	/*销毁重发器*/
	if(NULL != pstSession->pstDataRepeater)
	{
		destroy_data_repeater(pstSession->pstDataRepeater);
	}

	/*删除epoll事件监控*/
	if(1 == g_astSessionTypeInfo[pstSession->iSessionType].iSysPollSupport || 
		SESSION_WORK_MODE_WRONLY != \
		g_astSessionTypeInfo[pstSession->iSessionType].iWorkMode)
	{
		pPrivate->pstEpollFd->del_event(
			pPrivate->pstEpollFd,
			pstSession->iSessionId
		);
	}

	/*关闭客户端会话*/
	if(NULL != pstSession->pstClientList)
	{
		void *pSceeionInfo = NULL;
		session_conf_t *pstClientSession = NULL;
		while(NULL != (pSceeionInfo = 
				pstSession->pstClientList->next(
					pstSession->pstClientList,
					pSceeionInfo
				)
			)
		)
		{
			pstClientSession = (session_conf_t*)pSceeionInfo;
			/*关闭客户端会话*/
			session_close (_this, 
				pstClientSession->iSessionId);
		}

		/*销毁客户端会话列表*/
		destroy_double_link_list(pstSession->pstClientList);
	}
		
	/*关闭会话*/
	((CloseFun)(
			g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pClose
		)
	)
	(
		pstSession->iSessionId
	);
	pstSession->iActiveFlag = 0;

	/*删除hash*/
	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(acHashKey, HASH_KEY_MAX_LEN, "%d", iSessionId);
	pPrivate->pstSessionHash->del(
		pPrivate->pstSessionHash, acHashKey);
	
	/*删除会话*/
	if(NULL != pstSession->pParentSession)
	{
		/*该会话从属与某一个服务会话*/
		/*该会话信息存储在服务会话的客户端列表中*/
		session_conf_t *pstParentSession = 
			(session_conf_t *)pstSession->pParentSession;
		pstParentSession->pstClientList->del(
			pstParentSession->pstClientList,
			(void *)pstSession);
	}
	else
	{
		/*该会话为独立的会话*/
		_this->stLinkList.del(
			(double_link_list_class_t *)_this,
			(void *)pstSession
		);
	}

	return NoErr;
}

/**************************************************************
*描述: 会话控制
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @int iSessionId: 会话ID
*	   @int iCmd: 控制指令，详见session_ioctl_cmd_t
*	   @unsigned long ulArg: 参数
*返回: 成功返回0，失败返回-1, 具体错误码查看error_num；
**************************************************************/
int session_ioctl ( session_ser_class_t* _this, 
	int iSessionId,
	int iCmd, 
	unsigned long ulArg)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	if(iCmd < 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;

	/*查找会话*/
	session_conf_t *pstSession = 
		_session_find (_this, iSessionId);
	if(NULL == pstSession)
	{
		HY_ERROR("The SessionId not found.\n");
		return GeneralErr;
	}
	
	iRet = ((IoctlFun)(
			g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pIoctl
		)
	)
	(
		iSessionId,
		iCmd, 
		ulArg
	);
	
	return iRet;
}

/**************************************************************
*描述: 定位数据读取位置
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @int iOffset: 位移量
*	   @int iWhence: 起始位置
*返回: 成功返回SessionId，失败返回-1；
**************************************************************/
int session_lseek ( 
	session_ser_class_t* _this, 
	int iSessionId,
	int iOffset, 
	int iWhence)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	if(iOffset <= 0 ||
		iWhence <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}
		
	int iRet = 0;
	/*查找会话*/
	session_conf_t *pstSession = 
		_session_find (_this, iSessionId);
	if(NULL == pstSession)
	{
		HY_ERROR("The SessionId not found.\n");
		return GeneralErr;
	}
	
	iRet = ((LseekFun)(
			g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pLseek
		)
	)
	(
		iSessionId,
		iOffset, 
		iWhence
	);

	return iRet;
}	
/**************************************************************
*描述: 向指定会话发送数据
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @int iSessionId: 会话ID
*	   @void* pSendData: 将要发送的数据
*	   @void* pRecvData: 将要同步接收的数据, 如果异步接收则设为NULL
*	   @int iTimeOut: 同步接收超时
*
*返回: 成功返回已发送的数据长度，失败返回-1；
**************************************************************/
int session_send ( session_ser_class_t* _this, 
	session_data_t* pSendData, 
	session_data_t* pRecvData, 
	int iTimeOut)
{
	if(NULL == _this ||
		NULL == pSendData)
	{
		error_num = ParaErr;
		return GeneralErr;
	}
	if(NULL != pRecvData && iTimeOut < 0)
	{
		error_num = ParaErr;
		return GeneralErr;
	}
	int iRet = 0;

	if(0 == g_astSessionTypeInfo[pSendData->iSessionType].iAsyn)
	{
		/*同步会话*/
		/*发送*/
		int iSendDataLen = 
			((WriteFun)(
					g_astSessionTypeInfo[pSendData->iSessionType].stBaseOp.pWrite
				)
			)
			(
				pSendData->iSessionId, 
				pSendData->aucData, 
				pSendData->iDataLen
			);
		if(iSendDataLen != pSendData->iDataLen)
		{
			error_num = SessionSendErr;
			return GeneralErr;
		}
		/*记录发送时间*/
		base_time_get(&pSendData->stTimestamp);
		
		/*等待同步返回*/
		if(1 == pSendData->iSync && NULL != pRecvData)
		{
			int iRecvDataLen = 0;
			base_timeval_t stNow;
			base_timeval_t stEnd;
	
			base_time_get(&stNow);
			stEnd.uiUsec = stNow.uiUsec;
			stEnd.uiMsec = (stNow.uiMsec + iTimeOut) % 1000;
			stEnd.uiSec = stNow.uiSec + (stNow.uiMsec + iTimeOut) / 1000;
			while((stNow.uiUsec < stEnd.uiUsec) ||
				(stNow.uiUsec == stEnd.uiUsec && 
					stNow.uiMsec <= stEnd.uiMsec
				)
			)
			{
				iRecvDataLen = 
					((ReadFun)(
							g_astSessionTypeInfo[pSendData->iSessionType].stBaseOp.pRead
						)
					)
					(
						pSendData->iSessionId, 
						pRecvData->aucData, 
						SESSION_DATA_MAX_LEN
					);
				if(iRecvDataLen > 0)
				{
					pRecvData->iDataLen = iRecvDataLen;
					/*记录接收时间*/
					base_time_get(&pRecvData->stTimestamp);
					break;
				}

				base_delay_ms(100);
			}
		}
	}
	else
	{
		/*异步会话*/
	
		int iPushCount = 0;
		session_ser_private_t *pPrivate = 
			(session_ser_private_t *)_this->acPrivateParam;
		/*加入发送队列*/
		while(iPushCount < SESSION_PUSH_MAX_NUM)
		{
			iRet = pPrivate->pstSendMsgQueue->push(
				pPrivate->pstSendMsgQueue,
				pSendData->iSessionId, 
				(void *)pSendData,
				sizeof(session_data_t)
			);
			if(iRet < 0)
			{
				/*发送队列添加失败，50ms后重试*/
				iPushCount ++;
				base_delay_ms(50);
				continue;
			}
			else
			{
				break;
			}
		}
		if(iRet < 0)
		{
			HY_ERROR("Send session data error.\n");
			error_num = SessionSendErr;
			return GeneralErr;
		}
		

		/*等待同步返回*/
		if(1 == pSendData->iSync && NULL != pRecvData)
		{
			int iLen = sizeof(session_data_t);
			iRet = pPrivate->pstSyncRecvMsgQueue->pop(
				pPrivate->pstSyncRecvMsgQueue,
				&pSendData->iSessionId,
				iTimeOut, 
				(void *)pRecvData,
				&iLen
			);
			if(iRet < 0)
			{
				HY_ERROR("Recv session sync data error.\n");
				error_num = SessionRecvErr;
				return GeneralErr;
			}
		}
	}
	

	return pSendData->iDataLen;
}

/**************************************************************
*描述: 接收指定会话的数据
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @void* pData: 用于存放接收的数据
*	   @int* piDataLen: 传入接收缓存的最大长度，传出真实接收数据的长度
*	   @int iTimeOut: 0表示,如果没有数据则立即返回; -1表示, 如果没有数据则阻塞，直到收到数据;
*		大于0, 如果没有数据，则最多等待iTimeOut，单位ms。
*返回: 成功返回接收数据长度，失败返回-1；
**************************************************************/
int session_recv ( session_ser_class_t* _this, 
	session_data_t* pData,
	int iTimeOut)
{
	if(NULL == _this ||
		NULL == pData)
	{
		error_num = ParaErr;
		return ParaErr;
	}
		
	int iRet = 0;
	int iLen = sizeof(session_data_t);
	
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)_this->acPrivateParam;
	
	iRet = pPrivate->pstRecvMsgQueue->pop(
		pPrivate->pstRecvMsgQueue,
		&pData->iSessionId,
		iTimeOut, 
		(void *)pData,
		&iLen
	);
	if(iRet < 0)
	{
		/*查找会话*/
		session_conf_t *pstSession = 
		_session_find(_this, pData->iSessionId);
		if(NULL == pstSession)
		{
			HY_ERROR("The SessionId not found.\n");
			return GeneralErr;
		}
		
		if(0 == g_astSessionTypeInfo[pstSession->iSessionType].iAsyn)
		{
			base_timeval_t stNow;
			base_timeval_t stEnd;
	
			base_time_get(&stNow);
			stEnd.uiUsec = stNow.uiUsec;
			stEnd.uiMsec = (stNow.uiMsec + iTimeOut) % 1000;
			stEnd.uiSec = stNow.uiSec + (stNow.uiMsec + iTimeOut) / 1000;
			
			do{
				pData->iDataLen = 
					((ReadFun)(
							g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pRead
						)
					)
					(
						pstSession->iSessionId, 
						pData->aucData, 
						SESSION_DATA_MAX_LEN
					);
				if(pData->iDataLen > 0)
				{
					break;
				}
				
				base_delay_ms(100);
			}while((stNow.uiUsec < stEnd.uiUsec) ||
				(stNow.uiUsec == stEnd.uiUsec && stNow.uiMsec < stEnd.uiMsec)
			);
			if(pData->iDataLen >= 0)
			{
				error_num = TimeOutErr;
				return GeneralErr;
			}
		}
		else
		{
			return GeneralErr;
		}
	}

	return pData->iDataLen;
}
	/**************************************************************
*描述: 回调函数注册
*参数: @struct session_ser_class_s* _this: 会话服务类指针
*	   @int iSessionId: 会话ID
*	   @int iCbType: 回调类型
*	   @void* pFun: 回调函数指针
*	   @void* pUserData: 用户数据
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_callback_reg(struct session_ser_class_s* _this, 
	int iSessionId,
	int iCbType,
	void* pFun,
	void* pUserData)
{
	if(NULL == _this ||
		NULL == pFun)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	/*查找会话*/
	session_conf_t *pstSession = 
		_session_find (_this, iSessionId);
	if(NULL == pstSession)
	{
		HY_ERROR("The SessionId not found.\n");
		return GeneralErr;
	}
	
	switch(iCbType)
	{
		/*接收数据包黏连组合回调*/
		case SESSION_CB_TYPE_DATA_SPLITCOMBINE:
			{
				pstSession->stCallbackList.pDataSplitCombineCb = pFun;
				pstSession->stCallbackList.pDataSplitCombineUserData =
					pUserData;
			}
			break;
		/*接收数据解析回调*/
		case SESSION_CB_TYPE_DATA_PARSER:
			{
				pstSession->stCallbackList.pDataParserCb = pFun;
				pstSession->stCallbackList.pDataParserUserData = 
					pUserData;
			}
			break;
		/*接收数据与发送数据 匹配回调*/
		case SESSION_CB_TYPE_RESP_MATCHING:
			{
				if(NULL != pstSession->pstDataRepeater)
				{
					pstSession->stCallbackList.pRespondMatchingCb = pFun;
					pstSession->stCallbackList.pRespondMatchingUserData = 
						pUserData;
				
					pstSession->pstDataRepeater->respondMatchingCbReg(
						pstSession->pstDataRepeater,
						pFun,
						pUserData);
				}
				else
				{
					HY_ERROR("The Repeater is not initialized.\n");
				}
			}
			break;
		/*数据接收回调*/
		case SESSION_CB_TYPE_DATA_RECV:
			{
				pstSession->stCallbackList.pDataRecvCb = pFun;
				pstSession->stCallbackList.pDataRecvUserData = 
					pUserData;
			}
			break;

	default:
			HY_ERROR("Session type unknown\n");
			break;
	}
	return NoErr;
}
	
static void*
session_sys_exec(void *arg)
{
	int iRet = 0;
	int i = 0;
	base_timeval_t stNew;
	
	int iRecvOriginalDataLen = 0;
	unsigned char aucRecvOriginalData[SESSION_DATA_MAX_LEN] = {0};
	void *pSceeionInfo = NULL;
	void *pClinetSceeionInfo = NULL;
	session_conf_t *pstSession = NULL;
	session_conf_t *pstClientSession = NULL;
	
	session_ser_class_t *pstSessionSer =
		(session_ser_class_t *)arg;

	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pstSessionSer->acPrivateParam;

	epoll_fd_t * pstFd = pPrivate->pstEpollFd;
	int iEventNum = 0;
	epoll_event_t astEpollEven[SESSION_MAX_NUM] = {{0}};
	
	/*开启线程退出功能*/
	base_thread_set_cancel();
	
	while(1)
	{
		/*线程取消点*/
		base_thread_cancel_point();
		/*获取当前时间*/
		base_time_get(&stNew);

		/*发送数据*/
		/*从发送队列中获取数据，并发送*/
		int iMsgType = -1;
		session_data_t stSessionSendData = {0};
		int iSendDataLen = 0;
		int iSessionDataLen = sizeof(session_data_t);
		
		while(0 == pPrivate->pstSendMsgQueue->pop(
			pPrivate->pstSendMsgQueue,
			&iMsgType, 
			0, 
			(void *)&stSessionSendData,
			&iSessionDataLen)
		)
		{
			/*查找会话*/
			pstSession = 
				_session_find(pstSessionSer, 
				stSessionSendData.iSessionId);
			if(NULL == pstSession)
			{
				HY_ERROR("The SessionId not found.\n");
				continue;
			}
			
			/*最小发送间隔检测*/
			unsigned int uiSec  = 
				pstSession->iDataSpacing / 1000;
			unsigned int uiMsec = 
				pstSession->iDataSpacing % 1000; 
			while(1)
			{
				if((stNew.uiSec - pstSession->stTimestamp.uiSec > uiSec) ||
					((stNew.uiSec - pstSession->stTimestamp.uiSec == uiSec)  &&
						(stNew.uiMsec - pstSession->stTimestamp.uiMsec >= uiMsec)
					)
				)
				{
					break;
				}
				base_delay_ms(10);
				base_time_get(&stNew);
			}
			
			/*发送*/
			iSendDataLen = 
				((WriteFun)(
					g_astSessionTypeInfo[stSessionSendData.iSessionType].stBaseOp.pWrite
				)
			)
			(
				stSessionSendData.iSessionId,
				stSessionSendData.aucData, 
				stSessionSendData.iDataLen
			);
			if(iSendDataLen != stSessionSendData.iDataLen)
			{
				HY_ERROR("Session send failed.\n");
				pstSession->iActiveFlag = 0;
			}
			else
			{
				HY_INFO("Send Len : %d\n", stSessionSendData.iDataLen);
				LOG_HEX (LOG_INFO, 
					stSessionSendData.aucData, 
					0, 
					stSessionSendData.iDataLen
				);
				/*记录接收时间*/
				base_time_get(&stSessionSendData.stTimestamp);
				
				HY_ERROR("pstSession = %p.\n", pstSession);
				HY_ERROR("pDataSplitCombineCb = %p.\n", pstSession->stCallbackList.pDataSplitCombineCb);
				HY_DEBUG("pstSession->pstDataRepeater = %p\n", pstSession->pstDataRepeater);
				if(NULL != pstSession->pstDataRepeater && 
					1 == stSessionSendData.iResend &&
					NULL != pstSession->stCallbackList.pRespondMatchingCb)
				{
					HY_DEBUG("pstSession->pstDataRepeater = %p\n", pstSession->pstDataRepeater);
					/*将数据添加到重发器*/
					pstSession->pstDataRepeater->sendPush(
						pstSession->pstDataRepeater,
						stSessionSendData.iSync,
						(void *)&stSessionSendData,
						iSessionDataLen
					);
					HY_DEBUG("pstSession->pstDataRepeater = %p\n", pstSession->pstDataRepeater);
				}
				else
				{
					/*如果数据无需加入重发器，
					*则需要释放数据中的pProcessedData域*/
					if(stSessionSendData.pProcessedData)
					{
						base_free(stSessionSendData.pProcessedData);
					}
				}
			}
			iMsgType = -1;
			iSessionDataLen = sizeof(session_data_t);
			base_memset(&stSessionSendData, 0x0, iSessionDataLen);
		}


		
		/*接收数据*/
		/*处理支持系统epoll的会话*/
		iEventNum = SESSION_MAX_NUM;
		pstFd->happen_event(
			pstFd, 
			astEpollEven,
			&iEventNum,
			SESSION_WRITE_TIEMOUT_MS
		);
		
		/*获取当前时间*/
		base_time_get(&stNew);
		
		if(iEventNum > 0)
		{
			for (i = 0; i < iEventNum; ++i)
			{				
				/*查找会话*/
				pstSession = 
					_session_find(pstSessionSer, 
					astEpollEven[i].iFd);
				if(NULL == pstSession)
				{
					HY_ERROR("The SessionId not found.\n");
					continue;
				}
				
				if(SESSION_TYPE_TCP_SERVER == pstSession->iSessionType && 
					(astEpollEven[i].uiEvent & EPOLLIN))
				{
					/*新客户端连接事件*/
					iRecvOriginalDataLen = 
						((ReadFun)(
							g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pRead
						)
					)
					(
						astEpollEven[i].iFd, 
						aucRecvOriginalData, 
						SESSION_DATA_MAX_LEN
					);
					if(iRecvOriginalDataLen < 0)
					{
						HY_ERROR("The client connection failed.\n");
						continue;
					}

					HY_INFO("New client connection: %s\n", aucRecvOriginalData);
					iRet = _session_client_connect(
						pstSessionSer,
						pstSession,
						(char *)aucRecvOriginalData);
					if(0 != iRet)
					{
						continue;
					}

					/*向服务器端推送新客户端连接信息*/
					_session_report_original_data(
						pstSessionSer,
						pstSession,
						aucRecvOriginalData,
						iRecvOriginalDataLen);
				}
				else if((astEpollEven[i].uiEvent & EPOLLIN))
				{
					/*读事件*/
					base_memset(aucRecvOriginalData,
						0x0, SESSION_DATA_MAX_LEN);
					iRecvOriginalDataLen = 
						((ReadFun)(
							g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pRead
						)
					)
					(
						astEpollEven[i].iFd, 
						aucRecvOriginalData, 
						SESSION_DATA_MAX_LEN
					);
					if(0 == iRecvOriginalDataLen)
					{
						/*检测到读事件，数据长度却是0，此时认为对方断掉*/
						HY_ERROR("Session recive failed.\n");
						pstSession->iActiveFlag = 0;
						continue;
					}
					else if(iRecvOriginalDataLen < 0)
					{
						/*对于linux tcp套接字的特殊处理*/
						if((SESSION_TYPE_TCP_SERVER == pstSession->iSessionType ||
							SESSION_TYPE_TCP_CLIENT == pstSession->iSessionType) &&
							EINTR == errno)
						{
							/*errno == EINTR 则说明recv函数是由于程序接收到信号后返回的，
							*socket连接还是正常的，不应close掉socket连接*/
							continue;
						}
						HY_ERROR("Session recive failed.\n");
						pstSession->iActiveFlag = 0;
						continue;
					}
					/*处理原始数据*/
					HY_INFO("Recv Original Len : %d\n", 
						iRecvOriginalDataLen);
					LOG_HEX(LOG_INFO, 
						aucRecvOriginalData, 
						0, 
						iRecvOriginalDataLen
					);
					/*未注册数据处理函数，则将原始数据直接返回*/
					if(NULL == pstSession->stCallbackList.pDataSplitCombineCb)
					{
						_session_report_original_data(
							pstSessionSer,
							pstSession,
							aucRecvOriginalData,
							iRecvOriginalDataLen);
					}
					else
					{
						_session_report_protocol_data(
							pstSessionSer,
							pstSession,
							aucRecvOriginalData,
							iRecvOriginalDataLen);
					}
				}
			}
		}
		/*处理支持自实现的select的会话*/
		while(NULL != (pSceeionInfo = 
				pstSessionSer->stLinkList.next(
					(double_link_list_class_t* )pstSessionSer,
					pSceeionInfo
				)
			)
		)
		{
			pstSession = (session_conf_t*)pSceeionInfo;

			
			if(1 == g_astSessionTypeInfo[pstSession->iSessionType].iAsyn && 
				0 == g_astSessionTypeInfo[pstSession->iSessionType].iSysPollSupport)
			{
				/**/
				iRet = 
					((ReadableFun)(
							g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pReadable
						)
					)
					(
						pstSession->iSessionId, 
						0
					);
				if(0 == iRet)
				{
					/*读事件*/
					base_memset(aucRecvOriginalData,
						0x0, SESSION_DATA_MAX_LEN);
					iRecvOriginalDataLen = 
						((ReadFun)(
								g_astSessionTypeInfo[pstSession->iSessionType].stBaseOp.pRead
							)
						)
						(
							astEpollEven[i].iFd, 
							aucRecvOriginalData, 
							SESSION_DATA_MAX_LEN
						);
					if(iRecvOriginalDataLen < 0)
					{
						HY_ERROR("Session recive failed.\n");
						pstSession->iActiveFlag = 0;
						continue;
					}
					/*处理原始数据*/
					HY_INFO("Recv Original Len : %d\n", iRecvOriginalDataLen);
					LOG_HEX(LOG_INFO, 
						aucRecvOriginalData, 
						0, 
						iRecvOriginalDataLen
					);

					if(NULL == pstSession->stCallbackList.pDataSplitCombineCb)
					{
						/*未定义分包组包回调*/

						/*直接上报*/
						_session_report_original_data(
							pstSessionSer,
							pstSession,
							aucRecvOriginalData,
							iRecvOriginalDataLen);
						
					}
					else
					{
						_session_report_protocol_data(
							pstSessionSer,
							pstSession,
							aucRecvOriginalData,
							iRecvOriginalDataLen);
					}
				}
			}



			/*会话连接维护，被动保活*/
			if(0 == pstSession->iActiveFlag)
			{
				
			}
			
			/*针对tcp连接的主动保活*/
			{
				
			}

			/*存在客户端会话，遍历客户端会话*/
			if(NULL != pstSession->pstClientList)
			{
				session_conf_t *pstCloseSceeion = NULL;
				
				while(NULL != (pClinetSceeionInfo = 
						pstSession->pstClientList->next(
							pstSession->pstClientList,
							pClinetSceeionInfo
						)
					)
				)
				{
					pstClientSession = 
						(session_conf_t*)pClinetSceeionInfo;
					
					if(NULL != pstCloseSceeion)
					{
						/*关闭*/
						session_close (pstSessionSer, 
							pstCloseSceeion->iSessionId);
						pstCloseSceeion = NULL;
					}
					
					if(1 == g_astSessionTypeInfo[pstClientSession->iSessionType].iAsyn && 
						0 == g_astSessionTypeInfo[pstClientSession->iSessionType].iSysPollSupport)
					{
						/**/
						iRet = 
							((ReadableFun)(
									g_astSessionTypeInfo[pstClientSession->iSessionType].stBaseOp.pReadable
								)
							)
							(
								pstClientSession->iSessionId, 
								0
							);
						if(0 == iRet)
						{
							/*读事件*/
							base_memset(aucRecvOriginalData,
								0x0, SESSION_DATA_MAX_LEN);
							iRecvOriginalDataLen = 
								((ReadFun)(
										g_astSessionTypeInfo[pstClientSession->iSessionType].stBaseOp.pRead
									)
								)
								(
									astEpollEven[i].iFd, 
									aucRecvOriginalData, 
									SESSION_DATA_MAX_LEN
								);
							if(iRecvOriginalDataLen < 0)
							{
								HY_ERROR("Session recive failed.\n");
								pstClientSession->iActiveFlag = 0;
								continue;
							}
							/*处理原始数据*/
							HY_INFO("Recv Original Len : %d\n", iRecvOriginalDataLen);
							LOG_HEX(LOG_INFO, 
								aucRecvOriginalData, 
								0, 
								iRecvOriginalDataLen
							);
		
							if(NULL == pstClientSession->stCallbackList.pDataSplitCombineCb)
							{
								/*未定义分包组包回调*/
		
								/*直接上报*/
								_session_report_original_data(
									pstSessionSer,
									pstClientSession,
									aucRecvOriginalData,
									iRecvOriginalDataLen);
								
							}
							else
							{
								_session_report_protocol_data(
									pstSessionSer,
									pstClientSession,
									aucRecvOriginalData,
									iRecvOriginalDataLen);
							}
						}
					}
		
		
					/*客户端连接断裂，无需保活*/
					if(0 == pstClientSession->iActiveFlag)
					{
						/*关闭会话*/
						/*关闭会话时，
						会删除该会话链表节点，
						如果此处删除，会造成next接口找不到下一个节点
						所以此处只记录要删除的数据，由下一个循环删除*/
						pstCloseSceeion = pstClientSession;
					}
					
					/*针对tcp连接的主动保活*/
					{
						
					}
						
				}

				/*处理最后一个循环中有需要关闭的会话*/
				if(NULL != pstCloseSceeion)
				{
					/*关闭*/
					session_close (pstSessionSer, 
						pstCloseSceeion->iSessionId);
					pstCloseSceeion = NULL;
				}
			}
			
		}
	}	
	return NULL;
}

/*构造函数*/
session_ser_class_t *new_session_ser(void)
{
	session_ser_class_t* pNew = 
		(session_ser_class_t *)base_calloc(
			1, 
			sizeof(session_ser_class_t)
		);
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error.\n");
		
		return NULL;
	}

	/*调用父类的构造函数*/
	double_link_list_class_t *pstLink = 
		new_double_link_list();
	if(NULL == pstLink)
	{
		HY_ERROR("New DevList Error.\n");
		base_free(pNew);
		return NULL;
	}

	/*将父类的数据拷贝到子类中*/
	base_memcpy(pNew, pstLink, 
		sizeof(double_link_list_class_t));
	/*pstLink里的某些成员也是动态分配的，而这些内存是要使用的，所以
	*此处指释放pstLink，而不释放成员的分配
	*/
	base_free(pstLink);

	/*参数初始化*/
	/*私有变量初始化*/
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)pNew->acPrivateParam;
	base_memset(pPrivate, 0x0, 
		sizeof(session_ser_private_t));

	/*初始化hash*/
	pPrivate->pstSessionHash = 
		new_hash_map(0, 1);
	if(NULL == pPrivate->pstSessionHash)
	{
		HY_ERROR("Failed to create a hash table.\n");
		base_free(pNew);
		return NULL;
	}

	
	if(NULL == pPrivate->pstSendMsgQueue)
	{
		/*初始化发送队列*/
		pPrivate->pstSendMsgQueue = new_msg_queue();
		if(NULL == pPrivate->pstSendMsgQueue)
		{	
			HY_ERROR("Failed to create a Send queue.\n");
			destroy_hash_map(pPrivate->pstSessionHash);
			base_free(pNew);
			return NULL;
		}
	}
	
	if(NULL == pPrivate->pstRecvMsgQueue)
	{
		/*初始化接收队列*/
		pPrivate->pstRecvMsgQueue = new_msg_queue();
		if(NULL == pPrivate->pstRecvMsgQueue)
		{	
			HY_ERROR("Failed to create a receive queue.\n");
			destroy_hash_map(pPrivate->pstSessionHash);
			destroy_msg_queue(pPrivate->pstSendMsgQueue);
			base_free(pNew);
			return NULL;
		}
	}
	
	if(NULL == pPrivate->pstSyncRecvMsgQueue)
	{
		/*初始化同步接收队列*/
		pPrivate->pstSyncRecvMsgQueue = new_msg_queue();
		if(NULL == pPrivate->pstSyncRecvMsgQueue)
		{	
			HY_ERROR("Failed to create a receive queue.\n");
			destroy_hash_map(pPrivate->pstSessionHash);
			destroy_msg_queue(pPrivate->pstSendMsgQueue);
			destroy_msg_queue(pPrivate->pstRecvMsgQueue);
			base_free(pNew);
			return NULL;
		}
	}


	/*初始化epoll*/
	pPrivate->pstEpollFd = new_epoll_fd(SESSION_MAX_NUM);
	if(NULL == pPrivate->pstEpollFd)
	{
		HY_FATAL("New epoll fd failed.\n");
		destroy_hash_map(pPrivate->pstSessionHash);
		destroy_msg_queue(pPrivate->pstSendMsgQueue);
		destroy_msg_queue(pPrivate->pstRecvMsgQueue);
		destroy_msg_queue(pPrivate->pstSyncRecvMsgQueue);
		base_free(pNew);
		
		return NULL;
	}
	
	/*创建处理线程*/
	if(0 != base_thread_create(
		&pPrivate->sessionSysPid, 
		session_sys_exec, 
		(void *)pNew)
	)
	{
		HY_ERROR("pthread_attr_init Error.\n");
		destroy_hash_map(pPrivate->pstSessionHash);
		destroy_msg_queue(pPrivate->pstSendMsgQueue);
		destroy_msg_queue(pPrivate->pstRecvMsgQueue);
		destroy_msg_queue(pPrivate->pstSyncRecvMsgQueue);
		destroy_epoll_fd(pPrivate->pstEpollFd);
		base_free(pNew);
        return NULL;
	}

	/*初始化会话类型*/
	_session_type_init();

	/*方法初始化*/
	pNew->open = session_open; 
	pNew->close = session_close; 
	pNew->ioctl = session_ioctl; 
	pNew->lseek = session_lseek; 
	pNew->send = session_send; 
	pNew->recv = session_recv; 
	pNew->callback_reg = session_callback_reg; 
	
	return pNew;
}

/*析构函数*/
int destroy_session_ser(session_ser_class_t *_this)
{
	if(NULL == _this)
	{
		return -1;
	}
	session_ser_private_t *pPrivate = 
		(session_ser_private_t *)_this->acPrivateParam;

	/*停止线程*/
	base_thread_cancel(pPrivate->sessionSysPid);
	base_delay_s(1);
	
	/*关闭会话*/
	void *pSceeionInfo = NULL;
	session_conf_t *pstSession = NULL;
	while(NULL != (pSceeionInfo = 
			_this->stLinkList.next(
				(double_link_list_class_t* )_this,
				pSceeionInfo
			)
		)
	)
	{
		pstSession = (session_conf_t*)pSceeionInfo;
		_this->close(_this, pstSession->iSessionId);
	}
	
	/*销毁队列*/
	destroy_msg_queue(pPrivate->pstSendMsgQueue);
	/*销毁队列*/
	destroy_msg_queue(pPrivate->pstRecvMsgQueue);
	/*销毁队列*/
	destroy_msg_queue(pPrivate->pstSyncRecvMsgQueue);
	/*销毁哈希表*/
	destroy_hash_map(pPrivate->pstSessionHash);

	/*销毁epoll监控*/
	destroy_epoll_fd(pPrivate->pstEpollFd);

	return destroy_double_link_list((double_link_list_class_t*)_this);
}

