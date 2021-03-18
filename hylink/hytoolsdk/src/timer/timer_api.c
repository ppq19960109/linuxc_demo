#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <signal.h>
#include <unistd.h>

#include "timer_api.h"
#include "base_api.h"
#include "log_api.h"

task_timer_t *g_pstTimerList = NULL;
unsigned char g_ucThreadPoolDestroy = 0;
void *g_pool = NULL;
int g_iTimerNum = 0;
int g_iTimeClockWorkerId = 0;

void *_task_timer_clock(void *arg);

/*************************************************************
*函数:  task_timer_init
*参数:	iTimerNum:创建的定时器数量
*返回值:成功返回0，失败返回-1:
*描述:  初始化定时器
*************************************************************/
extern int
task_timer_init(void *thread_pool, int iTimerNum)
{
	int iRet = 0;
	if(iTimerNum < TIMER_NUM_MIN)
	{
		g_iTimerNum = TIMER_NUM_MIN;
	}
	else if(iTimerNum > TIMER_NUM_MAX)
	{
		g_iTimerNum = TIMER_NUM_MAX;
	}
	else
	{
		g_iTimerNum = iTimerNum;
	}
	
	g_pstTimerList = 
		(task_timer_t *)malloc(iTimerNum * sizeof(task_timer_t));
	if(NULL == g_pstTimerList)
	{
		return -1;
	}

	memset(g_pstTimerList, 0x0, iTimerNum * sizeof(task_timer_t));
	
		
	/*创建线程池*/
	if(NULL == thread_pool)
	{
		iRet = thread_pool_init(&g_pool, iTimerNum);
		if(iRet < 0)
		{
			return -1;
		}
		g_ucThreadPoolDestroy = 1;
	}
	else
	{
		g_pool = thread_pool;
		g_ucThreadPoolDestroy = 0;
	}

	/*添加定时器时钟任务*/
	g_iTimeClockWorkerId = 
		thread_pool_add_worker(g_pool, _task_timer_clock, NULL, 0);
	
	return 0;
}

extern int
task_timer_destroy()
{
	if(g_pstTimerList)
	{
		free(g_pstTimerList);
		g_pstTimerList = NULL;
	}
	
	
	if(1 == g_ucThreadPoolDestroy)
	{
		/*销毁线程池*/
		if(g_pool)
		{
			thread_pool_destroy(g_pool); 
			g_pool = NULL;
		}
	}
	else
	{
		/*去掉时钟任务*/
		thread_pool_del_worker(g_pool, g_iTimeClockWorkerId);
	}
	return	0;
}

/*************************************************************
*函数:  task_timer_create
*参数:	pfTimerExecFun:定时器时间到时的处理函数
*		pFunArg:处理函数的参数
*		ulTime:指定定时器时间片(单位s)
*		iFunExecTime:处理函数执行次数,-1表示一直执行
*返回值::成功返回定时器id
*描述:  创建一个定时器
*************************************************************/
extern int
task_timer_create(PF_TIMERFUN pfTimerExecFun, 
				void* pFunArg,
				int iFunArgLen,
				unsigned long ulTime,
				int iFunExecTime)
{
	/*参数定义*/
	int i = 0;

	/*参数校验*/
	if (0 == ulTime)
	{
		return -1;
	}
	if (0 == iFunExecTime || 
		iFunExecTime < -1)
	{
		return -1;
	}

	for (i = 0; i < g_iTimerNum; ++i)
	{
		if (0 == g_pstTimerList[i].ucUsedFlag)
		{
			break;
		}
	}

	if(i > g_iTimerNum)
	{
		printf("The timer exceeded its maximum capacity.\n");
		return -1;
	}
	
	memset(&g_pstTimerList[i], 0x0, 
		sizeof(g_pstTimerList[i]));

	
	g_pstTimerList[i].pfTimerExecFun = pfTimerExecFun;
	if(pFunArg)
	{
		void *pFunArgTmp = calloc(iFunArgLen, 1);
		if(NULL == pFunArgTmp)
		{
			printf("Calloc error.\n");
			return -1;
		}
		memcpy(pFunArgTmp, pFunArg, iFunArgLen);
		g_pstTimerList[i].pFunArg = pFunArgTmp;
		g_pstTimerList[i].iArgLen = iFunArgLen;
	}
	else
	{
		g_pstTimerList[i].pFunArg = NULL;
		g_pstTimerList[i].iArgLen = 0;
	}
	g_pstTimerList[i].ulTime = ulTime;
	g_pstTimerList[i].iFunExecTime = iFunExecTime;
	g_pstTimerList[i].ulRunTime = ulTime;
	g_pstTimerList[i].ucUsedFlag ++;
	
	return i;
}

/*************************************************************
*函数:  task_timer_change
*参数:	piTimerId:为已经存在的定时器id
*		pfTimerExecFun:定时器时间到时的处理函数，如果不改变置为NULL
*		pFunArg:处理函数的参数,如果不改变置为NULL
*		ulTime:指定定时器时间片(单位s),如果不改变置为0
*		iFunExecTime:处理函数执行次数,-1表示一直执行,如果不改变置为0
*返回值::
*描述:  修改一个定时器
*************************************************************/
extern int
task_timer_change(int iTimerId,
				PF_TIMERFUN pfTimerExecFun, 
				void* pFunArg,
				unsigned long ulTime,
				int iFunExecTime)
{
	if (0 == g_pstTimerList[iTimerId].ucUsedFlag)
	{
		return -1;
	}
	
	if(NULL != pfTimerExecFun)
	{
		g_pstTimerList[iTimerId].pfTimerExecFun = pfTimerExecFun;
	}
	if(NULL != pFunArg)
	{
		g_pstTimerList[iTimerId].pFunArg = pFunArg;
	}
	if(0 != ulTime)
	{
		g_pstTimerList[iTimerId].ulTime = ulTime;
		g_pstTimerList[iTimerId].ulRunTime = ulTime;
	}
	if(0 != iFunExecTime)
	{
		g_pstTimerList[iTimerId].iFunExecTime = iFunExecTime;
	}
	
	return 0;
}

/*************************************************************
*函数:  task_timer_dele
*参数:	iTimerId:计数器id
*返回值::
*描述:  删除一个定时器
*************************************************************/
extern int
task_timer_dele(int iTimerId)
{
	/*参数校验*/
	if (iTimerId >= g_iTimerNum)
	{
		return -1;
	}

	if(g_pstTimerList[iTimerId].pFunArg)
	{
		free(g_pstTimerList[iTimerId].pFunArg);
		g_pstTimerList[iTimerId].pFunArg = NULL;
	}
	g_pstTimerList[iTimerId].ucUsedFlag = 0;
	
	return 0;
}



/*************************************************************
*函数:  timer_reset
*参数:	iTimerId:计数器id
*返回值::
*描述:  复位一个定时器
*************************************************************/ 
extern int
task_timer_reset(int iTimerId,
				PF_TIMERFUN pfTimerExecFun, 
				void* pFunArg,
				unsigned long ulTime,
				int iFunExecTime)
{
	/*参数校验*/
	if (iTimerId >= g_iTimerNum)
	{
		return -1;
	}
	if (0 == iFunExecTime || 
		iFunExecTime < -1)
	{
		return -1;
	}
	if (0 == g_pstTimerList[iTimerId].ucUsedFlag)
	{
		return -1;
	}
	

	memset(&g_pstTimerList[iTimerId], 0x0, 
		sizeof(g_pstTimerList[iTimerId]));

	g_pstTimerList[iTimerId].ucUsedFlag ++;
	g_pstTimerList[iTimerId].pfTimerExecFun = pfTimerExecFun;
	g_pstTimerList[iTimerId].pFunArg = pFunArg;
	g_pstTimerList[iTimerId].ulTime = ulTime;
	g_pstTimerList[iTimerId].iFunExecTime = iFunExecTime;
	g_pstTimerList[iTimerId].ulRunTime = ulTime;

	return 0;
}

/*************************************************************
*函数:  timer_run
*参数:	
*返回值:
*描述:  定时器运行处理
*************************************************************/ 
void
task_timer_run(int signal)
{
	if(NULL == g_pstTimerList)
	{
		return;
	}
	/*参数定义*/
	int i = 0;
				
	for (i = 0; i < g_iTimerNum; ++i)
	{
		/*该计时器是否启用*/
		if(g_pstTimerList[i].ucUsedFlag)
		{
			/*查看计时器是否超时*/
			if(g_pstTimerList[i].ulRunTime <= 1)
			{
				/*计数器超时，执行任务*/
				thread_pool_add_worker(g_pool, g_pstTimerList[i].pfTimerExecFun, g_pstTimerList[i].pFunArg, g_pstTimerList[i].iArgLen);
				/*任务执行次数计数*/
				if(-1 != g_pstTimerList[i].iFunExecTime)
				{
					g_pstTimerList[i].iFunExecTime --;
				}
				
				/*重新配置计时值*/
				if(g_pstTimerList[i].iFunExecTime)
				{
					g_pstTimerList[i].ulRunTime = 
						g_pstTimerList[i].ulTime;
				}
				else
				{
					/*计数器任务完成，释放该计时器*/
					g_pstTimerList[i].ucUsedFlag = 0;
					if(g_pstTimerList[i].pFunArg)
					{
						free(g_pstTimerList[i].pFunArg);
						g_pstTimerList[i].pFunArg = NULL;
					}
				}
			}
			else
			{
				/*计数器运行时间更新*/
				g_pstTimerList[i].ulRunTime --;
			}
		}
	}

	return;
}


/*************************************************************
*函数:  timer_clock
*参数:	
*返回值:
*描述:  定时器时钟，每秒产生一个信号
*************************************************************/
void *_task_timer_clock(void *arg)
{
	/*注册信号*/
	signal(SIGUSR1, &task_timer_run);

	/*每隔一秒钟发送一次SIGUSR1*/
	while(1) 
	{
		sleep(1);
		kill(getpid(), SIGUSR1);
	}
}



