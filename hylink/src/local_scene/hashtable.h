/***********************************************************
*文件名    : hashtable.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 哈希表接口
*修改记录: 
************************************************************/
#ifndef HASH_TABLE_H
#define HASH_TABLE_H



#define HASH_TABLE_PRIVATE_METHODS_MAX_LEN	128
#define HASH_TABLE_PRIVATE_PARAM_MAX_LEN 128

typedef int (*PrintValueFunc)(void*);
typedef int (*FreeValueFunc)(void*);
#pragma pack(1)

typedef struct hash_table_class_s
{
	/*参数*/
	/*private*/
	unsigned char acHashTablePrivateParam[HASH_TABLE_PRIVATE_PARAM_MAX_LEN];
	unsigned int uiHashTableSize;//hash表大小
	unsigned char ucDilatationEnable;//自动扩容使能
	FreeValueFunc pFreeValueHandle;//释放value域函数
	/*方法*/
	/*private*/
	unsigned char acHashTablePrivateMethods[HASH_TABLE_PRIVATE_METHODS_MAX_LEN];
	
	/*判断是否为空*/
	int (*empty)(struct hash_table_class_s*);
	/*获取元素个数*/
	int (*size)(struct hash_table_class_s*);
	/*插入数据*/
	int (*inst)(struct hash_table_class_s*, char *, void *);
	/*删除数据*/
	int (*del)(struct hash_table_class_s*, char *);
	/*清空数据*/
	int (*clear)(struct hash_table_class_s*);
	/*查找数据*/
	void *(*find)(struct hash_table_class_s*, char *);
	/*打印数据*/
	int (*print)(struct hash_table_class_s*, PrintValueFunc);
}hash_table_class_t;
#pragma pack()

/*构造函数*/
hash_table_class_t *new_hash_table(int iSize, int iDilatationEnable, FreeValueFunc pFreeValueHandle);
/*析构函数*/
int destroy_hash_table(hash_table_class_t *_this);


#endif/* HASH_TABLE_H */

