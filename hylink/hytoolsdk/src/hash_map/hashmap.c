/***********************************************************
*文件名    : hashmap.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 哈希表接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
#include "log_api.h"
#include "error_msg.h"
#include "base_api.h"



typedef struct hash_elem_s
{
	unsigned char ucUsed;
	char acKey[HASH_KEY_MAX_LEN];
	void *pValue;
	int iValueLen;
	struct hash_elem_s *next;
}hash_elem_t;


/*私有参数定义*/
typedef struct hash_map_private_param_s  
{  
	//哈希表扩容大小
	unsigned int *pauiSizeList;
	unsigned int uiUsedCount;//元素个数
	unsigned int uiConflict;//冲突个数
	hash_elem_t *pstHashMap;//哈希头结点
	/*互斥锁*/
	void *mutex;
}hash_map_private_param_t;  

/*私有方法定义*/
typedef struct hash_map_private_methods_s  
{  
	/*哈希函数*/
	unsigned int (*hash)(
		struct hash_map_s*,
		char *,
		int
	);
}hash_map_private_methods_t;  

void * _hash_value_dup(void *pValue, int iValueLen)
{
	void *pValueDup = base_calloc(iValueLen, 1);
	if(NULL == pValueDup)
	{
		return NULL;
	}
	base_memcpy(pValueDup, pValue, iValueLen);

	return pValueDup;
}

void _hash_value_free(void *pValue)
{
	base_free(pValue);
}

/*************************************************************
*函数:	hash
*参数:	类指针
*		键值
*返回值:hash值
*描述:	获取key的哈希值
*************************************************************/
static unsigned int hash_map(
	hash_map_t *_this,
	char *pcKey,
	int iSize
)

{
	unsigned int uiSeed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int uiHash = 0;
	while (*pcKey)
	{
		uiHash = uiHash * uiSeed + (*pcKey++);
	}
	
	return (uiHash % iSize);
}

/*加锁(阻塞)*/
static int 
hash_map_lock (struct hash_map_s *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	hash_map_private_param_t *pPrivate = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*加锁(非阻塞)*/
static int
hash_map_trylock (struct hash_map_s *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	hash_map_private_param_t *pPrivate = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*解锁*/
static int
hash_map_unlock (struct hash_map_s *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	hash_map_private_param_t *pPrivate = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}


/*判断是否为空*/
int hash_map_is_empty(struct hash_map_s *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
		
	return pPrivateParam->uiUsedCount ? 0 : 1;
}
/*获取元素个数*/
int hash_map_size(struct hash_map_s *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;

	return pPrivateParam->uiUsedCount + pPrivateParam->uiConflict;
}


/*插入数据*/
int hash_map_inst(
	hash_map_t *_this, 
	char *pcKey, 
	void *pValue,
	int iValueLen
)
{
	if(NULL == _this ||
		NULL == pcKey ||
		NULL == pValue)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *pNew =NULL;
	
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashMap;
	
	/*私有方法*/
	hash_map_private_methods_t *pPrivateMethods = 
		(hash_map_private_methods_t *)_this->acHashMapPrivateMethods;
	
	/*判断是否需要扩容,冲突个数大于hash大小的20%*/
	if(_this->ucDilatationEnable && 
		pPrivateParam->uiConflict > \
		(unsigned int)(_this->uiHashMapSize * 0.15)
	)
	{
		/*确定扩容大小*/
		int i = 0;
		unsigned int uiNewConflict = 0;
		unsigned int uiNewUsedCount = 0;
		hash_elem_t *p1 = NULL;
		
		for(;
			i < 26 && \
				_this->uiHashMapSize >= pPrivateParam->pauiSizeList[i];
			++i
		);
		int iNewHashMapSize = 
			pPrivateParam->pauiSizeList[26 == i ? i - 1 : i];
		/*扩容*/
		hash_elem_t *pNewHashMap = 
			(hash_elem_t*)base_calloc(
				iNewHashMapSize,
				sizeof(hash_elem_t)
			);
		if(NULL == pNewHashMap)
		{
			return GeneralErr;
		}
		
		/*将原有哈希的所有元素添加到新的哈希表中*/
		for(i = 0; i < _this->uiHashMapSize; ++i)
		{
			if(pHash[i].ucUsed)
			{
				p = &pHash[i];
				while(p)
				{
					/*获取hash值*/
					uiHash = 
						pPrivateMethods->hash(
							_this, p->acKey, iNewHashMapSize);
					/*判断是否冲突*/
					if(pNewHashMap[uiHash].ucUsed)
					{
						/*冲突处理，采用链表法*/
						/*生成新结点*/
						pNew = 
							(hash_elem_t*)base_calloc(
								1,
								sizeof(hash_elem_t)
							);
						if(NULL == pNew)
						{
							return GeneralErr;
						}
						base_strncpy(pNew->acKey, 
							p->acKey,
							HASH_KEY_MAX_LEN - 1
						);
						
						pNew->pValue = p->pValue;
						pNew->iValueLen = p->iValueLen;
						pNew->ucUsed = 1;


						/*将新结点链接到链表尾部*/
						p1 = &pNewHashMap[uiHash];
						while(p1->next)
						{
							p1 = p1->next;
						}
						p1->next = pNew;

						/*冲突计数加一*/
						uiNewConflict++;
					}
					else
					{
						base_strncpy(
							pNewHashMap[uiHash].acKey,
							p->acKey,
							HASH_KEY_MAX_LEN - 1
						);
						pNewHashMap[uiHash].pValue = p->pValue;
						pNewHashMap[uiHash].iValueLen = p->iValueLen;
						pNewHashMap[uiHash].ucUsed = 1;

						/*哈希表计数加一*/
						uiNewUsedCount++;
					}	
					p = p->next;
				}
			}
		}
		
		/*清空旧哈希表*/
		for(i = 0; i < _this->uiHashMapSize; ++i)
		{
			if(pHash[i].ucUsed)
			{
				p = pHash[i].next;
				while(p)
				{
					p1 = p;
					p = p->next;
					base_free(p1);
				}
				base_memset(&pHash[i], 0x0, sizeof(hash_elem_t));
			}
		}
		/*释放旧的哈希表*/
		base_free(pPrivateParam->pstHashMap);

		
		/*将新生成的哈希表替换旧的哈希表*/
		pPrivateParam->pstHashMap = pNewHashMap;
		pPrivateParam->uiConflict = uiNewConflict;
		pPrivateParam->uiUsedCount = uiNewUsedCount;
		_this->uiHashMapSize = iNewHashMapSize;

		
		pHash = pPrivateParam->pstHashMap;
	}

	/*获取hash值*/
	uiHash = 
		pPrivateMethods->hash(
			_this, 
			pcKey,
			_this->uiHashMapSize
		);
	/*判断是否冲突*/
	if(pHash[uiHash].ucUsed)
	{
		/*冲突处理，采用链表法*/
		/*生成新结点*/
		pNew = 
			(hash_elem_t*)base_calloc(
				1, 
				sizeof(hash_elem_t)
			);
		if(NULL == pNew)
		{
			return GeneralErr;
		}
		
		base_strncpy(pNew->acKey, 
			pcKey, HASH_KEY_MAX_LEN - 1);
		pNew->pValue =
			_hash_value_dup(pValue, iValueLen);
		pNew->iValueLen = iValueLen;
		pNew->ucUsed = 1;


		/*将新结点链接到链表尾部*/
		p = &pHash[uiHash];
		while(p->next)
		{
			p = p->next;
		}
		p->next = pNew;

		/*冲突计数加一*/
		pPrivateParam->uiConflict++;
	}
	else
	{
		base_strncpy(pHash[uiHash].acKey,
			pcKey, HASH_KEY_MAX_LEN - 1);
		pHash[uiHash].pValue = 
			_hash_value_dup(pValue, iValueLen);
		pHash[uiHash].iValueLen = iValueLen;
		pHash[uiHash].ucUsed = 1;

		/*哈希表计数加一*/
		pPrivateParam->uiUsedCount++;
	}

	return NoErr;
}
/*删除数据*/
int hash_map_del(
	hash_map_t *_this,
	char *pcKey
)
{
	if(NULL == _this ||
		NULL == pcKey)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *p1 = NULL;
	void *pTmp = NULL;
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = 
		pPrivateParam->pstHashMap;
	
	/*私有方法*/
	hash_map_private_methods_t *pPrivateMethods = 
		(hash_map_private_methods_t *)_this->acHashMapPrivateMethods;

	/*获取hash值*/
	uiHash = 
		pPrivateMethods->hash(
			_this, 
			pcKey, 
			_this->uiHashMapSize
		);
	if(pHash[uiHash].ucUsed)
	{
		p = &pHash[uiHash];
		if(!base_strcmp(p->acKey, pcKey))
		{
			if(NULL == p->next)
			{
				pTmp = p->pValue;
				if(_this->pFreeValueHandle)
				{
					((FreeValueFunc)_this->pFreeValueHandle)(
						_this,
						pTmp,
						_this->pCbUserData
					);
				}
				_hash_value_free(pTmp);
				base_memset(p, 0x0, sizeof(hash_elem_t));
				/*哈希表计数减一*/
				pPrivateParam->uiUsedCount--;
			}
			else
			{
				pTmp = p->pValue;
				p1 = p->next;
				base_memcpy(p, p1, sizeof(hash_elem_t));
				if(_this->pFreeValueHandle)
				{
					((FreeValueFunc)_this->pFreeValueHandle)(
						_this,
						pTmp, 
						_this->pCbUserData
					);
				}
				_hash_value_free(pTmp);
				base_free(p1);
				/*冲突计数减一*/
				pPrivateParam->uiConflict--;
			}
			
		}
		else
		{
			while(p->next)
			{
				if(!base_strcmp(p->next->acKey, pcKey))
				{
					pTmp = p->next->pValue;
					p1 = p->next;
					p->next = p->next->next;
					if(_this->pFreeValueHandle)
					{
						((FreeValueFunc)_this->pFreeValueHandle)(
							_this, 
							pTmp, 
							_this->pCbUserData
						);
					}
					_hash_value_free(pTmp);
					base_free(p1);
					/*冲突计数减一*/
					pPrivateParam->uiConflict--;
					break;
				}
				p = p->next;
			}
		}
	}
	
	return NoErr;
}

/*清空数据*/
int hash_map_clear(hash_map_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	int i = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *p1 = NULL;
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashMap;
	
	for(; i < _this->uiHashMapSize; ++i)
	{
		if(pHash[i].ucUsed)
		{
			p = pHash[i].next;
			while(p)
			{
				p1 = p;
				p = p->next;
				if(_this->pFreeValueHandle)
				{
					((FreeValueFunc)_this->pFreeValueHandle)(
						_this, 
						p1->pValue,
						_this->pCbUserData
					);
				}
				_hash_value_free(p1->pValue);
				base_free(p1);
			}
			
			if(_this->pFreeValueHandle)
			{
				((FreeValueFunc)_this->pFreeValueHandle)(
					_this,
					pHash[i].pValue,
					_this->pCbUserData
				);
			}
			_hash_value_free(pHash[i].pValue);
			base_memset(&pHash[i], 0x0, sizeof(hash_elem_t));
		}
	}
	pPrivateParam->uiConflict = 0;
	pPrivateParam->uiUsedCount = 0;
	return NoErr;
}

/*查找数据*/
void* hash_map_find(hash_map_t *_this, char *pcKey)
{
	if(NULL == _this ||
		NULL == pcKey)
	{
		error_num = ParaErr;
        return NULL;
	}
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	void *pTmp = NULL;
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashMap;
	
	/*私有方法*/
	hash_map_private_methods_t *pPrivateMethods = 
		(hash_map_private_methods_t *)_this->acHashMapPrivateMethods;

	/*获取hash值*/
	uiHash = 
		pPrivateMethods->hash(
			_this, 
			pcKey, 
			_this->uiHashMapSize
		);
	
	if(pHash[uiHash].ucUsed)
	{
		p = &pHash[uiHash];
		while(p)
		{
			if(!base_strcmp(p->acKey, pcKey))
			{
				pTmp = p->pValue;
				break;
			}
			p = p->next;
		}
	}
	
	return pTmp;
}

/*查找数据*/
int hash_map_find2(
	hash_map_t*_this, 
	char *pcKey, 
	void *pValue,
	int *piValueLen
)
{
	if(NULL == _this ||
		NULL == pcKey ||
		NULL == pValue ||
		NULL == piValueLen)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	if(*piValueLen <= 0)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	void *pTmp = NULL;
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashMap;
	
	/*私有方法*/
	hash_map_private_methods_t *pPrivateMethods = 
		(hash_map_private_methods_t *)_this->acHashMapPrivateMethods;

	/*获取hash值*/
	uiHash = 
		pPrivateMethods->hash(
			_this, 
			pcKey, 
			_this->uiHashMapSize
		);
	
	if(pHash[uiHash].ucUsed)
	{
		p = &pHash[uiHash];
		while(p)
		{
			if(!base_strcmp(p->acKey, pcKey))
			{
				pTmp = p->pValue;
				break;
			}
			p = p->next;
		}
	}
	if(NULL != pTmp)
	{
		*piValueLen = *piValueLen >= p->iValueLen ? p->iValueLen : *piValueLen;
		base_memcpy((char*)pValue, pTmp, *piValueLen);
		return NoErr;
	}
	else
	{
		error_num = NotFoundErr;
		return GeneralErr;
	}
	
}
/*注册哈希数据域，内存回收回调*/
int hash_map_data_free_cb_reg(
	hash_map_t*_this, 
	void *pFun,
	void *pUserData
)
{
	if(NULL == _this ||
		NULL == pFun ||
		NULL == pUserData)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	_this->pFreeValueHandle = pFun;
	_this->pCbUserData = pUserData;
	return NoErr;
}

/*打印数据*/
int hash_map_print(
	hash_map_t *_this, 
	void* pPrintValueHandle
)
{
	if(NULL == _this ||
		NULL == pPrintValueHandle)
	{
		error_num = ParaErr;
        return GeneralErr;
	}

	int i = 0;
	int iCount = 0;
	hash_elem_t *p = NULL;
	/*私有变量*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashMap;
	HY_INFO("HashTableSize = %d\n", _this->uiHashMapSize);
	HY_INFO("UsedCount = %d\n", pPrivateParam->uiUsedCount);
	HY_INFO("Conflict = %d\n", pPrivateParam->uiConflict);
	for(; i < _this->uiHashMapSize; ++i)
	{
		if(pHash[i].ucUsed)
		{
			HY_INFO("hash table index: %d\n", i);
			HY_INFO("Key : %s\n",pHash[i].acKey);
			HY_INFO("Value : ");
			((PrintValueFunc)pPrintValueHandle)(pHash[i].pValue);
			HY_INFO("\n");
			iCount++;
			
			p = pHash[i].next;
			while(p)
			{
				HY_INFO("Key : %s\n",p->acKey);
				HY_INFO("Value : ");
				((PrintValueFunc)pPrintValueHandle)(p->pValue);
				HY_INFO("\n");
				iCount++;
				p = p->next;
			}
		}
	}
	HY_INFO("total: %d\n",iCount);
	return NoErr;
}



/*构造函数*/
hash_map_t *
new_hash_map(int iSize, int iDilatationEnable)
{

	/*申请对象*/
	int iDataSize = sizeof(hash_map_t);
	hash_map_t *pNew = 
		(hash_map_t*)base_calloc(1, iDataSize);
	if(NULL == pNew)
	{
		return NULL;
	}

	/*参数赋值*/
	pNew->ucDilatationEnable = (unsigned char)iDilatationEnable;
	
	/*私有变量初始化*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)pNew->acHashMapPrivateParam;
	 
	static unsigned int uiTmp[] = {
			53,			97,				193,		389,		769,
			1543,		3079,			6151,		12289,		24593,
			49157,		98317,			196613,		393241,		786443,
			1572869,	3145739,		6291469,	12582917,	25165842,
			50331553,	100663319,		201326611,	402653189,	805306457,
			1610612741
	};

	pPrivateParam->pauiSizeList = uiTmp;
	pPrivateParam->uiConflict = 0;
	pPrivateParam->uiUsedCount = 0;
	
	if(iSize > 0)
	{
		pNew->uiHashMapSize = (unsigned int)iSize;
	}
	else
	{
		pNew->uiHashMapSize = pPrivateParam->pauiSizeList[0];
	}
	pPrivateParam->pstHashMap = 
		(hash_elem_t*)base_calloc(
			pNew->uiHashMapSize, 
			sizeof(hash_elem_t)
		);
	if(NULL == pPrivateParam->pstHashMap)
	{
		base_free(pNew);
		return NULL;
	}
	
	/*初始化互斥锁*/
	pPrivateParam->mutex = base_mutex_lock_create();
	if(NULL == pPrivateParam->mutex)
	{
		HY_ERROR("Failed to create mutex lock.\n");
		base_free(pPrivateParam->pstHashMap);
		base_free(pNew);
		return NULL;
	}
	
	
	/*方法赋值*/
	/*私有方法*/
	hash_map_private_methods_t *pPrivateMethods = 
		(hash_map_private_methods_t *)pNew->acHashMapPrivateMethods;
	pPrivateMethods->hash = hash_map;

	pNew->lock = hash_map_lock;
	pNew->trylock = hash_map_trylock;
	pNew->unlock = hash_map_unlock;
	pNew->empty = hash_map_is_empty;
	pNew->size = hash_map_size;
	pNew->inst = hash_map_inst;
	pNew->del = hash_map_del;
	pNew->clear = hash_map_clear;
	pNew->find = hash_map_find;
	pNew->find2 = hash_map_find2;
	pNew->print = hash_map_print;
	pNew->free_cb_reg = hash_map_data_free_cb_reg;
	return pNew;
}
/*析构函数*/
int 
destroy_hash_map(hash_map_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return GeneralErr;
	}
	
	/*私有变量初始化*/
	hash_map_private_param_t *pPrivateParam = 
		(hash_map_private_param_t *)_this->acHashMapPrivateParam;
	
	if(_this->size(_this))
	{
		/*清空hash table*/
		_this->clear(_this);
	}

	/*释放hash表*/
	base_free(pPrivateParam->pstHashMap);
	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivateParam->mutex);
	/*释放hash类*/
	base_free(_this);
	_this = NULL;
	
	return NoErr;
}


