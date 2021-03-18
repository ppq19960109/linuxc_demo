/***********************************************************
*文件名     : simple_link_list.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.14
*说   明   : 简单链表类
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
typedef struct simple_link_list_node_s
{
	struct simple_link_list_node_s *pNext;
	int iDataLen;
	unsigned char aucData[0];
} __attribute__ ((__packed__)) simple_link_list_node_t;


/*私有类型定义*/
typedef struct simple_link_list_private_s  
{  
	simple_link_list_node_t *pHead;
	simple_link_list_node_t *pTail;
	int iListLen;
	void *pFreeDataCb;
	/*互斥锁*/
	void *mutex;
}simple_link_list_private_t;  

/*新建结点*/
static simple_link_list_node_t*
simple_link_list_node_new(
	simple_link_list_class_t* _this,
	void* pData,
	int iDataLen
)
{	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
		
	simple_link_list_node_t* pNewNode = 
		(simple_link_list_node_t *)base_calloc(1, sizeof(simple_link_list_node_t) + iDataLen);
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
simple_link_list_node_destroy(
	simple_link_list_class_t* _this, 
	simple_link_list_node_t* pNode
)
{
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
		
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
static simple_link_list_node_t*
simple_link_list_node_ptr (void* pCurrentData)
{
	return pCurrentData - sizeof(int) - sizeof(struct simple_link_list_node_s *);
}

/*加锁(阻塞)*/
static int 
simple_link_list_lock (simple_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*加锁(非阻塞)*/
static int
simple_link_list_trylock (simple_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*解锁*/
static int
simple_link_list_unlock (simple_link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}


/*获取链表长度*/
static int 
simple_link_list_len (simple_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->iListLen;
}

/*获取链表头数据指针*/
static void* simple_link_list_head (simple_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return NULL;
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->pHead->aucData;
}

/*获取链表尾数据指针*/
static void* simple_link_list_tail (simple_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return NULL;
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	return pPrivate->pTail->aucData;
}

/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
static void*
simple_link_list_next (
	simple_link_list_class_t* _this, 
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
		simple_link_list_private_t *pPrivate = 
			(simple_link_list_private_t *)_this->acPrivateParam;
		return (NULL == pPrivate->pHead) ? NULL : (void*)(pPrivate->pHead->aucData);
	}
	else
	{
		simple_link_list_node_t *pNode = 
			simple_link_list_node_ptr (pCurrentData);
		return (NULL == pNode->pNext) ? NULL : (void*)(pNode->pNext->aucData);
	}
}

/*查找数据，返回数据指针*/
static void* 
simple_link_list_find (
	simple_link_list_class_t* _this,
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


/*头插入数据*/
static int simple_link_list_head_insert (simple_link_list_class_t* _this, void* pData, int iDataLen)
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

	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;


	simple_link_list_node_t* pNewNode = 
		simple_link_list_node_new(_this, pData, iDataLen);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}

	pNewNode->pNext = pPrivate->pHead;
	pPrivate->pHead = pNewNode;
	
	if(NULL == pPrivate->pTail)
	{
		pPrivate->pTail = pNewNode;
	}
	
	return NoErr;
}

/*尾插入数据*/
static int simple_link_list_tail_insert (simple_link_list_class_t* _this, void* pData, int iDataLen)
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
        return ParaErr;
	}

	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;


	simple_link_list_node_t* pNewNode = 
		simple_link_list_node_new(_this, pData, iDataLen);
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
		pPrivate->pTail = pNewNode;
	}
		
	return NoErr;
}


/*删除指定数据*/
static int simple_link_list_del (simple_link_list_class_t* _this, void* pCurrentData)
{
	if(NULL == _this ||
		NULL == pCurrentData)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	simple_link_list_node_t *pCurrNode = 
		simple_link_list_node_ptr(pCurrentData);
	
	
	if(pPrivate->pHead == pCurrNode)
	{
		pPrivate->pHead = pCurrNode->pNext;
		if(pPrivate->pTail == pCurrNode)
		{
			pPrivate->pTail = NULL;
		}
		simple_link_list_node_destroy(_this, pCurrNode);
		
	}
	else
	{
		simple_link_list_node_t *pNode = 
			pPrivate->pHead;
		while(pNode && pNode->pNext)
		{
			if(pNode->pNext == pCurrNode)
			{
				if(pPrivate->pTail == pCurrNode)
				{
					pPrivate->pTail = pNode;
				}
				pNode->pNext = pCurrNode->pNext;
				simple_link_list_node_destroy(_this, pCurrNode);
				return NoErr;
			}
			pNode = pNode->pNext;
		}
	}
	return -1;
}

/*清空数据*/
static int simple_link_list_clear (simple_link_list_class_t* _this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
	
	simple_link_list_node_t *pNode = 
		pPrivate->pHead;
	simple_link_list_node_t *pTmpNode = NULL;
	while(pNode)
	{
		pTmpNode = pNode;
		pNode = pNode->pNext;
		simple_link_list_node_destroy(_this, pTmpNode);
	}

	return NoErr;
}


/*修改指定数据*/
static int 
simple_link_list_set (
	simple_link_list_class_t* _this, 
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
        return ParaErr;
	}
	if(iDataLen <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	simple_link_list_node_t *pCurrNode = 
			simple_link_list_node_ptr (pCurrentData);

	if(pCurrNode->iDataLen >= iDataLen)
	{
		pCurrNode->iDataLen = iDataLen;
		base_memcpy(pCurrNode->aucData, pData, iDataLen);
	}
	else
	{
		simple_link_list_private_t *pPrivate = 
			(simple_link_list_private_t *)_this->acPrivateParam;
	
		simple_link_list_node_t* pNewNode = 
			simple_link_list_node_new(_this, pData, iDataLen);
		if(NULL == pNewNode)
		{
			return GeneralErr;
		}
		pNewNode->pNext = pCurrNode->pNext;
		
		if(pPrivate->pHead == pCurrNode)
		{
			pPrivate->pHead = pNewNode;
			simple_link_list_node_destroy(_this, pCurrNode);
		}
		else
		{
			simple_link_list_node_t *pNode = 
				pPrivate->pHead;
			while(pNode && pNode->pNext)
			{
				if(pNode->pNext == pCurrNode)
				{
					if(pPrivate->pTail == pCurrNode)
					{
						pPrivate->pTail = pNewNode;
					}
					pNode->pNext = pNewNode;
					simple_link_list_node_destroy(_this, pCurrNode);
				}
				pNode = pNode->pNext;
			}
		}
	}
	return NoErr;
}


/*获取指定数据*/
static int 
simple_link_list_get (
	simple_link_list_class_t* _this,
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
        return ParaErr;
	}
	if(*piDataLen <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	simple_link_list_node_t *pCurrNode = 
			simple_link_list_node_ptr (pCurrentData);
	
	*piDataLen = (*piDataLen >= pCurrNode->iDataLen) ? pCurrNode->iDataLen : *piDataLen;
	base_memcpy(pData, pCurrNode->aucData, *piDataLen);

	return NoErr;
}



/*注册数据域释放回调，
* 如果数据域中有某一部分数据是动态分配的，
* 则需要注册释放回调接口*/
static int 
simple_link_list_free_data_cb_reg (
	simple_link_list_class_t* _this, 
	void* pFreeDataCb
)
{
	if(NULL == _this ||
		NULL == pFreeDataCb
	)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
	
	pPrivate->pFreeDataCb = pFreeDataCb;

	return NoErr;
}


/*打印数据*/
static int 
simple_link_list_print(
	simple_link_list_class_t* _this,
	void* pPrintDataCb
)
{
	if(NULL == _this ||
		NULL == pPrintDataCb
	)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;

	simple_link_list_node_t *pNode = 
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
simple_link_list_class_t *new_simple_link_list(void)
{
	simple_link_list_class_t* pstLinkList = 
		(simple_link_list_class_t *)base_calloc(1, sizeof(simple_link_list_class_t));
	if(NULL == pstLinkList)
	{
        return NULL;
	}


	/*参数初始化*/
	/*私有变量初始化*/
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)pstLinkList->acPrivateParam;
	base_memset(pPrivate, 0x0, sizeof(simple_link_list_private_t));

	/*初始化互斥锁*/
	pPrivate->mutex = base_mutex_lock_create();
	if(NULL == pPrivate->mutex)
	{
		HY_ERROR("Failed to create mutex lock.\n");
		base_free(pstLinkList);
		return NULL;
	}
	
	/*初始化函数指针*/
	pstLinkList->lock = simple_link_list_lock;
	pstLinkList->trylock = simple_link_list_trylock;
	pstLinkList->unlock = simple_link_list_unlock;
	pstLinkList->len = simple_link_list_len;
	pstLinkList->head = simple_link_list_head;
	pstLinkList->tail = simple_link_list_tail;
	pstLinkList->next = simple_link_list_next;
	pstLinkList->find = simple_link_list_find;
	pstLinkList->headInsert = simple_link_list_head_insert;
	pstLinkList->tailInsert = simple_link_list_tail_insert;
	pstLinkList->del = simple_link_list_del;
	pstLinkList->clear = simple_link_list_clear;
	pstLinkList->set = simple_link_list_set;
	pstLinkList->get = simple_link_list_get;
	pstLinkList->freeDataCbReg = simple_link_list_free_data_cb_reg;
	pstLinkList->print = simple_link_list_print;

	return pstLinkList;
}


/*析构函数*/
int destroy_simple_link_list(simple_link_list_class_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	if(_this->len(_this))
	{
		/*清空链表*/
		_this->clear(_this);
	}
	
	simple_link_list_private_t *pPrivate = 
		(simple_link_list_private_t *)_this->acPrivateParam;
	
	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivate->mutex);
	
	base_free(_this);
	_this = NULL;
	return NoErr;
}



