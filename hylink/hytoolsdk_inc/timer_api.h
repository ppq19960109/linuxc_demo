#ifndef _TIMER_H_
#define _TIMER_H_


#define TIMER_NUM 				16
#define TIMER_NUM_MIN 			1
#define TIMER_NUM_MAX 			32

/*定时任务循环执行标志*/
#define TIMER_TASK_ALWAYS		-1
#define TIMER_TASK_ONCE			1

/*定义功能参数*/
typedef struct task_timer_param_s{
	/*定时器id*/
	int *piTimerId;
	/*其他参数如需要自行定义*/
	void* pParam;
} task_timer_param_t;

typedef void*(*PF_TIMERFUN)(void*);

typedef struct
{
	/*计数器超时后执行的任务*/
	PF_TIMERFUN pfTimerExecFun;
	/*任务接口传入的参数*/
	void* pFunArg;
	int iArgLen;
	/*任务执行次数,-1表示一直运行*/
	int iFunExecTime;
	/*计数器时间*/
	unsigned long ulTime;
	/*计数器运行时间*/
    unsigned long ulRunTime;
	/*计数器启用标志*/
    unsigned char ucUsedFlag;
}task_timer_t;


/*初始化定时器*/
extern int
task_timer_init(void *thread_pool, int iTimerNum);
/*销毁定时器*/
extern int
task_timer_destroy();
/*创建一个定时器*/
extern int
task_timer_create(PF_TIMERFUN pfTimerExecFun, 
			void* pFunArg,
			int iFunArgLen,
			unsigned long ulTime,
			int iFunExecTime);
/*修改一个定时器*/
extern int
task_timer_change(int iTimerId,
			PF_TIMERFUN pfTimerExecFun, 
			void* pFunArg,
			unsigned long ulTime,
			int iFunExecTime);

/*删除一个定时器*/
extern int
task_timer_dele(int iTimerId);

/*复位一个定时器*/
extern int
task_timer_reset(int iTimerId,
			PF_TIMERFUN pfTimerExecFun, 
			void* pFunArg,
			unsigned long ulTime,
			int iFunExecTime);

#endif	/* _TIMER_H_ */
