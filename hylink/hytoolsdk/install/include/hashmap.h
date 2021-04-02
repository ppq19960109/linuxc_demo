/***********************************************************
*文件名    : hashmap.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 哈希表接口
*修改记录: 
************************************************************/
#ifndef HASH_MAP_H
#define HASH_MAP_H

#define HASH_KEY_MAX_LEN 128

#define HASH_MAP_PRIVATE_METHODS_MAX_LEN	128
#define HASH_MAP_PRIVATE_PARAM_MAX_LEN 128


typedef struct hash_map_s
{
	/*参数*/
	/*private*/
	unsigned char acHashMapPrivateParam[HASH_MAP_PRIVATE_PARAM_MAX_LEN];
	unsigned int uiHashMapSize;//hash表大小
	unsigned char ucDilatationEnable;//自动扩容使能
	void *pFreeValueHandle;//释放value域函数
	void *pCbUserData;
	/*方法*/
	/*private*/
	unsigned char acHashMapPrivateMethods[HASH_MAP_PRIVATE_METHODS_MAX_LEN];

	/*加锁(阻塞)*/
	int (*lock) (struct hash_map_s *);
	/*加锁(非阻塞)*/
	int (*trylock) (struct hash_map_s *);
	/*解锁*/
	int (*unlock) (struct hash_map_s *);
	
	/*判断是否为空*/
	int (*empty)(struct hash_map_s*);
	/*获取元素个数*/
	int (*size)(struct hash_map_s*);
	/*插入数据*/
	int (*inst)(struct hash_map_s*, char *, void *, int);
	/*删除数据*/
	int (*del)(struct hash_map_s*, char *);
	/*清空数据*/
	int (*clear)(struct hash_map_s*);
	/*查找数据,返回值返回真实数据*/
	void *(*find)(struct hash_map_s*, char *);
	/*查找数据,参数返回备份数据*/
	int (*find2)(struct hash_map_s*, char *, void *, int *);
	/*注册哈希数据域，内存回收回调*/
	int (*free_cb_reg)(struct hash_map_s*, void *, void *);
	/*打印数据*/
	int (*print)(struct hash_map_s*, void*);
}__attribute__ ((__packed__)) hash_map_t;

typedef int (*PrintValueFunc)(void*);
typedef int (*FreeValueFunc)(
	hash_map_t *_this,
	void *pData,
	void *pUserData
);

/*构造函数*/
hash_map_t *new_hash_map(int iSize, int iDilatationEnable);
/*析构函数*/
int destroy_hash_map(hash_map_t *_this);


#endif/* HASH_TABLE_H */

