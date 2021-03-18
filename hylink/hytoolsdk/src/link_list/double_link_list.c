/***********************************************************
*文件名     : double_link_list.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.14
*说   明   : 双向链表
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "log_api.h"
#include "base_api.h"
#include "link_list.h"

/*链表结点*/
typedef struct double_link_list_node_s
{
	struct double_link_list_node_s *pNext;
	struct double_link_list_node_s *pPrev;
	int iDataLen;
	unsigned char aucData[0];
}__attribute__ ((__packed__)) double_link_list_node_t;


/*私有类型定义*/
typedef struct double_link_list_private_s  
{  
	double_link_list_node_t *pHead;
	double_link_list_node_t *pTail;
	int iListLen;
	void *pFreeDataCb;
	/*互斥锁*/
	void *mutex;
}double_link_list_private_t;

/*新建结点*/
static double_link_list_node_t*
double_link_list_node_new(
	double_link_list_class_t* _this,
	void* pData,
	int iDataLen
)
{	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
		
	double_link_list_node_t* pNewNode = 
		(double_link_list_node_t *)base_calloc(1, sizeof(double_link_list_node_t) + iDataLen);
	if(NULL == pNewNode)
	{
		return NULL;
	}
	pNewNode->iDataLen = iDataLen;
	base_memcpy(pNewNode->aucData, pData, iDataLen);
	pPrivate->iListLen ++;
	return pNewNode;
}
/*释放结点*/
static int
double_link_list_node_destroy(
	double_link_list_class_t* _this, 
	double_link_list_node_t* pNode
)
{
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
		
	if(pPrivate->pFreeDataCb)
	{
		((PrintDataFun)(pPrivate->pFreeDataCb))(
			pNode->aucData, 
			pNode->iDataLen
		);
	}
	base_free(pNode);
	pPrivate->iListLen--;
	pNode = NULL;
	
	return NoErr;
}
/*根据数据指针，返回链表结点指针*/
static double_link_list_node_t*
double_link_list_node_ptr (void* pCurrentData)
{
	return pCurrentData - sizeof(int) - sizeof(struct double_link_list_node_s *) - sizeof(struct double_link_list_node_s *);;
}

/*加锁(阻塞)*/
static int 
double_link_list_lock (double_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*加锁(非阻塞)*/
static int
double_link_list_trylock (double_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*解锁*/
static int
double_link_list_unlock (double_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}

/*获取链表长度*/
static int
double_link_list_len (double_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->iListLen;
}

/*获取链表头数据指针*/
static void* 
double_link_list_head (double_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return NULL;
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->pHead->aucData;
}

/*获取链表尾数据指针*/
static void* 
double_link_list_tail (double_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		HY_ERROR("Parameter error.\n");
		return NULL;
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->pTail->aucData;
}

/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
static void* 
double_link_list_next (
	double_link_list_class_t* _this, 
	void* pCurrentData
)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return NULL;
	}
	
	if(NULL == pCurrentData)
	{
		double_link_list_private_t *pPrivate = 
			(double_link_list_private_t *)_this->acPrivateParam;
		return (NULL == pPrivate->pHead) ? NULL : (void*)(pPrivate->pHead->aucData);
	}
	else
	{
		double_link_list_node_t *pNode = 
			double_link_list_node_ptr (pCurrentData);
		return (NULL == pNode->pNext) ? NULL : (void*)(pNode->pNext->aucData);
	}
}

/*获取prev数据指针, 如果pCurrentData=NULL，则返回尾数据指针。
返回值为所需要的prev数据指针，如果返回NULL，则表示没有prev*/
static void* 
double_link_list_prev (
	double_link_list_class_t* _this,
	void* pCurrentData
)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return NULL;
	}
	
	if(NULL == pCurrentData)
	{
		double_link_list_private_t *pPrivate = 
			(double_link_list_private_t *)_this->acPrivateParam;
		return (NULL == pPrivate->pTail) ? NULL : (void*)(pPrivate->pTail->aucData);
	}
	else
	{
		double_link_list_node_t *pNode = 
			double_link_list_node_ptr (pCurrentData);
		return (NULL == pNode->pPrev) ? NULL : (void*)(pNode->pPrev->aucData);
	}
}

/*查找数据，返回数据指针*/
static void* 
double_link_list_find (
	double_link_list_class_t* _this, 
	void* pCmpDataCb,
	void* pData, 
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pCmpDataCb ||
		NULL == pData)
	{
		error_num = ParaErr;
        return NULL;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return NULL;
	}

	void* pCurrentData = NULL;
	while(NULL != (pCurrentData = _this->next(_this, pCurrentData)))
	{
		if(0 == ((CmpDataFunc)pCmpDataCb)(pCurrentData, *(int *)((unsigned char *)pCurrentData - 4), pData, iDataLen))
		{
			return pCurrentData;
		}
	}
	error_num = NotFoundErr;
	return NULL;
}


/*头插入数据*/
static int 
double_link_list_head_insert (
	double_link_list_class_t* _this,
	void* pData,
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;


	double_link_list_node_t* pNewNode = 
		double_link_list_node_new(_this, pData, iDataLen);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}
	
	if(NULL == pPrivate->pHead)
	{
		pPrivate->pHead = pNewNode;
		pPrivate->pTail = pNewNode;
	}
	else
	{
		pNewNode->pNext = pPrivate->pHead;
		pPrivate->pHead->pPrev = pNewNode;
		pPrivate->pHead = pNewNode;
	}
	return NoErr;
}

/*尾插入数据*/
static int 
double_link_list_tail_insert (
	double_link_list_class_t* _this,
	void* pData, 
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;


	double_link_list_node_t* pNewNode = 
		double_link_list_node_new(_this, pData, iDataLen);
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

/*当前位置入数据*/
static int
double_link_list_insert (
	double_link_list_class_t* _this,
	void* pCurrentData, 
	void* pData, 
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	double_link_list_node_t *pCurrNode = 
		double_link_list_node_ptr(pCurrentData);

	if(pPrivate->pHead == pCurrNode)
	{
		return _this->headInsert(_this, pData, iDataLen);
	}
	else
	{
		double_link_list_node_t* pNewNode = 
			double_link_list_node_new(_this, pData, iDataLen);
		if(NULL == pNewNode)
		{
			return GeneralErr;
		}
		pNewNode->pNext = pCurrNode;
		pNewNode->pPrev = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = pNewNode;
		pCurrNode->pPrev = pNewNode;
		return NoErr;
	}
}


/*删除指定数据*/
static int
double_link_list_del (
	double_link_list_class_t* _this,
	void* pCurrentData
)
{
	if(NULL == _this ||
		NULL == pCurrentData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;

	double_link_list_node_t *pCurrNode = 
		double_link_list_node_ptr(pCurrentData);
	
	if(pPrivate->pHead == pCurrNode && pPrivate->pTail == pCurrNode)
	{
		pPrivate->pHead = NULL;
		pPrivate->pTail = NULL;
		double_link_list_node_destroy(_this, pCurrNode);
	}
	else if(pPrivate->pHead == pCurrNode )
	{
		pPrivate->pHead = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = NULL;
		double_link_list_node_destroy(_this, pCurrNode);
	}
	else if(pPrivate->pTail == pCurrNode)
	{
		pPrivate->pTail = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = NULL;
		double_link_list_node_destroy(_this, pCurrNode);
	}
	else
	{
		pCurrNode->pPrev->pNext = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = pCurrNode->pPrev;
		double_link_list_node_destroy(_this, pCurrNode);
	}
	return NoErr;
}

/*清空数据*/
static int 
double_link_list_clear (double_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	
	double_link_list_node_t *pNode = 
		pPrivate->pHead;
	double_link_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;

		double_link_list_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}



/*修改指定数据*/
static int 
double_link_list_set (
	double_link_list_class_t* _this,
	void* pCurrentData, 
	void* pData,
	int iDataLen
)
{
	if(NULL == _this ||
		NULL == pCurrentData ||
		NULL == pData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	int iRet = 0;
	double_link_list_node_t *pCurrNode = 
			double_link_list_node_ptr (pCurrentData);

	if(pCurrNode->iDataLen >= iDataLen)
	{
		pCurrNode->iDataLen = iDataLen;
		base_memcpy(pCurrNode->aucData, pData, iDataLen);
	}
	else
	{
		void *pInsertData = (void *)(pCurrNode->pNext->aucData);
		iRet = _this->del(_this, (void *)pCurrNode->aucData);
		if(0 != iRet)
		{
			return iRet;
		}
		iRet = _this->insert(_this, pInsertData, pData, iDataLen);
		if(0 != iRet)
		{
			return iRet;
		}
	}
	return NoErr;
}


/*获取指定数据*/
static int 
double_link_list_get (
	double_link_list_class_t* _this,
	void* pCurrentData, 
	void* pData,
	int* piDataLen
)
{
	if(NULL == _this ||
		NULL == pCurrentData ||
		NULL == pData ||
		NULL == piDataLen)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(*piDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_node_t *pCurrNode = 
			double_link_list_node_ptr (pCurrentData);
	
	*piDataLen = (*piDataLen >= pCurrNode->iDataLen) ? pCurrNode->iDataLen : *piDataLen;
	base_memcpy(pData, pCurrNode->aucData, *piDataLen);

	return NoErr;
}



/*注册数据域释放回调，
* 如果数据域中有某一部分数据是动态分配的，
* 则需要注册释放回调接口*/
static int
double_link_list_free_data_cb_reg (
	double_link_list_class_t* _this, 
	void* pFreeDataCb
)
{
	if(NULL == _this ||
		NULL == pFreeDataCb
	)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	
	pPrivate->pFreeDataCb = pFreeDataCb;

	return NoErr;
}


/*打印数据*/
static int 
double_link_list_print (
	double_link_list_class_t* _this,
	void* pPrintDataCb
)
{
	if(NULL == _this ||
		NULL == pPrintDataCb
	)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	HY_INFO("ListLen :%d\n", pPrivate->iListLen);
	double_link_list_node_t *pNode = 
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


/*构造函数*/
double_link_list_class_t *new_double_link_list(void)
{
	
	double_link_list_class_t* pstLinkList = 
		(double_link_list_class_t *)base_calloc(1, sizeof(double_link_list_class_t));
	if(NULL == pstLinkList)
	{
		return NULL;
	}


	/*参数初始化*/
	/*私有变量初始化*/
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)pstLinkList->acPrivateParam;
	base_memset(pPrivate, 0x0, sizeof(double_link_list_private_t));

	/*初始化互斥锁*/
	pPrivate->mutex = base_mutex_lock_create();
	if(NULL == pPrivate->mutex)
	{
		HY_ERROR("Failed to create mutex lock.\n");
		base_free(pstLinkList);
		return NULL;
	}

	pstLinkList->lock = double_link_list_lock;
	pstLinkList->trylock = double_link_list_trylock;
	pstLinkList->unlock = double_link_list_unlock;
	pstLinkList->len = double_link_list_len;
	pstLinkList->head = double_link_list_head;
	pstLinkList->tail = double_link_list_tail;
	pstLinkList->next = double_link_list_next;
	pstLinkList->prev = double_link_list_prev;
	pstLinkList->find = double_link_list_find;
	pstLinkList->headInsert = double_link_list_head_insert;
	pstLinkList->tailInsert = double_link_list_tail_insert;
	pstLinkList->insert = double_link_list_insert;
	pstLinkList->del = double_link_list_del;
	pstLinkList->clear = double_link_list_clear;
	pstLinkList->set = double_link_list_set;
	pstLinkList->get = double_link_list_get;
	pstLinkList->freeDataCbReg = double_link_list_free_data_cb_reg;
	pstLinkList->print = double_link_list_print;

	return pstLinkList;
}

/*析构函数*/
int destroy_double_link_list(double_link_list_class_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	if(_this->len(_this))
	{
		/*清空链表*/
		_this->clear(_this);
	}
	
	double_link_list_private_t *pPrivate = 
		(double_link_list_private_t *)_this->acPrivateParam;
	
	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivate->mutex);

	
	base_free(_this);
	_this = NULL;
	return NoErr;
}

