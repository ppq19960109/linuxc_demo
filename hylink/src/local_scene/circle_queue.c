/***********************************************************
*文件名    : circle_queue.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 环形队列接口
*修改记录: 
************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "param_check.h"
#include "circle_queue.h"

#pragma pack(1)

/*私有参数定义*/
typedef struct queue_private_param_s  
{  
	int iCount;//队列元素个数
	int iFront;//队头指针，队列中第一个元素的下标
	int iRear;//队尾指针，当前可以存放数据的下标
	void **Elem;//数据域
} queue_private_param_t;  
#pragma pack()


/*队列最大长度*/
int circle_queue_size(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->iSize;
}
/*队列长度*/
int circle_queue_len(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	/*私有变量*/
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	return pPrivateParam->iCount;
}

/*队列是否为空*/
int circle_queue_is_empty(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	
	return pPrivateParam->iCount ? 0 : 1;
}

/*队列是否已满*/
int circle_queue_is_full(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	return _this->iSize == pPrivateParam->iCount ? 1 : 0;
}

/*出队*/
void* circle_queue_pop(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	void *pTmp = NULL;
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	/*获取数据*/
	if(1 == _this->empty(_this))
	{
		//ERROR("The queue is empty\n");
		return NULL;
	}
	pTmp = pPrivateParam->Elem[pPrivateParam->iFront];
	
	/*删除*/
	pPrivateParam->iFront = (pPrivateParam->iFront + 1) % _this->iSize;

	/*队列计数减一*/
	pPrivateParam->iCount --;
	
	return pTmp;
}

/*入队*/
int circle_queue_push(circle_queue_class_t *_this, void *pValue)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pValue);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;

	if(1 == _this->full(_this))
	{
		/*如果队列已满，则出队队首*/
		void *pTmp = _this->pop(_this);
		if(pTmp && _this->ucFreeValueEnable)
		{
			free(pTmp);
		}
	}
	pPrivateParam->Elem[pPrivateParam->iRear] = pValue;
	pPrivateParam->iRear = (pPrivateParam->iRear + 1) % _this->iSize;

	/*队列计数加一*/
	pPrivateParam->iCount ++;

	return NoErr;
}

/*清空队列*/
int circle_queue_clear(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	void *pTmp = NULL;
	while(NULL != (pTmp = _this->pop(_this)))
	{
		if(_this->ucFreeValueEnable)
		{
			free(pTmp);
		}
	}

	return NoErr;
}

/*获取队头元素，但不出队*/
void* circle_queue_get_top(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	if(1 == _this->empty(_this))
	{
		HY_ERROR("The queue is empty\n");
		return NULL;
	}
	return pPrivateParam->Elem[pPrivateParam->iFront];
}

/*获取队尾元素，但不出队*/
void* circle_queue_get_foot(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	if(1 == _this->empty(_this))
	{
		HY_ERROR("The queue is empty\n");
		return NULL;
	}
	if(pPrivateParam->iFront < pPrivateParam->iRear)
	{
		return pPrivateParam->Elem[pPrivateParam->iRear - 1];
	}
	else
	{
		return pPrivateParam->Elem[pPrivateParam->iRear + 1];
	}
	
}

/*获取指定id的元素，但不出队*/
void* circle_queue_get_by_id(circle_queue_class_t *_this, int iIndex)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_NULL_1(iIndex);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	int i = 0;
	int iCount = 0;
	void *pTmp = NULL;
	if(1 == _this->empty(_this))
	{
		HY_ERROR("The queue is empty\n");
		return NULL;
	}
	if(iIndex >= _this->len(_this))
	{
		HY_ERROR("The id is not found\n");
		return NULL;
	}
	
	if(pPrivateParam->iFront < pPrivateParam->iRear)
	{
		for(i = pPrivateParam->iFront; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			if(iIndex == iCount)
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
	}
	else
	{
		for(i = pPrivateParam->iFront; i < _this->iSize; ++i, ++iCount)
		{
			if(iIndex == iCount)
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
		for(i = 0; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			if(iIndex == iCount)
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
	}

	return pTmp;
}

/*获取指定id的元素，但不出队*/
void* circle_queue_get(circle_queue_class_t *_this, CmpValueFunc pCmpValueHandle, void *pCmpValue)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pCmpValueHandle, pCmpValue);

	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	int i = 0;
	int iCount = 0;
	void *pTmp = NULL;
	if(1 == _this->empty(_this))
	{
		HY_ERROR("The queue is empty\n");
		return NULL;
	}
	
	if(pPrivateParam->iFront < pPrivateParam->iRear)
	{
		for(i = pPrivateParam->iFront; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			
			if(0 == pCmpValueHandle(pPrivateParam->Elem[i], pCmpValue))
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
	}
	else
	{
		for(i = pPrivateParam->iFront; i < _this->iSize; ++i, ++iCount)
		{
			if(0 == pCmpValueHandle(pPrivateParam->Elem[i], pCmpValue))
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
		for(i = 0; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			if(0 == pCmpValueHandle(pPrivateParam->Elem[i], pCmpValue))
			{
				pTmp = pPrivateParam->Elem[i];
				break;
			}
		}
	}

	return pTmp;
}

/*打印队列，仅用于调试*/
int circle_queue_print(circle_queue_class_t *_this, PrintValueFunc pPrintValueHandle)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pPrintValueHandle);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	int i = 0;
	int iCount = 0;
	if(1 == _this->empty(_this))
	{
		HY_INFO("The Total : %d\n", 0);
		return NoErr;
	}
	HY_INFO("The Total : %d\n", pPrivateParam->iCount);
	if(pPrivateParam->iFront < pPrivateParam->iRear)
	{
		for(i = pPrivateParam->iFront; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			HY_INFO("Id: %d\n", iCount);
			HY_INFO("Value: ");
			pPrintValueHandle(pPrivateParam->Elem[i]);
			HY_INFO("\n");
		}
	}
	else
	{
		for(i = pPrivateParam->iFront; i < _this->iSize; ++i, ++iCount)
		{
			HY_INFO("Id: %d\n", iCount);
			HY_INFO("Value: ");
			pPrintValueHandle(pPrivateParam->Elem[i]);
			HY_INFO("\n");
		}
		for(i = 0; i < pPrivateParam->iRear; ++i, ++iCount)
		{
			HY_INFO("Id: %d\n", iCount);
			HY_INFO("Value: ");
			pPrintValueHandle(pPrivateParam->Elem[i]);
			HY_INFO("\n");
		}
	}

	return NoErr;
}


/*构造函数*/
circle_queue_class_t *new_circle_queue(int iSize, int iFreeValueEnable)
{
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_NULL_1(iSize);
	int iDataSize = sizeof(circle_queue_class_t);
	circle_queue_class_t *pNew = (circle_queue_class_t*)calloc(1, iDataSize);
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}

	/*参数赋值*/
	pNew->iSize = iSize;
	pNew->ucFreeValueEnable = (unsigned char)iFreeValueEnable;
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)pNew->acQueuePrivateParam;
	pPrivateParam->Elem = (void**)calloc(pNew->iSize, sizeof(void*));
	if(NULL == pPrivateParam->Elem)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		free(pNew);
		return NULL;
	}
	pPrivateParam->iCount = 0;
	pPrivateParam->iFront = 0;
	pPrivateParam->iRear = 0;

	/*方法赋值*/
	pNew->size = circle_queue_size;
	pNew->len = circle_queue_len;
	pNew->empty = circle_queue_is_empty;
	pNew->full = circle_queue_is_full;
	pNew->pop = circle_queue_pop;
	pNew->push = circle_queue_push;
	pNew->clear = circle_queue_clear;
	pNew->top = circle_queue_get_top;
	pNew->foot = circle_queue_get_foot;
	pNew->get_by_id = circle_queue_get_by_id;
	pNew->get = circle_queue_get;
	pNew->print = circle_queue_print;


	return pNew;
}
/*析构函数*/
int destroy_circle_queue(circle_queue_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	queue_private_param_t *pPrivateParam = 
		(queue_private_param_t *)_this->acQueuePrivateParam;
	/*清空队列*/
	if(_this->len(_this) > 0)
	{
		_this->clear(_this);
	}

	/*释放队列缓存*/
	free(pPrivateParam->Elem);
	
	/*释放类*/
	free(_this);
	_this = NULL;

	return NoErr;
}


