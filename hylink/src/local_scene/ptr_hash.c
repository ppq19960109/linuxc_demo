/***********************************************************
*文件名    : ptr_hash.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.21
*说   明   : 指针哈希表接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "param_check.h"
#include "ptr_hash.h"

int print_ptr_value(void *pHashValue)
{
	ptr_node_t *pValueList = (ptr_node_t *)pHashValue;
	ptr_node_t *node = pValueList->next;

	while(node)
	{
		HY_INFO("Id = %s\n", node->stCondPtr.acId);
		node = node->next;
	}
	return NoErr;
}

int free_hash_value(void* pHashValue)
{
	ptr_node_t *head = (ptr_node_t *)pHashValue;
	ptr_node_t *pTmp = NULL;

	while(head)
	{
		pTmp = head;
		head = head->next;
		free(pTmp);
	}

	return NoErr;
}


/*插入数据*/
int ptr_hash_inst(ptr_hash_table_class_t *_this, char *pcKey, ptr_node_t *pValue)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcKey, pValue);

	/*查找*/
	ptr_node_t * hashValueList = 
		(ptr_node_t *)(_this->stHash.find((hash_table_class_t*)_this, pcKey));
	if(NULL == hashValueList)
	{
	HY_DEBUG("Add cond hash\n");
		/*添加哈希表*/
		ptr_node_t *head = (ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == head)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
				
		head->next = pValue;
		pValue->prev = head;
		
		_this->stHash.inst((hash_table_class_t*)_this, pcKey, (void*)head);
	}
	else
	{
	HY_DEBUG("Add cond hash node\n");
		/*添加value链表结点*/
		ptr_node_t *head = hashValueList;

		/*插入到头结点之后*/
		pValue->prev = head;
		pValue->next = head->next;
		if(pValue->next)
		{
			pValue->next->prev = pValue;
		}
		head->next = pValue;
	}

	return NoErr;
}

/*删除数据*/
int ptr_hash_del_all(ptr_hash_table_class_t *_this, char *pcKey)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcKey);
	
	return _this->stHash.del((hash_table_class_t*)_this, pcKey);
}

/*删除数据*/
int ptr_hash_del(ptr_hash_table_class_t *_this, char *pcKey, char *paId)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcKey, paId);

	/*查找*/
	ptr_node_t * hashValueList = 
		(ptr_node_t *)(_this->stHash.find((hash_table_class_t*)_this, pcKey));
	if(NULL == hashValueList)
	{
		return NoErr;
	}
	else
	{
		/*删除value链表结点*/
		ptr_node_t *node = hashValueList->next;
		
		while(node)
		{
			if(!strncmp(node->stCondPtr.acId, paId, INDEX_MAX_LEN))
			{
				/*删除结点*/
				node->prev->next = node->next;
				if(node->next)
				{
					node->next->prev = node->prev;
				}
				free(node);
				/*删除后，如果只剩头结点，则需要删除该哈希值*/
				if(NULL == hashValueList->next)
				{
					_this->stHash.del((hash_table_class_t*)_this, pcKey);
				}
				break;
			}
			node = node->next;
		}
	}

	return NoErr;
}

/*查找数据*/
ptr_node_t *ptr_hash_find_all(ptr_hash_table_class_t *_this, char *pcKey)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pcKey);

	return (ptr_node_t *)(_this->stHash.find((hash_table_class_t*)_this, pcKey));
}

/*查找数据*/
ptr_node_t *ptr_hash_find(ptr_hash_table_class_t *_this, char *pcKey, char *paId)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pcKey, paId);

	/*查找*/
	ptr_node_t * hashValueList = 
		(ptr_node_t *)(_this->stHash.find((hash_table_class_t*)_this, pcKey));
	if(NULL == hashValueList)
	{
		return NULL;
	}
	else
	{
		/*查找value链表结点*/
		ptr_node_t *node = hashValueList->next;
		
		while(node)
		{
			if(!strncmp(node->stCondPtr.acId, paId, INDEX_MAX_LEN))
			{
				return node;
			}
			node = node->next;
		}
	}

	return NULL;
}

/*打印数据*/
int ptr_hash_print(ptr_hash_table_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->stHash.print((hash_table_class_t*)_this, print_ptr_value);
}

/*构造函数*/
ptr_hash_table_class_t *new_ptr_hash_table(void)
{
	/*申请空间*/
	ptr_hash_table_class_t *pNew = 
		(ptr_hash_table_class_t *)calloc(1, sizeof(ptr_hash_table_class_t));
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}

	/*调用父类的构造函数*/
	hash_table_class_t *pstHash = new_hash_table(0, 1, free_hash_value);
	if(NULL == pstHash)
	{
		HY_ERROR("New Hash Table Error.\n");
		free(pNew);
		return NULL;
	}
	/*将父类的数据拷贝到子类中*/
	memcpy(pNew, pstHash, sizeof(hash_table_class_t));
	/*pstLink里的某些成员也是动态分配的，而这些内存是要使用的，所以
	*此处指释放pstHash，而不释放成员的分配
	*/
	free(pstHash);

	/*初始化方法*/
	pNew->ptr_hash_inst = ptr_hash_inst;
	pNew->ptr_hash_del_all = ptr_hash_del_all;
	pNew->ptr_hash_del = ptr_hash_del;
	pNew->ptr_hash_find_all = ptr_hash_find_all;
	pNew->ptr_hash_find = ptr_hash_find;
	pNew->ptr_hash_print = ptr_hash_print;
	
	
	return pNew;
}
/*析构函数*/
int destroy_ptr_hash_table(ptr_hash_table_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return destroy_hash_table((hash_table_class_t *)_this);
}


