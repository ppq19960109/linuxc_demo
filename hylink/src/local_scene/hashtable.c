/***********************************************************
*文件名    : hashtable.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 哈希表接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "param_check.h"
#include "hashtable.h"
#include "log_api.h"

#define HASH_KEY_MAX_LEN 128

#pragma pack(1)

typedef struct hash_elem_s
{
	unsigned char ucUsed;
	char acKey[HASH_KEY_MAX_LEN];
	void *pValue;
	struct hash_elem_s *next;
}hash_elem_t;


/*私有参数定义*/
typedef struct hash_table_private_param_s  
{  
	//哈希表扩容大小
	unsigned int *pauiSizeList;
	unsigned int uiUsedCount;//元素个数
	unsigned int uiConflict;//冲突个数
	hash_elem_t *pstHashTable;//哈希头结点
}hash_table_private_param_t;  

/*私有方法定义*/
typedef struct hash_table_private_methods_s  
{  
	/*哈希函数*/
	unsigned int (*hash)(struct hash_table_class_s*, char *, int);
}hash_table_private_methods_t;  
#pragma pack()


/*************************************************************
*函数:	hash
*参数:	类指针
*		键值
*返回值:hash值
*描述:	获取key的哈希值
*************************************************************/
unsigned int hash(hash_table_class_t *_this, char *pcKey, int iSize)

{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcKey);
		
	unsigned int uiSeed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int uiHash = 0;
	while (*pcKey)
	{
		uiHash = uiHash * uiSeed + (*pcKey++);
	}
	
	return (uiHash % iSize);
}

/*判断是否为空*/
int hash_is_empty(struct hash_table_class_s *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
		
	return pPrivateParam->uiUsedCount ? 0 : 1;
}
/*获取元素个数*/
int hash_size(struct hash_table_class_s *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;

	return pPrivateParam->uiUsedCount + pPrivateParam->uiConflict;
}


/*插入数据*/
int hash_inst(hash_table_class_t *_this, char *pcKey, void *pValue)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcKey, pValue);
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *pNew =NULL;
	
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashTable;
	
	/*私有方法*/
	hash_table_private_methods_t *pDevlistPrivateMethods = 
		(hash_table_private_methods_t *)_this->acHashTablePrivateMethods;
	
	/*判断是否需要扩容,冲突个数大于hash大小的20%*/
	if(_this->ucDilatationEnable && 
		pPrivateParam->uiConflict > (unsigned int)(_this->uiHashTableSize * 0.15))
	{
		/*确定扩容大小*/
		int i = 0;
		unsigned int uiNewConflict = 0;
		unsigned int uiNewUsedCount = 0;
		hash_elem_t *p1 = NULL;
		
		for(;i < 26 && _this->uiHashTableSize >= pPrivateParam->pauiSizeList[i]; ++i);
		int iNewHashTableSize = pPrivateParam->pauiSizeList[26 == i ? i - 1 : i];
		/*扩容*/
		hash_elem_t *pNewHashTable = 
			(hash_elem_t*)calloc(iNewHashTableSize, sizeof(hash_elem_t));
		if(NULL == pNewHashTable)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		
		/*将原有哈希的所有元素添加到新的哈希表中*/
		for(i = 0; i < _this->uiHashTableSize; ++i)
		{
			if(pHash[i].ucUsed)
			{
				p = &pHash[i];
				while(p)
				{
					/*获取hash值*/
					uiHash = pDevlistPrivateMethods->hash(_this, p->acKey, iNewHashTableSize);
					/*判断是否冲突*/
					if(pNewHashTable[uiHash].ucUsed)
					{
						/*冲突处理，采用链表法*/
						/*生成新结点*/
						pNew = 
							(hash_elem_t*)calloc(1, sizeof(hash_elem_t));
						if(NULL == pNew)
						{
							HY_ERROR("Malloc error: %s\n", strerror(errno));
							return HeapReqErr;
						}
						strncpy(pNew->acKey, p->acKey, HASH_KEY_MAX_LEN - 1);
						pNew->pValue = p->pValue;
						pNew->ucUsed = 1;


						/*将新结点链接到链表尾部*/
						p1 = &pNewHashTable[uiHash];
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
						strncpy(pNewHashTable[uiHash].acKey, p->acKey, HASH_KEY_MAX_LEN - 1);
						pNewHashTable[uiHash].pValue = p->pValue;
						pNewHashTable[uiHash].ucUsed = 1;

						/*哈希表计数加一*/
						uiNewUsedCount++;
					}	
					p = p->next;
				}
			}
		}
		
		/*清空旧哈希表*/
		for(i = 0; i < _this->uiHashTableSize; ++i)
		{
			if(pHash[i].ucUsed)
			{
				p = pHash[i].next;
				while(p)
				{
					p1 = p;
					p = p->next;
					free(p1);
				}
				memset(&pHash[i], 0x0, sizeof(hash_elem_t));
			}
		}
		/*释放旧的哈希表*/
		free(pPrivateParam->pstHashTable);

		
		/*将新生成的哈希表替换旧的哈希表*/
		pPrivateParam->pstHashTable = pNewHashTable;
		pPrivateParam->uiConflict = uiNewConflict;
		pPrivateParam->uiUsedCount = uiNewUsedCount;
		_this->uiHashTableSize = iNewHashTableSize;

		
		pHash = pPrivateParam->pstHashTable;
	}

	/*获取hash值*/
	uiHash = pDevlistPrivateMethods->hash(_this, pcKey, _this->uiHashTableSize);
	/*判断是否冲突*/
	if(pHash[uiHash].ucUsed)
	{
		/*冲突处理，采用链表法*/
		/*生成新结点*/
		pNew = 
			(hash_elem_t*)calloc(1, sizeof(hash_elem_t));
		if(NULL == pNew)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		
		strncpy(pNew->acKey, pcKey, HASH_KEY_MAX_LEN - 1);
		pNew->pValue = pValue;
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
		strncpy(pHash[uiHash].acKey, pcKey, HASH_KEY_MAX_LEN - 1);
		pHash[uiHash].pValue = pValue;
		pHash[uiHash].ucUsed = 1;

		/*哈希表计数加一*/
		pPrivateParam->uiUsedCount++;
	}

	return NoErr;
}
/*删除数据*/
int hash_del(hash_table_class_t *_this, char *pcKey)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcKey);
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *p1 = NULL;
	void *pTmp = NULL;
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashTable;
	
	/*私有方法*/
	hash_table_private_methods_t *pDevlistPrivateMethods = 
		(hash_table_private_methods_t *)_this->acHashTablePrivateMethods;

	/*获取hash值*/
	uiHash = pDevlistPrivateMethods->hash(_this, pcKey, _this->uiHashTableSize);
	if(pHash[uiHash].ucUsed)
	{
		p = &pHash[uiHash];
		if(!strcmp(p->acKey, pcKey))
		{
			if(NULL == p->next)
			{
				pTmp = p->pValue;
				memset(p, 0x0, sizeof(hash_elem_t));
				/*哈希表计数减一*/
				pPrivateParam->uiUsedCount--;
			}
			else
			{
				pTmp = p->pValue;
				p1 = p->next;
				memcpy(p, p1, sizeof(hash_elem_t));
				free(p1);
				/*冲突计数减一*/
				pPrivateParam->uiConflict--;
			}
			
		}
		else
		{
			while(p->next)
			{
				if(!strcmp(p->next->acKey, pcKey))
				{
					pTmp = p->next->pValue;
					p1 = p->next;
					p->next = p->next->next;
					free(p1);
					/*冲突计数减一*/
					pPrivateParam->uiConflict--;
					break;
				}
				p = p->next;
			}
		}
	}
	if(_this->pFreeValueHandle)
	{
		_this->pFreeValueHandle(pTmp);
	}
	return NoErr;
}

/*清空数据*/
int hash_clear(hash_table_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	int i = 0;
	hash_elem_t *p = NULL;
	hash_elem_t *p1 = NULL;
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashTable;
	
	for(; i < _this->uiHashTableSize; ++i)
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
					_this->pFreeValueHandle(p1->pValue);
				}
				free(p1);
			}
			
			if(_this->pFreeValueHandle)
			{
				_this->pFreeValueHandle(pHash[i].pValue);
			}
			memset(&pHash[i], 0x0, sizeof(hash_elem_t));
		}
	}
	pPrivateParam->uiConflict = 0;
	pPrivateParam->uiUsedCount = 0;
	return NoErr;
}

/*查找数据*/
void* hash_find(hash_table_class_t *_this, char *pcKey)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pcKey);
	
	unsigned int uiHash = 0;
	hash_elem_t *p = NULL;
	void *pTmp = NULL;
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashTable;
	
	/*私有方法*/
	hash_table_private_methods_t *pDevlistPrivateMethods = 
		(hash_table_private_methods_t *)_this->acHashTablePrivateMethods;

	/*获取hash值*/
	uiHash = pDevlistPrivateMethods->hash(_this, pcKey, _this->uiHashTableSize);
	if(pHash[uiHash].ucUsed)
	{
		p = &pHash[uiHash];
		while(p)
		{
			if(!strcmp(p->acKey, pcKey))
			{
				pTmp = p->pValue;
				break;
			}
			p = p->next;
		}
	}
	return pTmp;
}

/*打印数据*/
int hash_print(hash_table_class_t *_this, PrintValueFunc pPrintValueHandle)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pPrintValueHandle);

	int i = 0;
	int iCount = 0;
	hash_elem_t *p = NULL;
	/*私有变量*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	hash_elem_t *pHash = pPrivateParam->pstHashTable;
	HY_INFO("HashTableSize = %d\n", _this->uiHashTableSize);
	HY_INFO("UsedCount = %d\n", pPrivateParam->uiUsedCount);
	HY_INFO("Conflict = %d\n", pPrivateParam->uiConflict);
	for(; i < _this->uiHashTableSize; ++i)
	{
		if(pHash[i].ucUsed)
		{
			HY_INFO("hash table index: %d\n", i);
			HY_INFO("Key : %s\n",pHash[i].acKey);
			HY_INFO("Value : ");
			pPrintValueHandle(pHash[i].pValue);
			HY_INFO("\n");
			iCount++;
			
			p = pHash[i].next;
			while(p)
			{
				HY_INFO("Key : %s\n",p->acKey);
				HY_INFO("Value : ");
				pPrintValueHandle(p->pValue);
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
hash_table_class_t *
new_hash_table(int iSize, int iDilatationEnable, FreeValueFunc pFreeValueHandle)
{

	/*申请对象*/
	int iDataSize = sizeof(hash_table_class_t);
	hash_table_class_t *pNew = (hash_table_class_t*)calloc(1, iDataSize);
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}

	/*参数赋值*/
	pNew->ucDilatationEnable = (unsigned char)iDilatationEnable;
	pNew->pFreeValueHandle = pFreeValueHandle;
	/*私有变量初始化*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)pNew->acHashTablePrivateParam;
	 
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
		pNew->uiHashTableSize = (unsigned int)iSize;
	}
	else
	{
		pNew->uiHashTableSize = pPrivateParam->pauiSizeList[0];
	}
	pPrivateParam->pstHashTable = 
		(hash_elem_t*)calloc(pNew->uiHashTableSize, sizeof(hash_elem_t));
	if(NULL == pPrivateParam->pstHashTable)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		free(pNew);
		return NULL;
	}
	/*方法赋值*/
	/*私有方法*/
	hash_table_private_methods_t *pDevlistPrivateMethods = 
		(hash_table_private_methods_t *)pNew->acHashTablePrivateMethods;
	pDevlistPrivateMethods->hash = hash;

	pNew->empty = hash_is_empty;
	pNew->size = hash_size;
	pNew->inst = hash_inst;
	pNew->del = hash_del;
	pNew->clear = hash_clear;
	pNew->find = hash_find;
	pNew->print = hash_print;

	return pNew;
}
/*析构函数*/
int 
destroy_hash_table(hash_table_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	/*私有变量初始化*/
	hash_table_private_param_t *pPrivateParam = 
		(hash_table_private_param_t *)_this->acHashTablePrivateParam;
	
	if(_this->size(_this))
	{
		/*清空hash table*/
		_this->clear(_this);
	}

	/*释放hash表*/
	free(pPrivateParam->pstHashTable);

	/*释放hash类*/
	free(_this);
	_this = NULL;
	
	return NoErr;
}


