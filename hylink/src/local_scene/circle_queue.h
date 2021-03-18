/***********************************************************
*文件名    : circle_queue.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.06
*说   明   : 环形队列接口
*修改记录: 
************************************************************/
#ifndef CIRCLE_QUEUE_H
#define CIRCLE_QUEUE_H

#define QUEUE_PRIVATE_METHODS_MAX_LEN	128

typedef int (*PrintValueFunc)(void*);
typedef int (*CmpValueFunc)(void*, void*);
#pragma pack(1)
typedef struct circle_queue_class_s
{
	/*参数*/
	/*private*/
	unsigned char acQueuePrivateParam[QUEUE_PRIVATE_METHODS_MAX_LEN];
	
	int iSize;//队列最大元素个数
	unsigned char ucFreeValueEnable;//释放value域使能
	
	/*方法*/
	/*队列最大长度*/
	int (*size)(struct circle_queue_class_s*);
	/*队列长度*/
	int (*len)(struct circle_queue_class_s*);
	/*队列是否为空*/
	int (*empty)(struct circle_queue_class_s*);
	/*队列是否已满*/
	int (*full)(struct circle_queue_class_s*);
	/*出队*/
	void* (*pop)(struct circle_queue_class_s*);
	/*入队*/
	int (*push)(struct circle_queue_class_s*, void*);
	/*清空队列*/
	int (*clear)(struct circle_queue_class_s*);
	
	/*获取队头元素，但不出队*/
	void* (*top)(struct circle_queue_class_s*);
	/*获取队尾元素，但不出队*/
	void* (*foot)(struct circle_queue_class_s*);
	/*获取指定id的元素，但不出队*/
	void* (*get_by_id)(struct circle_queue_class_s*, int);
	/*获取内容的元素，但不出队*/
	void* (*get)(struct circle_queue_class_s*, CmpValueFunc, void*);
	
	/*打印队列，仅用于调试*/
	int (*print)(struct circle_queue_class_s*, PrintValueFunc);
}circle_queue_class_t;
#pragma pack()

/*构造函数*/
circle_queue_class_t *new_circle_queue(int iSize, int iFreeValueEnable);
/*析构函数*/
int destroy_circle_queue(circle_queue_class_t *_this);


#endif /* CIRCLE_QUEUE_H */