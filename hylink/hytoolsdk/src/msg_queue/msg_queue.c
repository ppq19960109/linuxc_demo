/***********************************************************
*文件名     : msg_queue.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 消息队列类
*修改记录: 
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"
#include "msg_queue.h"
#include "pthread.h"

#define MSG_TYPE_CHECK_TIMEOUT	3600


/*链表结点*/
typedef struct msg_link_list_node_s
{
	struct msg_link_list_node_s *pNext;
	struct msg_link_list_node_s *pPrev;
	int iMsgType;
	int iMsgLen;
	unsigned char aucMsg[0];
}msg_link_list_node_t;


typedef struct msg_type_link_list_node_s
{
	struct msg_type_link_list_node_s *pNext;
	struct msg_type_link_list_node_s *pPrev;
	int iMsgType;
	/*信号量*/
	void *Sem;
	/*该消息类在队列中的长度*/
	int iCount;
	/*时间戳*/
	base_timeval_t stTimetamp;
} msg_type_link_list_node_t;


/*私有类型定义*/
typedef struct msg_queue_private_s  
{  
	msg_link_list_node_t *pMsgHead;
	msg_link_list_node_t *pMsgTail;
	msg_type_link_list_node_t *pMsgTypeHead;
	msg_type_link_list_node_t *pMsgTypeTail;
	
	/*信号量*/
	void *Sem;
	int iMsgCount;
	int iMsgTypeCount;
	void *pFreeDataCb;
	
	/*互斥锁*/
	void *mutex;
} msg_queue_private_t;


/*新建结点*/
static msg_link_list_node_t*
msg_link_list_node_new(
	msg_queue_class_t* _this,
	int iMsgType,
	void* pMsgData,
	int iDataLen
)
{	
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
		
	msg_link_list_node_t* pNewNode = 
		(msg_link_list_node_t *)base_calloc(1, sizeof(msg_link_list_node_t) + iDataLen);
	if(NULL == pNewNode)
	{
		return NULL;
	}
	pNewNode->iMsgType = iMsgType;
	pNewNode->iMsgLen = iDataLen;
	base_memcpy(pNewNode->aucMsg, pMsgData, iDataLen);
	pPrivate->iMsgCount ++;
	return pNewNode;
}
/*释放结点*/
static int
msg_link_list_node_destroy(
	msg_queue_class_t* _this, 
	msg_link_list_node_t* pNode
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
		
	
	base_free(pNode);
	pPrivate->iMsgCount--;
	pNode = NULL;
	
	return NoErr;
}

/*释放结点(调用数据域释放回调)*/
static int
msg_link_list_free_data_node_destroy(
	msg_queue_class_t* _this, 
	msg_link_list_node_t* pNode
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
		
	if(pPrivate->pFreeDataCb)
	{
		((FreeDataFun)(pPrivate->pFreeDataCb))(
			pNode->aucMsg, 
			pNode->iMsgLen
		);
	}
	base_free(pNode);
	pPrivate->iMsgCount--;
	pNode = NULL;
	
	return NoErr;
}

/*获取链表头数据指针*/
static msg_link_list_node_t * 
msg_link_list_head (msg_queue_class_t* _this)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	return pPrivate->pMsgHead;
}

/*从头结点开始查找首次出现iMsgType的消息*/
static msg_link_list_node_t* 
msg_link_list_find (msg_queue_class_t* _this, int iMsgType)
{
	
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
	
	msg_link_list_node_t *pNode = 
		pPrivate->pMsgHead;
	while(pNode)
	{
		if(pNode->iMsgType == iMsgType)
		{
			return pNode;
		}
		
		pNode = pNode->pNext;
	}
	
	return NULL;
}


/*尾插入数据*/
static int 
msg_link_list_tail_insert (
	msg_queue_class_t* _this,
	int iMsgType,
	void* pMsgData, 
	int iDataLen
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	msg_link_list_node_t* pNewNode = 
		msg_link_list_node_new(_this, iMsgType, pMsgData, iDataLen);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}	
	
	if(NULL == pPrivate->pMsgTail)
	{
		pPrivate->pMsgHead = pNewNode;
		pPrivate->pMsgTail = pNewNode;
	}
	else
	{
		pPrivate->pMsgTail->pNext = pNewNode;
		pNewNode->pPrev = pPrivate->pMsgTail;
		pPrivate->pMsgTail = pNewNode;
	}
	
	return NoErr;
}


/*删除指定数据*/
static int
msg_link_list_del (
	msg_queue_class_t* _this,
	msg_link_list_node_t* pCurrent
)
{
	msg_queue_private_t *pPrivate = (msg_queue_private_t *)_this->acPrivateParam;

	if(pPrivate->pMsgHead == pCurrent && pPrivate->pMsgTail == pCurrent)
	{
		pPrivate->pMsgHead = NULL;
		pPrivate->pMsgTail = NULL;
		msg_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pMsgHead == pCurrent)
	{
		pPrivate->pMsgHead = pCurrent->pNext;
		pCurrent->pNext->pPrev = NULL;
		msg_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pMsgTail == pCurrent)
	{
		pPrivate->pMsgTail = pCurrent->pPrev;
		pCurrent->pPrev->pNext = NULL;
		msg_link_list_node_destroy(_this, pCurrent);
	}
	else
	{
		pCurrent->pPrev->pNext = pCurrent->pNext;
		pCurrent->pNext->pPrev = pCurrent->pPrev;
		msg_link_list_node_destroy(_this, pCurrent);
	}
	return NoErr;
}

/*清空数据*/
static int 
msg_link_list_clear (msg_queue_class_t* _this)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
	
	msg_link_list_node_t *pNode = 
		pPrivate->pMsgHead;
	msg_link_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;

		msg_link_list_free_data_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}

/*新建结点*/
static msg_type_link_list_node_t*
msg_type_link_list_node_new(
	msg_queue_class_t* _this,
	int iMsgType
)
{	
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
		
	msg_type_link_list_node_t* pNewNode = 
		(msg_type_link_list_node_t *)base_calloc(1, sizeof(msg_type_link_list_node_t));
	if(NULL == pNewNode)
	{
		return NULL;
	}
	pNewNode->iMsgType = iMsgType;
	base_time_get(&(pNewNode->stTimetamp));

	pNewNode->Sem = base_sem_create (0, 0);
	if (NULL == pNewNode->Sem)
	{
		HY_ERROR("Semaphore initialization failed.\n");
		base_free(pNewNode);
		return NULL;
	}
	
	pPrivate->iMsgTypeCount ++;
	return pNewNode;
}
/*释放结点*/
static int
msg_type_link_list_node_destroy(
	msg_queue_class_t* _this, 
	msg_type_link_list_node_t* pNode
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
		
	/*销毁信号量*/
	base_sem_destroy(pNode->Sem);
	
	base_free(pNode);
	
	pPrivate->iMsgTypeCount--;
	pNode = NULL;
	
	return NoErr;
}


/*尾插入数据*/
static int 
msg_type_link_list_tail_insert (
	msg_queue_class_t* _this,
	int iMsgType
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;


	msg_type_link_list_node_t* pNewNode = 
		msg_type_link_list_node_new(_this, iMsgType);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}	
	
	if(NULL == pPrivate->pMsgTypeTail)
	{
		pPrivate->pMsgTypeHead = pNewNode;
		pPrivate->pMsgTypeTail = pNewNode;
	}
	else
	{
		pPrivate->pMsgTypeTail->pNext = pNewNode;
		pNewNode->pPrev = pPrivate->pMsgTypeTail;
		pPrivate->pMsgTypeTail = pNewNode;
	}
		
	return NoErr;
}


/*删除指定数据*/
static int
msg_type_link_list_del (
	msg_queue_class_t* _this,
	msg_type_link_list_node_t* pCurrent
)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	
	if(pPrivate->pMsgTypeHead == pCurrent && pPrivate->pMsgTypeTail == pCurrent)
	{
		pPrivate->pMsgTypeHead = NULL;
		pPrivate->pMsgTypeTail = NULL;
		msg_type_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pMsgTypeHead == pCurrent)
	{
		pPrivate->pMsgTypeHead = pCurrent->pNext;
		pCurrent->pNext->pPrev = NULL;
		msg_type_link_list_node_destroy(_this, pCurrent);
	}
	else if(pPrivate->pMsgTypeTail == pCurrent)
	{
		pPrivate->pMsgTypeTail = pCurrent->pPrev;
		pCurrent->pPrev->pNext = NULL;
		msg_type_link_list_node_destroy(_this, pCurrent);
	}
	else
	{
		pCurrent->pPrev->pNext = pCurrent->pNext;
		pCurrent->pNext->pPrev = pCurrent->pPrev;
		msg_type_link_list_node_destroy(_this, pCurrent);
	}
	return NoErr;
}

/*清空数据*/
static int 
msg_type_link_list_clear (msg_queue_class_t* _this)
{
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
	
	msg_type_link_list_node_t *pNode = 
		pPrivate->pMsgTypeHead;
	msg_type_link_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;

		msg_type_link_list_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}

/*查找维护消息类型*/
static msg_type_link_list_node_t* 
msg_type_find(msg_queue_class_t* _this, int iMsgType)
{
	//printf("msg_type_find\n");
	int iRet = 0;
	int iFlag = 0;
	msg_type_link_list_node_t *pstMsgTypeInfo = NULL;
	
	msg_queue_private_t *pPrivate = 
			(msg_queue_private_t *)_this->acPrivateParam;
	base_timeval_t stNow;

	base_time_get(&stNow);

	msg_type_link_list_node_t *pNode = pPrivate->pMsgTypeHead;
	
	while(pNode)
	{
		/*校验最后收到该消息类型的时间，如果较长时间未收到该类型，
		*且队列中该类型的长度为0，则删除该消息类型*/
		if(0 == pNode->iCount && 
			(stNow.uiSec - pNode->stTimetamp.uiSec >= MSG_TYPE_CHECK_TIMEOUT))
		{
			msg_type_link_list_node_t *pDelNode = pNode;
			pNode = pNode->pNext;

 			msg_type_link_list_del(_this, pDelNode);
			continue;
		}

		if(iMsgType == pNode->iMsgType)
		{
			pstMsgTypeInfo = pNode;
			iFlag = 1;
			
			/*更新时间*/
			base_memcpy(&(pNode->stTimetamp), &stNow, sizeof(base_timeval_t));
			break;
		}
		pNode = pNode->pNext;
	}
	
	if(0 == iFlag)
	{
		iRet = msg_type_link_list_tail_insert(_this, iMsgType);
		if(0 != iRet)
		{
			error_num = ListInsertErr;
			return NULL;
		}

		pstMsgTypeInfo = pPrivate->pMsgTypeTail;
	}
	return pstMsgTypeInfo;
}

/*设备列表加锁(阻塞)*/
static int 
msg_queue_lock (msg_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*设备列表加锁(非阻塞)*/
static int
msg_queue_trylock (msg_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*设备列表解锁*/
static int
msg_queue_unlock (msg_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}


/*获取队列长度*/
static int
msg_queue_len(msg_queue_class_t* _this, int iMsgType)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iLen = 0;
	msg_queue_private_t *pPrivate = 
			(msg_queue_private_t *)_this->acPrivateParam;
	
	if(iMsgType < 0)
	{
		iLen = pPrivate->iMsgCount;
	}
	else
	{
		msg_type_link_list_node_t *pstMsgTypeInfo  = 
			msg_type_find(_this, iMsgType);
		if(NULL == pstMsgTypeInfo)
		{
			iLen = -1;
		}
		else
		{
			iLen = pstMsgTypeInfo->iCount;
		}
	}
	return iLen;
}

/*入队*/
static int
msg_queue_push(
	msg_queue_class_t* _this,
	int iMsgType,
	void* pData, 
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pData)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	if(iMsgType < 0 ||
		iDataLen <= 0)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
	msg_queue_private_t *pPrivate = 
			(msg_queue_private_t *)_this->acPrivateParam;

	/*查找维护*/
	msg_type_link_list_node_t *pstMsgTypeInfo  =
		msg_type_find(_this, iMsgType);
	if(NULL == pstMsgTypeInfo)
	{
		return GeneralErr;
	}

	/*入队*/
	iRet = msg_link_list_tail_insert (
		_this,
		iMsgType,
		pData, 
		iDataLen
	);
	if(0 != iRet)
	{
		HY_ERROR("push msg failed.\n");
		error_num = QueuePushErr;
		iRet = QueuePushErr;
		goto msg_queue_push_end;
	}
	
	pstMsgTypeInfo->iCount ++;

	if ((iRet = base_sem_post(pstMsgTypeInfo->Sem)) < 0)
	{
		iRet = GeneralErr;
		goto msg_queue_push_end;
	}
	
	if (base_sem_post(pPrivate->Sem) < 0)
	{
		HY_ERROR("Semaphore post failed.\n");
		iRet = GeneralErr;
		goto msg_queue_push_end;
	}
	
msg_queue_push_end:
	return iRet;
}

/*检测信号量*/
static int msg_sem_wait(void *Sem, int iTimeOut)
{
	int iRet = 0;
	if(-1 == iTimeOut)
	{
		/*阻塞*/
		if((iRet = base_sem_wait(Sem)) < 0)
		{
			HY_ERROR("Semaphore sem_wait failed.");
			return iRet;
		}
	}
	else if(0 == iTimeOut)
	{
		/*非阻塞*/
		if((iRet = base_sem_trywait(Sem)) < 0)
		{
			//HY_ERROR("The message queue is empty.\n");
			return iRet;
		}
	}
	else
	{
		/*超时*/
		base_timeval_t stTimeout;
		base_time_get(&stTimeout);
		
		stTimeout.uiSec += iTimeOut / 1000;
		stTimeout.uiMsec += iTimeOut % 1000;
		
		if((iRet = base_sem_timedwait(Sem, &stTimeout)) < 0)
		{
			//HY_WARN("The message queue is timeout.\n");
			return iRet;
		}
		
	}
	return NoErr;
}

/*出队,iTimeOut:   0表示非阻塞，-1表示阻塞, >0表示超时时间，单位ms*/
static int
msg_queue_pop(
	msg_queue_class_t* _this,
	int *piMsgType, 
	int iTimeOut,
	void* pMsg, 
	int *piMsgLen
)
{
	if(NULL == _this ||
		NULL == piMsgType ||
		NULL == pMsg ||
		NULL == piMsgLen)
	{
		return ParaErr;
	}
	if(*piMsgType < -1 ||
		iTimeOut < -1 ||
		*piMsgLen <=0)
	{
		return ParaErr;
	}

	msg_link_list_node_t* pstMsg = NULL;
	msg_queue_private_t *pPrivate = 
			(msg_queue_private_t *)_this->acPrivateParam;
	
	if(-1 == *piMsgType)
	{
		/*全部消息类型*/
		if(msg_sem_wait(pPrivate->Sem, iTimeOut))
		{
			return GeneralErr;
		}



		/*获取队头结点*/
		pstMsg = 
			msg_link_list_head(_this);
		if(NULL == pstMsg)
		{
			HY_ERROR("Pop failed.\n");
			error_num = QueuePopErr;
			return QueuePopErr;
		}
		
		/*返回消息数据*/
		*piMsgType = pstMsg->iMsgType;
		*piMsgLen = (*piMsgLen >= pstMsg->iMsgLen) ? pstMsg->iMsgLen : *piMsgLen;
		
		base_memcpy(pMsg, pstMsg->aucMsg, *piMsgLen);

		/*出队*/
 		msg_link_list_del(_this, pstMsg);
		

		/*处理该消息类型的信号量*/
		msg_type_link_list_node_t *pstMsgTypeInfo = 
			msg_type_find(_this, *piMsgType);
		if(NULL != pstMsgTypeInfo)
		{
			base_sem_trywait(pstMsgTypeInfo->Sem);
			pstMsgTypeInfo->iCount --;
		}
	}
	else
	{
		/*特定消息类型*/
	

		/*查找维护*/
		msg_type_link_list_node_t *pstMsgTypeInfo  = 
			msg_type_find(_this, *piMsgType);
		if(NULL == pstMsgTypeInfo)
		{
			return GeneralErr;
		}
		/*全部消息类型*/
		if(msg_sem_wait(pstMsgTypeInfo->Sem, iTimeOut))
		{
			return GeneralErr;
		}

		/*从消息队列中查找该消息类型的消息*/
		pstMsg = msg_link_list_find (_this, *piMsgType);
		if(NULL == pstMsg)
		{
			HY_ERROR("Pop failed.\n");
			error_num = QueuePopErr;
			return QueuePopErr;
		}
		

		/*返回消息数据*/
		*piMsgType = pstMsg->iMsgType;
		*piMsgLen = (*piMsgLen >= pstMsg->iMsgLen) ? pstMsg->iMsgLen : *piMsgLen;
		base_memcpy(pMsg, pstMsg->aucMsg, *piMsgLen);

		
		/*出队*/
 		msg_link_list_del(_this, pstMsg);
		

		/*处理总消息类型的信号量*/
		pstMsgTypeInfo->iCount --;
		base_sem_trywait(pPrivate->Sem);
	}
	

	return NoErr;
}

/*清空队列*/
static int msg_queue_clear(msg_queue_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
			
	/*清空消息队列*/
	iRet += msg_link_list_clear(_this);
	/*清空消息类型*/
	iRet += msg_type_link_list_clear(_this);

	return iRet;
}

/*注册数据域释放回调，
* 如果数据域中有某一部分数据是动态分配的，
* 则需要注册释放回调接口*/
static int 
msg_queue_free_msg_cb_reg(
	msg_queue_class_t* _this,
	void* pFreeDataCb
)
{
	if(NULL == _this ||
		NULL == pFreeDataCb)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	msg_queue_private_t *pPrivate = 
			(msg_queue_private_t *)_this->acPrivateParam;

	pPrivate->pFreeDataCb = pFreeDataCb;

	return NoErr;
}

/*打印数据*/
static int 
msg_queue_print(
	msg_queue_class_t* _this,
	void* pPrintDataCb
)
{
	if(NULL == _this ||
		NULL == pPrintDataCb)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;

	HY_INFO("msg_queue len = %d\n", pPrivate->iMsgCount);
		
	msg_link_list_node_t *pNode = 
		pPrivate->pMsgHead;
	while(pNode)
	{
		HY_INFO("msg_type = %d\n", pNode->iMsgType);
		((PrintDataFun)pPrintDataCb)(
			pNode->aucMsg, 
			pNode->iMsgLen
		);
		pNode = pNode->pNext;
	}
	return NoErr;
}


/*构造函数*/
msg_queue_class_t *new_msg_queue(void)
{
	/*申请空间*/
	msg_queue_class_t *pNew = 
		(msg_queue_class_t *)base_calloc(1, sizeof(msg_queue_class_t));
	if(NULL == pNew)
	{
		return NULL;
	}
	
	/*私有变量初始化*/
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)pNew->acPrivateParam;
	base_memset(pPrivate, 0x0, sizeof(msg_queue_private_t));
	
	/*创建信号量*/
	if (NULL == (pPrivate->Sem = base_sem_create(0, 0)))
	{
		HY_ERROR("Semaphore initialization failed.\n");
        base_free(pNew);
        return NULL;
	}

	/*初始化互斥锁*/
	if (NULL == (pPrivate->mutex = base_mutex_lock_create()))
	{
		HY_ERROR("Mutex initialization failed.\n");
		base_free(pNew);
        return NULL;
	}
	pNew->lock = msg_queue_lock;
	pNew->trylock = msg_queue_trylock;
	pNew->unlock = msg_queue_unlock;
	pNew->len = msg_queue_len;
	pNew->push = msg_queue_push;
	pNew->pop = msg_queue_pop;
	pNew->clear = msg_queue_clear;
	pNew->freeDataCbReg = msg_queue_free_msg_cb_reg;
	pNew->print = msg_queue_print;
	
	return pNew;
}


/*析构函数*/
int destroy_msg_queue(msg_queue_class_t *_this)
{	
	if(NULL == _this)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	/*私有变量初始化*/
	msg_queue_private_t *pPrivate = 
		(msg_queue_private_t *)_this->acPrivateParam;
	
	if(pPrivate->iMsgCount > 0)
	{
		/*清空链表*/
		_this->clear(_this);
	}
	/*销毁信号量*/
	base_sem_destroy(pPrivate->Sem);

	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivate->mutex);

	base_free(_this);
	_this = NULL;
	return NoErr;
}


