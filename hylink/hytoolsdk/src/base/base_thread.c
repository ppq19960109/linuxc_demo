#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <memory.h>   
#include <sys/types.h>  
#include <signal.h>

#include "error_msg.h"
#include "base_api.h"

int g_worker_id = 0;

typedef void *(*WorkerFunc)(void*);

/* 线程任务链表 */  
typedef struct _thread_worker_t  
{
	int worker_id;				/*任务ID*/
	pthread_t threadid;		/*执行该任务的线程id*/
    WorkerFunc process;  /* 线程处理的任务 */  
    void *arg;                    /* 任务接口参数 */  
	int arg_len;				/*参数长度*/
    struct _thread_worker_t *next;/* 下一个节点 */  
}thread_worker_t;  
  
/* 线程池对象 */  
typedef struct  
{  
    pthread_mutex_t queue_lock;   /* 队列互斥锁 */  
    pthread_cond_t queue_ready;   /* 队列条件锁 */  
  
    thread_worker_t *head;        /* 任务队列头指针 */  
    int isdestroy;    /* 是否已销毁线程 */  
    pthread_t *threadid;          /* 线程ID数组 —动态分配空间 */  
    int reqnum;                   /* 请求创建的线程个数 */  
    int num;                      /* 实际创建的线程个数 */  
    int queue_size;               /* 工作队列当前大小 */  
}thread_pool_t;  

static void *thread_routine(void *arg);
static void *thread_keepalive(void *arg);
static int thread_create_detach(void *pool, int idx);

/************************************************************* 
 **功  能：线程池的初始化 
 **参  数： 
 **    pool：线程池对象 
 **    num ：线程池中线程个数 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
int thread_pool_init(void **pool, int num)  
{
	int ret = 0;
    int idx = 0; 

    /* 为线程池分配空间 */  
    thread_pool_t *pstPool = 
    	(thread_pool_t*)base_calloc(1, sizeof(thread_pool_t));  
    if(NULL == pstPool)  
    {
        return -1;  
    }  

    /* 初始化线程池 */  
    pthread_mutex_init(&(pstPool->queue_lock), NULL);  
    pthread_cond_init(&(pstPool->queue_ready), NULL);  
    pstPool->head = NULL; 
	num += 1;/*增加一个线程，用于线程保活*/
    pstPool->reqnum = num;  
    pstPool->queue_size = 0;  
    pstPool->isdestroy = 0;  
    pstPool->threadid = 
		(pthread_t*)base_calloc(1, num*sizeof(pthread_t));  
    if(NULL == pstPool->threadid)  
    {  
        base_free(pstPool);  
        pstPool = NULL;  
        return -1;  
    }  

    /* 依次创建线程 */  
    for(idx=0; idx<num; idx++)  
    {  
        ret = thread_create_detach(pstPool, idx);  
        if(0 != ret)  
        {  
            return -1;  
        }  
        pstPool->num++;  
    }  
	base_delay_s(1);
	
	/*添加线程保活任务*/
	thread_pool_add_worker(
		pstPool, 
		thread_keepalive,
		&pstPool,
		sizeof(thread_pool_t *)
	);
	
	*pool = (void*)pstPool;
    return 0;  
}

/************************************************************* 
 **功  能：将任务加入线程池处理队列 
 **参  数： 
 **    pool：线程池对象 
 **    process：需处理的任务 
 **    arg: process函数的参数 
 **返回值：成功返回任务ID ，失败小于0 
 *************************************************************/  
int thread_pool_add_worker(
	void *pool, 
	ThreadTaskFunc process,
	void *arg,
	int arg_len
)  
{ 
	thread_pool_t *pstPool = (thread_pool_t *)pool;
    thread_worker_t *worker=NULL, *member=NULL;  
    
    worker = (thread_worker_t*)base_calloc(
		1, sizeof(thread_worker_t));  
    if(NULL == worker)  
    {  
        return -1;  
    }  
	
	worker->threadid = 0;
	worker->worker_id = (++g_worker_id);
    worker->process = process;  
	if(arg)
	{
		void *pFunArgTmp = base_calloc(arg_len, 1);
		if(NULL == pFunArgTmp)
		{
			printf("Calloc error.\n");
			return -1;
		}
		
		memcpy(pFunArgTmp, arg, arg_len);
		worker->arg = pFunArgTmp;
		worker->arg_len = arg_len;
	}
	else
	{
		worker->arg = arg;
		worker->arg_len = 0;
	}
    worker->next = NULL;  
  
    pthread_mutex_lock(&(pstPool->queue_lock));  
 
    member = pstPool->head;  
    if(NULL != member)  
    {  
        while(NULL != member->next) member = member->next;  
        member->next = worker;  
    }  
    else  
    {  
        pstPool->head = worker;  
    }
  
    pstPool->queue_size++;  
    pthread_mutex_unlock(&(pstPool->queue_lock));  
    pthread_cond_signal(&(pstPool->queue_ready)); 
	
    return worker->worker_id;  
}  
/************************************************************* 
 **功  能：终止任务
 **参  数： 
 **    pool：线程池对象 
 **    worker_id：任务ID 
 **返回值：成功返回0 ，失败小于0 
 *************************************************************/  
int thread_pool_del_worker(void *pool, int worker_id) 
{
	thread_pool_t *pstPool = (thread_pool_t *)pool;
	thread_worker_t *worker = NULL; 
	thread_worker_t *worker_pre = NULL;
	thread_worker_t *worker_del = NULL;
	pthread_mutex_lock(&(pstPool->queue_lock));  

	worker = pstPool->head;
	while(NULL != worker)  
    {  
        if(worker_id == worker->worker_id)
    	{
    		if(0 != worker->threadid)
			{
				/*取消线程*/
				HY_INFO("Kill the thread(%d).\n",
					worker->threadid);
				pthread_cancel(worker->threadid);

				/*重新创建线程*/
				//int idx = 0;
			    //for(idx=0; idx<pstPool->num; idx++)  
			    //{
			    //	if(pstPool->threadid[idx] == worker->threadid)
			    //	{
			    //		HY_INFO("Create a thread.\n");
			    //        thread_create_detach(pstPool, idx); 
			    //    }  
			    //}  
			}

			/*删除任务*/
			if(worker == pstPool->head)
			{
				worker_del = worker;
				pstPool->head = NULL;
				pstPool->queue_size--;
				break;
			}
			else
			{
				worker_pre->next = worker->next;
				worker_del = worker;
				pstPool->queue_size--;
				break;
			}
        }
		worker_pre = worker;
        worker = worker->next;  
    }
	
	pthread_mutex_unlock(&(pstPool->queue_lock));

	if(worker_del)
	{
		if(worker_del->arg)
		{
			base_free(worker_del->arg);
		}
		
        base_free(worker_del); 
		worker_del = NULL;
	}
	return 0;
}

/************************************************************* 
 **功  能：线程池的销毁 
 **参  数： 
 **    pool：线程池对象 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
int thread_pool_destroy(void *pool)  
{  
	thread_pool_t *pstPool = (thread_pool_t *)pool;
	int iRet = 0;
    int idx = 0;  
 
    if(0 != pstPool->isdestroy)  
    {  
        return -1;  
    }  
	pstPool->isdestroy = 1;

	/*关闭所有任务*/
	thread_worker_t *worker = NULL; 
	thread_worker_t *worker_del = NULL;
	worker = pstPool->head;
	while(NULL != worker)  
    {  
		if(0 != worker->threadid)
		{
			worker_del = worker;
		}

        worker = worker->next;  

		if(NULL == worker_del)
		{
			thread_pool_del_worker(pool, worker_del->worker_id);
			worker_del = NULL;
		}
    }

	/*关闭所有线程*/
	pthread_cond_broadcast(&(pstPool->queue_ready));
	for(idx=0; idx<pstPool->num; idx++)
	{
		iRet = pthread_kill(pstPool->threadid[idx], 0);
        if(ESRCH == iRet)  
        {
        	/*线程已退出*/
			pthread_join(pstPool->threadid[idx], NULL);
			
           continue;
        }
		pthread_cancel(pstPool->threadid[idx]);
	}
	base_delay_s(1);
    base_free(pstPool->threadid);  
    pstPool->threadid = NULL;  
    pthread_mutex_destroy(&(pstPool->queue_lock));  
    pthread_cond_destroy(&(pstPool->queue_ready));  
    base_free(pstPool); 

    return 0;  
}  
/************************************************************* 
 **功  能：线程保活函数 
 **参  数： 
 **    arg：线程池对象 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
static void *thread_keepalive(void *arg)
{
	thread_pool_t *pstPool = *(thread_pool_t**)arg; 
	int iRet = 0;
	int idx = 0;
	
	/*开启线程退出功能*/
	base_thread_set_cancel();

	while(1)
	{
		for(idx=0; idx<pstPool->num; idx++)  
	    {  
	        iRet = pthread_kill(pstPool->threadid[idx], 0);  
	        if(ESRCH == iRet)  
	        {
	        	/*回收线程资源*/
				pthread_join(pstPool->threadid[idx], NULL);

				/*创建新的线程*/
	            iRet = thread_create_detach(pstPool, idx);  
	            if(iRet < 0)  
	            {  
	                return NULL;  
	            }  
	        }  
	    }
		base_delay_s(1);
	}

	return NULL;
}

/************************************************************* 
 **功  能：线程池各个线程入口函数 
 **参  数： 
 **    arg：线程池对象 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
static void *thread_routine(void *arg)
{  
    thread_worker_t *worker = NULL; 
	thread_worker_t *worker_del = NULL; 
    thread_pool_t *pool = (thread_pool_t*)arg;  
  	/*开启线程退出功能*/
	base_thread_set_cancel();

    while(1)  
    {  
        pthread_mutex_lock(&(pool->queue_lock));  
		if((0 == pool->isdestroy))  
        { 
            pthread_cond_wait(&(pool->queue_ready),
				&(pool->queue_lock)); 
        }  
  		
        if(0 != pool->isdestroy)  
        {  
            pthread_mutex_unlock(&(pool->queue_lock));  
            pthread_exit(NULL);  
        }  
		
		/*查找未执行的任务*/
		int worker_flag = 0;
		worker = pool->head;
		while(NULL != worker)  
	    {  
	        if(0 == worker->threadid)
        	{
        		worker_flag = 1;
				break;
        	}
	        worker = worker->next;  
	    }
		
		if(1 == worker_flag)
		{
			worker->threadid = pthread_self();
		}
		else
		{
			pthread_mutex_unlock(&(pool->queue_lock));  
			continue;
		}
        pthread_mutex_unlock(&(pool->queue_lock));

		HY_DEBUG("pid = %d, Start a task.\n", pthread_self());
        /* 执行队列中的任务 */  
        (*(worker->process))(worker->arg);
		HY_DEBUG("pid = %d, End a task.\n");

		pthread_mutex_lock(&(pool->queue_lock));  
		/*任务执行完毕，删除任务*/
		if(worker == pool->head)
		{
			worker_del = worker;
			pool->head = NULL;
			pool->queue_size--;
		}
		else
		{
			worker_del = pool->head;
			while(NULL != worker_del)
			{
				if(worker == worker_del->next)
				{
					worker_del->next = worker->next;
					worker_del = worker;
					pool->queue_size--;
					break;
				}
				worker_del = worker_del->next;
			}
		}
		pthread_mutex_unlock(&(pool->queue_lock));

		if(worker_del)
		{
			if(worker_del->arg)
			{
				base_free(worker_del->arg);
			}
			
	        base_free(worker_del); 
			worker_del = NULL;
		}
		
        worker = NULL;  
    }  


	return NULL;
}  

/************************************************************* 
 **函数名称: thread_create_detach 
 **功    能: 创建线程 
 **输入参数:  
 **       pool: 线程池 
 **       idx: 线程索引号 
 **输出参数: NONE 
 **返    回: 0: success !0: failed 
 ************************************************************/  
static int thread_create_detach(void *pool, int idx)
{  
	thread_pool_t *pstPool = (thread_pool_t *)pool;
    int ret = 0;  
    do  
    {
		ret = pthread_create(
        	&(pstPool->threadid[idx]),
        	NULL, 
        	thread_routine, 
        	pstPool
        );
        if(0 != ret)
        {
            if(EINTR == errno)
            {
                continue;
            }
            return -1;
        }
    }while(0);

    return 0;
}  




void *mutex_lock_create(void)
{
	int iRet = 0;
	pthread_mutex_t *mutex =
		(pthread_mutex_t *)base_malloc(sizeof(pthread_mutex_t));
	if (NULL == mutex) {
		return NULL;
	}

	if (0 != (iRet = pthread_mutex_init(mutex, NULL)))
	{
		HY_ERROR("create mutex failed");
		base_free(mutex);
		return NULL;
	}

	return mutex;
}

void mutex_lock_destroy(void *mutex)
{
	int iRet = 0;

	if (!mutex)
	{
		HY_WARN("mutex want to destroy is NULL!");
		return;
	}
	if (0 != (iRet = pthread_mutex_destroy(
			(pthread_mutex_t *)mutex
			)
		)
	)
	{
		HY_ERROR("destroy mutex failed:  '%s' (%d)", 
			strerror(iRet), iRet);
	}

	base_free(mutex);
                   
	return;
}

void mutex_lock(void *mutex)
{
	int iRet = 0;

	if (0 != (iRet = pthread_mutex_lock(
				(pthread_mutex_t *)mutex
			)
		)
	) 
	{
		HY_ERROR("lock mutex failed: '%s' (%d)", 
			strerror(iRet), iRet);
	}
}

void mutex_unlock( void *mutex)
{
	int iRet = 0;

	if (0 != (iRet = pthread_mutex_unlock((pthread_mutex_t *)mutex)))
	{
		HY_ERROR("unlock mutex failed- '%s' (%d)", strerror(iRet), iRet);
	}
}
/*************************************************************
*函数:	base_pthread_detached_create
*参数:	pid:线程ID
*		pthread_handle:线程执行函数
*		arg:线程执行函数参数
*返回值:0表示成功，非0便是失败
*描述:	创建分离线程
*************************************************************/
extern int
base_thread_create(thread_t *pid,
                  ThreadTaskFunc pthread_handle,
                  void *arg)
{
	int iRet = 0;
	if(NULL == pid || 
		NULL == pthread_handle)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	pthread_attr_t attr;
    iRet = pthread_attr_init(&attr);
    if(0 != iRet)
    {
		error_num = ThreadCreateErr;
        return ThreadCreateErr;
    }
    iRet = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(0 != iRet)
    {
    	pthread_attr_destroy(&attr);
        error_num = ThreadCreateErr;
        return ThreadCreateErr;
    }
    iRet = pthread_create(pid, &attr, pthread_handle, arg);
    if(0 != iRet)
    {
        pthread_attr_destroy(&attr);
		error_num = ThreadCreateErr;
        return ThreadCreateErr;
    }
    pthread_attr_destroy(&attr);

	return NoErr;
}


