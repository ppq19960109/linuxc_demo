/***********************************************************
*文件名     : msg_queue.h
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 消息队列类
*修改记录: 
************************************************************/


#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#define USE_MUTEX_LOCK
//#define USE_SEMAPHORE_LOCK

#define MSG_QUEUE_PRIVATE_PARAM_MAX_LEN	128

/*相关函数指针类型定义*/
typedef int (*PrintDataFun)(void *, int);
typedef int (*FreeDataFun)(void *, int);



/*简单链表类（单向链表）*/
typedef struct msg_queue_class_s
{
	/*private*/
	unsigned char acPrivateParam[MSG_QUEUE_PRIVATE_PARAM_MAX_LEN];

	/**************************************************************
	*描述: 消息队列加锁
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*lock) (struct msg_queue_class_s* _this);
	/**************************************************************
	*描述: 消息队列加锁(非阻塞)
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*trylock) (struct msg_queue_class_s* _this);
	/**************************************************************
	*描述: 消息队列解锁
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*unlock) (struct msg_queue_class_s* _this);
	
	/**************************************************************
	*描述: 获取队列长度
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*	   @int iMsgType: 消息类型, -i表示任意类型, >=0表示消息类型
	*返回: 成功返回消息队列长度，失败返回-1；
	**************************************************************/
	int (*len) (struct msg_queue_class_s* _this, int iMsgType);
	/**************************************************************
	*描述: 入队
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*	   @int iMsgType: 消息类型
	*	   @void* pMsg: 消息
	*	   @int iMsgLen: 消息长度
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*push) (struct msg_queue_class_s* _this, int iMsgType, void* pMsg, int iMsgLen);
	/**************************************************************
	*描述: 出队
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*	   @int* piMsgType: 消息类型
	*	   @int iTimeOut: 超时时间，-1表示阻塞, 0表示立即返回, >0表示超时时间，单位ms
	*	   @void* pMsg: 用于返回消息
	*	   @int *piMsgLen: 传入消息缓存最大长度，传出消息真实长度
	*返回: 成功返回0，失败返回-1；
	*注意：如果pMsg域中有需要free的数据，需要自己释放
	**************************************************************/
	int (*pop) (struct msg_queue_class_s* _this, int* piMsgType,  int iTimeOut, void* pMsg, int *piMsgLen);

	/**************************************************************
	*描述: 清空队列
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*clear) (struct msg_queue_class_s* _this);

	/**************************************************************
	*描述: 注册数据域释放回调
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*	   @void* pFreeDataCb: 数据域释放回调
	*返回: 成功返回0，失败返回-1；
	*说明: 如果队列数据中没有需要释放的，则无需调用该接口
	**************************************************************/
	int (*freeDataCbReg) (struct msg_queue_class_s* _this, void* pFreeDataCb);
	
	/**************************************************************
	*描述: 打印数据
	*参数: @struct msg_queue_class_s* _this: 消息队列类指针
	*	   @void* pPrintDataCb: 打印回调
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*print)(struct msg_queue_class_s* _this, void* pPrintDataCb);
}
msg_queue_class_t;

/*构造函数*/
msg_queue_class_t *new_msg_queue(void);

/*析构函数*/
int destroy_msg_queue(msg_queue_class_t *_this);

#endif /*MSG_QUEUE_H*/
