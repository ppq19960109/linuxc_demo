#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "base_api.h"

/*堆空间记录*/
typedef struct heap_node_s
{
	struct heap_node_s *pNext;

	
	void *pPtr;
	unsigned int uiSize;
	/*时间戳*/
	base_timeval_t stTimeStamp;
}heap_node_t;

/*堆空间链表*/
typedef struct heap_list_s{
	heap_node_t *pHead;
	heap_node_t *pTail;
	unsigned long int ulHeapUsage;
	int iHeapListCount;
} heap_list_t;

/*堆空间申请记录链表*/
heap_list_t *g_pstHeapList = NULL;
/*链表锁*/
void *g_pHeapListMutexLock = NULL;

static heap_node_t*
_heap_node_new(
	void *pPtr,
	unsigned int uiSize
)
{	
	heap_node_t* pNewNode = 
		(heap_node_t *)calloc(1, sizeof(heap_node_t));
	if(NULL == pNewNode)
	{
		error_num = HeapReqErr;
		return NULL;
	}
	pNewNode->pPtr = pPtr;
	pNewNode->uiSize = uiSize;
	return pNewNode;
}
/*释放结点*/
static int
_heap_node_destroy(
	heap_node_t* pNode
)
{
	free(pNode);
	pNode = NULL;
	
	return NoErr;
}

/*尾插入记录*/
static int _heap_node_tail_insert (
	heap_list_t *pstHeapList, 
	void *pPtr, 
	unsigned int uiSize
)
{
	if(NULL == g_pHeapListMutexLock)
	{
		g_pHeapListMutexLock = base_mutex_lock_create();
		if(NULL == g_pHeapListMutexLock)
		{
			return GeneralErr;
		}
	}
	
	heap_node_t* pNewNode = 
		_heap_node_new(pPtr, uiSize);
	if(NULL == pNewNode)
	{
		return GeneralErr;
	}	

	/*加锁*/
	base_mutex_lock(g_pHeapListMutexLock);
	
	if(NULL == pstHeapList->pTail)
	{
		pstHeapList->pHead = pNewNode;
		pstHeapList->pTail = pNewNode;
	}
	else
	{
		pstHeapList->pTail->pNext = pNewNode;
		pstHeapList->pTail = pNewNode;
	}

	pstHeapList->iHeapListCount ++;
	pstHeapList->ulHeapUsage += uiSize;

	/*解锁*/
	base_mutex_unlock(g_pHeapListMutexLock);
	return NoErr;
}

/*删除指定数据*/
static int _heap_node_del (
	heap_list_t *pstHeapList,
	void *pPtr
)
{
	int iRet = NotFoundErr;
	
	/*加锁*/
	base_mutex_lock(g_pHeapListMutexLock);
	

	heap_node_t *pCurrNode = 
		pstHeapList->pHead;
	if(NULL == pCurrNode)
	{
		goto _heap_node_del_end;
	}

	/*要删除的结点是头结点*/
	if(pPtr == pCurrNode->pPtr)
	{
		pstHeapList->pHead = pCurrNode->pNext;
		if(pstHeapList->pTail == pCurrNode)
		{
			pstHeapList->pTail = NULL;
		}
		pstHeapList->iHeapListCount --;
		pstHeapList->ulHeapUsage -= pCurrNode->uiSize;
		_heap_node_destroy(pCurrNode);

		iRet = NoErr;
		goto _heap_node_del_end;
	}
	heap_node_t *pDelNode = NULL;
	while(pCurrNode && pCurrNode->pNext)
	{
		if(pPtr == pCurrNode->pNext->pPtr)
		{
			pDelNode = pCurrNode->pNext;
			if(pstHeapList->pTail == pDelNode)
			{
				pstHeapList->pTail = pCurrNode;
			}
			pCurrNode->pNext = pDelNode->pNext;
			pstHeapList->iHeapListCount --;
			pstHeapList->ulHeapUsage -= pDelNode->uiSize;
			_heap_node_destroy(pDelNode);
			iRet = NoErr;
			goto _heap_node_del_end;
		}
	
		pCurrNode = pCurrNode->pNext;
	}
	

_heap_node_del_end:
	/*解锁*/
	base_mutex_unlock(g_pHeapListMutexLock);
	
	return iRet;
}

extern void* _base_calloc(unsigned int num, unsigned int size, const char *pfile, const char *pfun, int line)
{
	int iRet = 0;
	/*初始化堆申请记录链表*/
	if(NULL == g_pstHeapList)
	{
		g_pstHeapList = calloc(1, sizeof(heap_list_t));
		if(NULL == g_pstHeapList)
		{
			error_num = HeapReqErr;
			return NULL;
		}
	}
	
	void* ptr = calloc(num, size);
	if(NULL == ptr)
	{
		error_num = HeapReqErr;
		return NULL;
	}
	else
	{
		
		/*将申请成功的堆空间记录下来*/
		iRet = _heap_node_tail_insert (
			g_pstHeapList, 
			ptr, 
			num * size
		);
		if(NoErr != iRet)
		{
			free(ptr);
			error_num = iRet;
			return NULL;
		}
#ifdef MEM_DEBUG		
		printf("[%s:%s:%d]calloc mem = %p, size = %d\n", pfile, pfun, line, ptr, num * size);
#endif
		return ptr;
	}
}
extern void* _base_malloc(unsigned int size, const char *pfile, const char *pfun, int line)
{
	int iRet = 0;
	void* ptr = malloc(size);
	if(NULL == ptr)
	{
		error_num = HeapReqErr;
		return NULL;
	}
	else
	{
		/*将申请成功的堆空间记录下来*/
		iRet = _heap_node_tail_insert (
			g_pstHeapList, 
			ptr, 
			size
		);
		if(NoErr != iRet)
		{
			free(ptr);
			error_num = iRet;
			return NULL;
		}
#ifdef MEM_DEBUG
		printf("[%s:%s:%d]malloc mem = %p, size = %d\n", pfile, pfun, line, ptr, size);
#endif

		return ptr;
	}
}

extern void* _base_realloc(void *mem, unsigned int newsize, const char *pfile, const char *pfun, int line)
{
	int iRet = 0;
	
	/*初始化堆申请记录链表*/
	if(NULL == g_pstHeapList)
	{
		g_pstHeapList = calloc(1, sizeof(heap_list_t));
		if(NULL == g_pstHeapList)
		{
			error_num = HeapReqErr;
			return NULL;
		}
	}
	
	/*删除堆栈申请记录*/
	iRet = _heap_node_del (
		g_pstHeapList,
		 mem
	);
	if(NoErr != iRet)
	{
 		return NULL;
	}
	
#ifdef MEM_DEBUG
	printf("[%s:%s:%d]free mem = %p\n", pfile, pfun, line, mem);
#endif

	void* ptr = realloc(mem, newsize);
	if(NULL == ptr)
	{
		error_num = HeapReqErr;
		return NULL;
	}
	else
	{
		
		/*将申请成功的堆空间记录下来*/
		iRet = _heap_node_tail_insert (
			g_pstHeapList, 
			ptr, 
			newsize
		);
		if(NoErr != iRet)
		{
			free(ptr);
			error_num = iRet;
			return NULL;
		}
#ifdef MEM_DEBUG
		printf("[%s:%s:%d]malloc mem = %p, size = %d\n", pfile, pfun, line, ptr, newsize);
#endif
		return ptr;
	}
}

extern void _base_free(void *mem, const char *pfile, const char *pfun, int line)
{
#ifdef MEM_DEBUG
	printf("[%s:%s:%d]free mem = %p\n", pfile, pfun, line, mem);
#endif

	int iRet = 0;
	
	/*初始化堆申请记录链表*/
	if(NULL == g_pstHeapList)
	{
		g_pstHeapList = calloc(1, sizeof(heap_list_t));
		if(NULL == g_pstHeapList)
		{
			error_num = HeapReqErr;
			return ;
		}
	}

	/*删除堆栈申请记录*/
	iRet = _heap_node_del (
		g_pstHeapList,
		 mem
	);
	
	if(NoErr != iRet)
	{
 		return ;
	}

	free(mem);
	mem = NULL;

	return;
}

extern int base_heap_usage(void)
{
	/*初始化堆申请记录链表*/
	if(NULL == g_pstHeapList)
	{
		g_pstHeapList = calloc(1, sizeof(heap_list_t));
		if(NULL == g_pstHeapList)
		{
			error_num = HeapReqErr;
			return GeneralErr;
		}
	}

	return g_pstHeapList->ulHeapUsage;
}

extern int base_heap_usage_printf(void)
{
	/*初始化堆申请记录链表*/
	if(NULL == g_pstHeapList)
	{
		g_pstHeapList = calloc(1, sizeof(heap_list_t));
		if(NULL == g_pstHeapList)
		{
			error_num = HeapReqErr;
			return GeneralErr;
		}
	}

	
	heap_node_t* pCurrNode = g_pstHeapList->pHead;
	
	while(pCurrNode)
	{
		printf("Size = %d\n",  pCurrNode->uiSize);
		printf("pPtr = %p\n",  pCurrNode->pPtr);
		pCurrNode = pCurrNode->pNext;
	}

	
	return NoErr;
}


