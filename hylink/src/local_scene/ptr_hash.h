/***********************************************************
*文件名    : ptr_hash.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.21
*说   明   : 指针哈希表接口
*修改记录: 
************************************************************/

#ifndef PTR_HASH_H
#define PTR_HASH_H

#include "hashtable.h"
#include "len.h"


#pragma pack(1)
typedef struct ptr_info_s
{
	/*该条件属于哪个场景ID*/
	char acId[INDEX_MAX_LEN];
	
	/*该条件属于哪个场景*/
	void *pstScene;
	/*该条件属于哪个条件列表*/
	void *pstCondList;
	/*该条件属于哪个条件集合结点*/
	void *pCondSetNode;
	/*该条件是条件集合中的哪个条件结点*/
	void *pCondNode;

	/*实体场景面板*/
	void *pScenePanelNode;
}ptr_info_t;


/*hash value 是一个带头结点的双向链表*/
typedef struct ptr_node_s  
{  
	ptr_info_t stCondPtr;
	struct ptr_node_s *next;
	struct ptr_node_s *prev;
}ptr_node_t;


typedef struct ptr_hash_table_class_s
{
	hash_table_class_t stHash;
	
	/*方法*/
	
	/*插入数据*/
	int (*ptr_hash_inst)(struct ptr_hash_table_class_s*, char *, ptr_node_t *);
	/*删除数据*/
	int (*ptr_hash_del_all)(struct ptr_hash_table_class_s*, char *);
	/*删除数据*/
	int (*ptr_hash_del)(struct ptr_hash_table_class_s*, char *, char *);
	/*查找数据*/
	ptr_node_t *(*ptr_hash_find_all)(struct ptr_hash_table_class_s*, char *);
	/*查找数据*/
	ptr_node_t *(*ptr_hash_find)(struct ptr_hash_table_class_s*, char *, char *);
	/*打印数据*/
	int (*ptr_hash_print)(struct ptr_hash_table_class_s*);
}ptr_hash_table_class_t;
#pragma pack()

/*构造函数*/
ptr_hash_table_class_t *new_ptr_hash_table(void);
/*析构函数*/
int destroy_ptr_hash_table(ptr_hash_table_class_t *_this);

#endif/* PTR_HASH_H */



