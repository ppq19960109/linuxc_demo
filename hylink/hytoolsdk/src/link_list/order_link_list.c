/***********************************************************
*文件名     : order_link_list.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.14
*说   明   : 有序链表
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"
#include "link_list.h"


/*链表结点*/
typedef struct order_list_node_s
{
	struct order_list_node_s *pNext;
	struct order_list_node_s *pPrev;
	int iDataLen;
	unsigned char aucData[0];
}__attribute__ ((__packed__)) order_list_node_t;


/*私有类型定义*/
typedef struct order_list_private_s  
{  
	order_list_node_t *pHead;
	order_list_node_t *pTail;
	int iListLen;
	void *pFreeDataCb;
	/*互斥锁*/
	void *mutex;
}order_list_private_t;  

/*新建结点*/
static order_list_node_t*
order_link_list_node_new(
	order_list_class_t* _this,
	void* pData,
	int iDataLen
)
{	
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
		
	order_list_node_t* pNewNode = 
		(order_list_node_t *)base_calloc(1, sizeof(order_list_node_t) + iDataLen);
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
order_link_list_node_destroy(
	order_list_class_t* _this, 
	order_list_node_t* pNode
)
{
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
		
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
static order_list_node_t*
order_link_list_node_ptr (void* pCurrentData)
{
	return pCurrentData - sizeof(int) - sizeof(struct order_list_node_s *) - sizeof(struct order_list_node_s *);
}

/*加锁(阻塞)*/
static int 
order_link_list_lock (order_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*加锁(非阻塞)*/
static int
order_link_list_trylock (order_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*解锁*/
static int
order_link_list_unlock (order_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}

/*获取链表长度*/
static int 
order_link_list_len (order_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	return pPrivate->iListLen;
}


/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
static void* 
order_link_list_next (
	order_list_class_t* _this, 
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
		order_list_private_t *pPrivate = 
			(order_list_private_t *)_this->acPrivateParam;
		return (NULL == pPrivate->pHead) ? NULL : (void*)(pPrivate->pHead->aucData);
	}
	else
	{
		order_list_node_t *pNode = 
			order_link_list_node_ptr (pCurrentData);
		return (NULL == pNode->pNext) ? NULL : (void*)(pNode->pNext->aucData);
	}
}

/*查找数据，返回数据指针*/
static void*
order_link_list_find(
	order_list_class_t* _this,
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
		if(0 == ((CmpDataFunc)pCmpDataCb)(pCurrentData, *(int *)(pCurrentData - 4), pData, iDataLen))
		{
			return pCurrentData;
		}
	}
	error_num = NotFoundErr;
	return NULL;
}


/*获取指定下标数据，返回数据指针*/
static void* 
order_link_list_find_by_id (
	order_list_class_t* _this,
	int iIndex
)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
		return NULL;
	}
	if(iIndex < 0)
	{
		error_num = ParaErr;
		return NULL;
	}

	int iCound = 0;
	
	void* pCurrentData = NULL;
	while(NULL != (pCurrentData = _this->next(_this, pCurrentData)))
	{
		if(iCound == iIndex)
		{
			return pCurrentData;
		}
		iCound ++;
	}
	error_num = NotFoundErr;
	return NULL;
}

/*获取数据Index*/
static int 
order_link_list_index (
	order_list_class_t* _this,
	void* pCurrentData
)
{
	if(NULL == _this ||
		NULL == pCurrentData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	int iCound = 0;

	void* pData = NULL;
	while(NULL != (pData = _this->next(_this, pData)))
	{
		if(pData == pCurrentData)
		{
			return iCound;
		}
		iCound ++;
	}
	error_num = NotFoundErr;
	return GeneralErr;
}

/*当前位置入数据*/
static int
order_link_list_insert (
	order_list_class_t* _this,
	void* pCurrentData, 
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
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
	order_list_node_t *pCurrNode = 
		order_link_list_node_ptr(pCurrentData);

	if(pPrivate->pHead == pCurrNode)
	{
		order_list_node_t* pNewNode = 
			order_link_list_node_new(_this, pData, iDataLen);
		if(NULL == pNewNode)
		{
			return GeneralErr;
		}

		pNewNode->pNext = pPrivate->pHead;
		pPrivate->pHead->pPrev = pNewNode->pNext;
		pPrivate->pHead = pNewNode;
		
		if(NULL == pPrivate->pTail)
		{
			pPrivate->pTail = pNewNode;
		}
		
	}
	else
	{
		order_list_node_t* pNewNode = 
			order_link_list_node_new(_this, pData, iDataLen);
		if(NULL == pNewNode)
		{
			return GeneralErr;
		}
		pNewNode->pNext = pCurrNode;
		pNewNode->pPrev = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = pNewNode;
		pCurrNode->pPrev = pNewNode;
	}
	
	return NoErr;
}


/*插入数据*/
static int 
order_link_list_insert_by_id (
	order_list_class_t* _this, 
	int iIndex,
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

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	if(0 == iIndex)
	{
		order_list_node_t* pNewNode = 
			order_link_list_node_new(_this, pData, iDataLen);
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
	}
	else
	{
		void* pCurrentData = _this->findByIndex(_this, iIndex);
		order_list_node_t *pCurrNode = 
			order_link_list_node_ptr(pCurrentData);
		order_list_node_t* pNewNode = 
			order_link_list_node_new(_this, pData, iDataLen);
		if(NULL == pNewNode)
		{
			return GeneralErr;
		}
		pNewNode->pNext = pCurrNode;
		pNewNode->pPrev = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = pNewNode;
		pCurrNode->pPrev = pNewNode;
		
	}
	return NoErr;
}

/*追加数据*/
static int
order_link_list_append (
	order_list_class_t* _this, 
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

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;


	order_list_node_t* pNewNode = 
		order_link_list_node_new(_this, pData, iDataLen);
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
order_link_list_del (
	order_list_class_t* _this,
	void* pCurrentData
)
{
	if(NULL == _this ||
		NULL == pCurrentData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	order_list_node_t *pCurrNode = 
		order_link_list_node_ptr(pCurrentData);
	
	
	if(pPrivate->pHead == pCurrNode && pPrivate->pTail == pCurrNode)
	{
		pPrivate->pHead = NULL;
		pPrivate->pTail = NULL;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	else if(pPrivate->pHead == pCurrNode)
	{
		pPrivate->pHead = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = NULL;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	else if(pPrivate->pTail == pCurrNode)
	{
		pPrivate->pTail = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = NULL;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	else
	{
		pCurrNode->pPrev->pNext = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = pCurrNode->pPrev;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	return NoErr;
}

/*删除指定数据*/
static int 
order_link_list_del_by_id (
	order_list_class_t* _this, 
	int iIndex
)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(iIndex < 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	void* pCurrentData = _this->findByIndex(_this, iIndex);
	order_list_node_t *pCurrNode = 
		order_link_list_node_ptr(pCurrentData);
	
	
	if(pPrivate->pHead == pCurrNode)
	{
		pPrivate->pHead = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = NULL;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	else if(pPrivate->pTail == pCurrNode)
	{
		pPrivate->pTail = pCurrNode->pPrev;
		pCurrNode->pPrev->pNext = NULL;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	else
	{
		pCurrNode->pPrev->pNext = pCurrNode->pNext;
		pCurrNode->pNext->pPrev = pCurrNode->pPrev;
		order_link_list_node_destroy(_this, pCurrNode);
	}
	return NoErr;
}

/*清空数据*/
static int
order_link_list_clear (order_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
	
	order_list_node_t *pNode = 
		pPrivate->pHead;
	order_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;

		order_link_list_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}


/*修改指定数据*/
static int 
order_link_list_set (
	order_list_class_t* _this,
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
	order_list_node_t *pCurrNode = 
			order_link_list_node_ptr (pCurrentData);

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
order_link_list_get (
	order_list_class_t* _this, 
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

	order_list_node_t *pCurrNode = 
		order_link_list_node_ptr (pCurrentData);
	
	*piDataLen = (*piDataLen >= pCurrNode->iDataLen) ? pCurrNode->iDataLen : *piDataLen;
	base_memcpy(pData, pCurrNode->aucData, *piDataLen);

	return NoErr;
}


/*注册数据域释放回调，
* 如果数据域中有某一部分数据是动态分配的，
* 则需要注册释放回调接口*/
static int 
order_link_list_free_data_cb_reg (
	order_list_class_t* _this, 
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

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
	
	pPrivate->pFreeDataCb = pFreeDataCb;

	return NoErr;
}

/*打印数据*/
static int 
order_link_list_print(
	order_list_class_t* _this, 
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
	
	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;

	order_list_node_t *pNode = 
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
order_list_class_t *new_order_link_list(void)
{
	order_list_class_t* pstLinkList = 
		(order_list_class_t *)base_calloc(1, sizeof(order_list_class_t));
	if(NULL == pstLinkList)
	{
		return NULL;
	}


	/*参数初始化*/
	/*私有变量初始化*/
	order_list_private_t *pPrivate = 
		(order_list_private_t *)pstLinkList->acPrivateParam;
	base_memset(pPrivate, 0x0, sizeof(order_list_private_t));

	/*初始化互斥锁*/
	pPrivate->mutex = base_mutex_lock_create();
	if(NULL == pPrivate->mutex)
	{
		HY_ERROR("Failed to create mutex lock.\n");
		base_free(pstLinkList);
		return NULL;
	}
	
	pstLinkList->lock = order_link_list_lock;
	pstLinkList->trylock = order_link_list_trylock;
	pstLinkList->unlock = order_link_list_unlock;
	pstLinkList->len = order_link_list_len;
	pstLinkList->next = order_link_list_next;
	pstLinkList->find = order_link_list_find;
	pstLinkList->findByIndex = order_link_list_find_by_id;
	pstLinkList->index = order_link_list_index;
	pstLinkList->insert = order_link_list_insert;
	pstLinkList->insertById = order_link_list_insert_by_id;
	pstLinkList->append = order_link_list_append;
	pstLinkList->del = order_link_list_del;
	pstLinkList->delById = order_link_list_del_by_id;
	pstLinkList->clear = order_link_list_clear;
	pstLinkList->set = order_link_list_set;
	pstLinkList->get = order_link_list_get;
	pstLinkList->freeDataCbReg = order_link_list_free_data_cb_reg;
	pstLinkList->print = order_link_list_print;

	

	return pstLinkList;
}

/*析构函数*/
int destroy_order_link_list(order_list_class_t *_this)
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

	order_list_private_t *pPrivate = 
		(order_list_private_t *)_this->acPrivateParam;
	
	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivate->mutex);
	
	base_free(_this);
	_this = NULL;
	return NoErr;
}

