/***********************************************************
*文件名     : data_repeater.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.31
*说   明   : 数据重发器
*修改记录: 
************************************************************/	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "base_api.h"
#include "data_repeater.h"
#include "log_api.h"

/*链表结点*/
typedef struct data_repeater_link_list_node_s
{
	struct data_repeater_link_list_node_s *pNext;
	struct data_repeater_link_list_node_s *pPrev;
	/*时间戳*/
	base_timeval_t stTimestamp;

	/*重发次数*/
	int iResendCount;
	/*数据类型标志位，1表示同步接口发出的数据，0表示异步接口发出的数据*/
	int iSyncFlag;
	/*数据*/
	int iDataLen;
	unsigned char aucData[0];
}__attribute__ ((__packed__)) data_repeater_link_list_node_t;

/*私有类型定义*/
typedef struct data_repeater_private_s  
{  
	data_repeater_link_list_node_t *pHead;
	data_repeater_link_list_node_t *pTail;
	/*重发器队列长度*/
	int iLen;
	/*超时时间,单位ms*/
	int iTimeOut;
	/*重发次数*/
	int iMaxResend;
	
	/*数据超时检测线程*/
	thread_t pid;

	/*注册重发接口*/
	void* pResendCb;
	void* pResendUserData;
	
	/*注册数据匹配接口*/
	void* pDataMatchingCb;
	void* pDataMatchingUserData;
	
	/*数据域释放回调*/
	void* pFreeDataCb;
}data_repeater_private_t;

/*新建结点*/
static data_repeater_link_list_node_t*
data_repeater_link_list_node_new(
	data_repeater_t* _this,
	int iSendFlag,
	void *pData,
	int iDataLen
)
{	
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
		
	data_repeater_link_list_node_t* pNewNode = 
		(data_repeater_link_list_node_t *)base_calloc(1, sizeof(data_repeater_link_list_node_t) + iDataLen);
	if(NULL == pNewNode)
	{
		return NULL;
	}
	pNewNode->iSyncFlag = iSendFlag;
	pNewNode->iResendCount = pPrivate->iMaxResend;
	pNewNode->iDataLen = iDataLen;
	base_memcpy(pNewNode->aucData, pData, iDataLen);
	base_time_get(&(pNewNode->stTimestamp));
	
	pPrivate->iLen ++;
	return pNewNode;
}
/*释放结点*/
static int
data_repeater_link_list_node_destroy(
	data_repeater_t* _this, 
	data_repeater_link_list_node_t* pNode
)
{
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
	
	base_free(pNode);
	pPrivate->iLen--;
	pNode = NULL;
	
	return NoErr;
}

/*释放结点*/
static int
data_repeater_link_list_free_data_node_destroy(
	data_repeater_t* _this, 
	data_repeater_link_list_node_t* pNode
)
{
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
		
	if(pPrivate->pFreeDataCb)
	{
		((FreeDataFun)(pPrivate->pFreeDataCb))(
			pNode->aucData, 
			pNode->iDataLen
		);
	}
	base_free(pNode);
	pPrivate->iLen--;
	pNode = NULL;
	
	return NoErr;
}

/*尾插入数据*/
static int 
data_repeater_link_list_tail_insert (
	data_repeater_t* _this,
	int iSendFlag,
	void *pData,
	int iDataLen
)
{
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;


	data_repeater_link_list_node_t* pNewNode = 
		data_repeater_link_list_node_new(_this, iSendFlag, pData, iDataLen);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}	
	
	if(NULL == pPrivate->pTail)
	{
		pPrivate->pHead = pNewNode;
		pPrivate->pTail = pNewNode;
	}
	else
	{
		pPrivate->pTail->pNext = pNewNode;
		pNewNode->pPrev = pPrivate->pTail;
		pPrivate->pTail = pNewNode;
	}
		
	return NoErr;
}


/*删除指定数据*/
static int
data_repeater_link_list_del (
	data_repeater_t* _this,
	data_repeater_link_list_node_t* pCurrent
)
{
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;

	if(pPrivate->pHead == pCurrent && pPrivate->pTail == pCurrent)
	{
		pPrivate->pHead = NULL;
		pPrivate->pTail = NULL;
		data_repeater_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pHead == pCurrent )
	{
		pPrivate->pHead = pCurrent->pNext;
		pCurrent->pNext->pPrev = NULL;
		data_repeater_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pTail == pCurrent)
	{
		pPrivate->pTail = pCurrent->pPrev;
		pCurrent->pPrev->pNext = NULL;
		data_repeater_link_list_node_destroy(_this, pCurrent);
	}
	else
	{
		pCurrent->pPrev->pNext = pCurrent->pNext;
		pCurrent->pNext->pPrev = pCurrent->pPrev;
		data_repeater_link_list_node_destroy(_this, pCurrent);
	}
	return NoErr;
}

/*清空数据*/
static int 
data_repeater_link_list_clear (data_repeater_t* _this)
{
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
	
	data_repeater_link_list_node_t *pNode = 
		pPrivate->pHead;
	data_repeater_link_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;

		data_repeater_link_list_free_data_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}

/*设置发送数据超时时间，单位ms，默认500ms*/
static int 
data_repeater_timeout(
	data_repeater_t *_this, 
	int iTimeOut
)
{
	if(NULL == _this
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	if(iTimeOut <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
	pPrivate->iTimeOut = iTimeOut;
	
	return NoErr;
}

/*设置最大重发次数，默认3次*/
static int 
data_repeater_max_resend(
	data_repeater_t *_this,
	int iMaxResend
)
{
	if(NULL == _this
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	if(iMaxResend <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;

	pPrivate->iMaxResend = iMaxResend;
	return NoErr;
}

/*注册数据重发接口*/
static int 
data_repeater_resend_cb_reg(
	data_repeater_t *_this, 
	void *pResendCb,
	void *pUserData
)
{
	if(NULL == _this ||
		NULL == pResendCb
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;

	pPrivate->pResendCb = pResendCb;
	pPrivate->pResendUserData = pUserData;
	return NoErr;
}

/*注册发送数据与回应数据配对回调接口*/
static int 
data_repeater_respond_matching_cb_reg(
	data_repeater_t *_this, 
	void *pRespondMatchingCb,
	void *pUserData
)
{
	if(NULL == _this ||
		NULL == pRespondMatchingCb
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;

	pPrivate->pDataMatchingCb = pRespondMatchingCb;
	pPrivate->pDataMatchingUserData = pUserData;
	return NoErr;
}

/*放入发送数据, iSendFlag: 0表示异步接口发出的数据, 1表示同步接口发出的数据, 2表示无需进行回复匹配*/
static int 
data_repeater_send_push(
	data_repeater_t *_this, 
	int iSendFlag,
	void *pData,
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pData
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	
	int iRet = 0;
		
	iRet = data_repeater_link_list_tail_insert (
		_this,
		iSendFlag,
		pData, 
		iDataLen
	);	
	if(0 != iRet)
	{
		HY_ERROR("Data inst failed.\n");
		error_num = ListInsertErr;
		return ListInsertErr;
	}

	return NoErr;
}


/*放入回应数据，用于匹配发送数据*/
static int 
data_repeater_respond_push(
	data_repeater_t *_this,
	void *pRecvData, 
	int iRecvDataLen,
	void *pSendData, 
	int *piSendDataLen
)
{
	if(NULL == _this ||
		NULL == pRecvData ||
		NULL == pSendData ||
		NULL == piSendDataLen
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	if(iRecvDataLen <= 0 ||
		*piSendDataLen <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
			(data_repeater_private_t *)_this->acPrivateParam;
	
	if(0 == pPrivate->iLen)
	{
		return NotFoundErr;
	}

	
	data_repeater_link_list_node_t *pNode = 
		pPrivate->pHead;
	while(pNode)
	{
		/*判断报文*/
		if(0 == ((DataMatchingFun)(pPrivate->pDataMatchingCb))(_this,
															pNode->aucData, 
															pNode->iDataLen, 
															pRecvData, 
															iRecvDataLen, 
															pPrivate->pDataMatchingUserData)
		)
		{
			/*匹配到发送报文*/	
			/*返回消息数据*/
			*piSendDataLen = (*piSendDataLen >= pNode->iDataLen) ? 
							pNode->iDataLen : 
							*piSendDataLen;
			base_memcpy(pSendData, pNode->aucData, *piSendDataLen);

			/*删除该发送报文*/
			data_repeater_link_list_del(_this, pNode);

			return NoErr;
		}
		pNode = pNode->pNext;
	}
	

	return NotFoundErr;
}



/*注册数据域释放回调，
* 如果数据域中有某一部分数据是动态分配的，
* 则需要注册释放回调接口*/
static int data_repeater_free_data_cb_reg (data_repeater_t* _this, void* pFreeDataCb)
{
	if(NULL == _this ||
		NULL == pFreeDataCb
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
			(data_repeater_private_t *)_this->acPrivateParam;
	
	pPrivate->pFreeDataCb = pFreeDataCb;
	return NoErr;
}


/*打印数据*/
static int data_repeater_print(data_repeater_t* _this, void* pPrintDataCb)
{
	if(NULL == _this ||
		NULL == pPrintDataCb
	)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
	
	data_repeater_link_list_node_t *pNode = 
		pPrivate->pHead;
	while(pNode)
	{
		((PrintDataFun)pPrintDataCb)(
			pNode->aucData, 
			pNode->iDataLen
		);
		pNode = pNode->pNext;
	}
	return NoErr;
}

/*数据报文处理线程函数*/
static void*
data_repeater_manage(void *arg)
{
	data_repeater_t *pstDataRepeater =
		(data_repeater_t *)arg;
		
	
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)pstDataRepeater->acPrivateParam;

	/*开启线程取消功能*/
	base_thread_set_cancel();

	
	while(1)
	{
		/*线程取消点*/
		base_thread_cancel_point();
		
		/*减少cpu的占用*/
		base_delay_ms(10);
		
		if(0 == pPrivate->iLen)
		{
			
			continue;
		}
		
		base_timeval_t stNow;
		base_time_get(&(stNow));
		unsigned long ulSec  = pPrivate->iTimeOut / 1000;
		unsigned long ulMssec = pPrivate->iTimeOut % 1000; 
		
		
		data_repeater_link_list_node_t *pNode = 
			pPrivate->pHead;
		while(pNode)
		{
			/*判断报文是否超时*/
			base_timeval_t *pstTimestamp = &(pNode->stTimestamp);
			if((stNow.uiSec - pstTimestamp->uiSec > ulSec) ||
				((stNow.uiSec - pstTimestamp->uiSec == ulSec)  &&
					(stNow.uiMsec - pstTimestamp->uiMsec >= ulMssec)
				)
			)
			{
				/*超时*/
				/*判断是否需要重发*/
				if(0 == pNode->iResendCount)
				{
					/*已达到最大重发次数，无需重发*/
					data_repeater_link_list_node_t *pNodeDel = pNode;
					pNode = pNode->pNext;

					data_repeater_link_list_del (
						pstDataRepeater,
						pNodeDel
					);
					continue;
				}
				else if(pNode->iResendCount > 0)
				{
					base_memcpy(pstTimestamp, &stNow, sizeof(base_timeval_t));
					/*重发次数减一*/
					pNode->iResendCount --;
					/*重发*/
					((ResendFun)(pPrivate->pResendCb))(
						pstDataRepeater,
						pNode->aucData, 
						pNode->iDataLen,
						pPrivate->pResendUserData);
				}
			}
			
			pNode = pNode->pNext;
		}
	}
	return NULL;
}


/*构造函数*/
data_repeater_t *new_data_repeater(int iTimeOut, int iMaxResend)
{
	if(iTimeOut <= 0 ||
		iMaxResend <= 0)
	{
		error_num = ParaErr;
		return NULL;
	}
		
	/*申请空间*/
	data_repeater_t *pNew = 
		(data_repeater_t *)base_calloc(1, sizeof(data_repeater_t));
	if(NULL == pNew)
	{
		return NULL;
	}

	data_repeater_private_t *pPrivate =
		(data_repeater_private_t *)pNew->acPrivateParam;
	
	/*超时时间,单位ms*/
	pPrivate->iTimeOut = iTimeOut;
	/*重发次数*/
	pPrivate->iMaxResend = iMaxResend;
	
	if(0 != base_thread_create(
		&pPrivate->pid, 
		data_repeater_manage, 
		(void*)pNew)
	)
	{
		HY_ERROR("pthread_attr_init Error.\n");
		base_free(pNew);
        return NULL;
	}
	
	pNew->timeout = data_repeater_timeout;
	pNew->maxResend = data_repeater_max_resend;
	pNew->resendCbReg = data_repeater_resend_cb_reg;
	pNew->respondMatchingCbReg = data_repeater_respond_matching_cb_reg;
	pNew->sendPush = data_repeater_send_push;
	pNew->respondPush = data_repeater_respond_push;
	pNew->freeDataCbReg = data_repeater_free_data_cb_reg;
	pNew->print = data_repeater_print;
	
	return pNew;
}


/*析构函数*/
int destroy_data_repeater(data_repeater_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	data_repeater_private_t *pPrivate = 
		(data_repeater_private_t *)_this->acPrivateParam;
	
	/*取消线程*/
	base_thread_cancel(pPrivate->pid);
	base_delay_s(1);
	
	if(pPrivate->iLen)
	{
		/*清空链表*/
		data_repeater_link_list_clear (_this);
	}
		
	base_free(_this);
	_this = NULL;
	return NoErr;
}

