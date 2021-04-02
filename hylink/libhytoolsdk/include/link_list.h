/***********************************************************
*文件名     : link_list.h
*版   本   : v1.0.0.0
*日   期   : 2019.04.14
*说   明   : 链表类
*修改记录: 
************************************************************/


#ifndef LINK_LIST_H
#define LINK_LIST_H

/*相关函数指针类型定义*/
typedef int (*CmpDataFunc)(void *, int, void *, int);
typedef int (*PrintDataFun)(void *, int);
typedef int (*FreeDataFun)(void *, int);


#define LINK_LIST_PRIVATE_PARAM_MAX_LEN	32

/*简单链表类（单向链表）*/
typedef struct simple_link_list_class_s
{
	/*private*/
	unsigned char acPrivateParam[LINK_LIST_PRIVATE_PARAM_MAX_LEN];

	/*加锁(阻塞)*/
	int (*lock) (struct simple_link_list_class_s *_this);
	/*加锁(非阻塞)*/
	int (*trylock) (struct simple_link_list_class_s *_this);
	/*解锁*/
	int (*unlock) (struct simple_link_list_class_s *_this);
	
	/*获取链表长度*/
	int (*len) (struct simple_link_list_class_s* _this);
	/*获取链表头数据指针*/
	void* (*head) (struct simple_link_list_class_s* _this);
	/*获取链表尾数据指针*/
	void* (*tail) (struct simple_link_list_class_s* _this);
	/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
	返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
	void* (*next) (struct simple_link_list_class_s* _this, void* pCurrentData);
	/*查找数据，返回数据指针*/
	void* (*find) (struct simple_link_list_class_s* _this, void* pCmpDataCb, void* pData, int iDataLen);
	
	/*头插入数据*/
	int (*headInsert) (struct simple_link_list_class_s* _this, void* pData, int iDataLen);
	/*尾插入数据*/
	int (*tailInsert) (struct simple_link_list_class_s* _this, void* pData, int iDataLen);
	
	/*删除指定数据*/
	int (*del) (struct simple_link_list_class_s* _this, void* pCurrentData);
	/*清空数据*/
	int (*clear) (struct simple_link_list_class_s* _this);
	
	/*修改指定数据*/
	int (*set) (struct simple_link_list_class_s* _this, void* pCurrentData, void* pData, int iDataLen);
	
	/*获取指定数据*/
	int (*get) (struct simple_link_list_class_s* _this, void* pCurrentData, void* pData, int* piDataLen);

	
	/*注册数据域释放回调，
	* 如果数据域中有某一部分数据是动态分配的，
	* 则需要注册释放回调接口*/
	int (*freeDataCbReg) (struct simple_link_list_class_s* _this, void* pFreeDataCb);
	
	/*打印数据*/
	int (*print)(struct simple_link_list_class_s* _this, void* pPrintDataCb);
}
simple_link_list_class_t;

/*构造函数*/
simple_link_list_class_t *new_simple_link_list(void);

/*析构函数*/
int destroy_simple_link_list(simple_link_list_class_t *_this);





/*双向链表类*/
typedef struct double_link_list_class_s
{
	/*private*/
	unsigned char acPrivateParam[LINK_LIST_PRIVATE_PARAM_MAX_LEN];

	/*加锁(阻塞)*/
	int (*lock) (struct double_link_list_class_s *_this);
	/*加锁(非阻塞)*/
	int (*trylock) (struct double_link_list_class_s *_this);
	/*解锁*/
	int (*unlock) (struct double_link_list_class_s *_this);
	
	/*获取链表长度*/
	int (*len) (struct double_link_list_class_s* _this);
	/*获取链表头数据指针*/
	void* (*head) (struct double_link_list_class_s* _this);
	/*获取链表尾数据指针*/
	void* (*tail) (struct double_link_list_class_s* _this);
	/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
	返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
	void* (*next) (struct double_link_list_class_s* _this, void* pCurrentData);
	/*获取prev数据指针, 如果pCurrentData=NULL，则返回尾数据指针。
	返回值为所需要的prev数据指针，如果返回NULL，则表示没有prev*/
	void* (*prev) (struct double_link_list_class_s* _this, void* pCurrentData);
	/*查找数据，返回数据指针*/
	void* (*find) (struct double_link_list_class_s* _this, void* pCmpDataCb, void* pData, int iDataLen);
	
	/*头插入数据*/
	int (*headInsert) (struct double_link_list_class_s* _this, void* pData, int iDataLen);
	/*尾插入数据*/
	int (*tailInsert) (struct double_link_list_class_s* _this, void* pData, int iDataLen);
	/*当前位置入数据*/
	int (*insert) (struct double_link_list_class_s* _this, void* pCurrentData, void* pData, int iDataLen);
	
	/*删除指定数据*/
	int (*del) (struct double_link_list_class_s* _this, void* pCurrentData);
	/*清空数据*/
	int (*clear) (struct double_link_list_class_s* _this);
	
	
	/*修改指定数据*/
	int (*set) (struct double_link_list_class_s* _this, void* pCurrentData, void* pData, int iDataLen);
	
	/*获取指定数据*/
	int (*get) (struct double_link_list_class_s* _this, void* pCurrentData, void* pData, int* piDataLen);

	
	/*注册数据域释放回调，
	* 如果数据域中有某一部分数据是动态分配的，
	* 则需要注册释放回调接口*/
	int (*freeDataCbReg) (struct double_link_list_class_s* _this, void* pFreeDataCb);
	
	/*打印数据*/
	int (*print)(struct double_link_list_class_s* _this, void* pPrintDataCb);
}double_link_list_class_t;

/*构造函数*/
double_link_list_class_t *new_double_link_list(void);

/*析构函数*/
int destroy_double_link_list(double_link_list_class_t *_this);





/*list类(有序链表)*/
typedef struct order_list_class_s
{
	/*private*/
	unsigned char acPrivateParam[LINK_LIST_PRIVATE_PARAM_MAX_LEN];

	/*加锁(阻塞)*/
	int (*lock) (struct order_list_class_s *_this);
	/*加锁(非阻塞)*/
	int (*trylock) (struct order_list_class_s *_this);
	/*解锁*/
	int (*unlock) (struct order_list_class_s *_this);
	
	/*获取链表长度*/
	int (*len) (struct order_list_class_s* _this);
	
	/*获取next数据指针, 如果pCurrentData=NULL，则返回头数据指针。
	返回值为所需要的next数据指针，如果返回NULL，则表示没有next*/
	void* (*next) (struct order_list_class_s* _this, void* pCurrentData);
	/*查找数据，返回数据指针*/
	void* (*find) (struct order_list_class_s* _this, void* pCmpDataCb, void* pData, int iDataLen);
	/*获取指定下标数据，返回数据指针*/
	void* (*findByIndex) (struct order_list_class_s* _this, int iIndex);
	/*获取数据Index*/
	int (*index) (struct order_list_class_s* _this, void* pCurrentData);

	/*当前位置入数据*/
	int (*insert) (struct order_list_class_s* _this, void* pCurrentData, void* pData, int iDataLen);
	/*插入数据*/
	int (*insertById) (struct order_list_class_s* _this, int iIndex, void* pData, int iDataLen);
	/*追加数据*/
	int (*append) (struct order_list_class_s* _this, void* pData, int iDataLen);

	/*删除指定数据*/
	int (*del) (struct order_list_class_s* _this, void* pCurrentData);
	/*删除指定数据*/
	int (*delById) (struct order_list_class_s* _this, int iIndex);
	/*清空数据*/
	int (*clear) (struct order_list_class_s* _this);
	
	/*修改指定数据*/
	int (*set) (struct order_list_class_s* _this, void* pCurrentData, void* pData, int iDataLen);
	
	/*获取指定数据*/
	int (*get) (struct order_list_class_s* _this, void* pCurrentData, void* pData, int* piDataLen);

	
	/*注册数据域释放回调，
	* 如果数据域中有某一部分数据是动态分配的，
	* 则需要注册释放回调接口*/
	int (*freeDataCbReg) (struct order_list_class_s* _this, void* pFreeDataCb);
	
	/*打印数据*/
	int (*print)(struct order_list_class_s* _this, void* pPrintDataCb);
}
order_list_class_t;
/*构造函数*/
order_list_class_t *new_order_link_list(void);

/*析构函数*/
int destroy_order_link_list(order_list_class_t *_this);


#endif /* LINK_LIST_H */
