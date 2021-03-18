#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include "error_msg.h"
#include "base_api.h"


extern void * base_sem_create (int pshared, unsigned int value)
{
	int iRet = 0;
	sem_t *sem = 
		(sem_t *)malloc(sizeof(sem_t));
	if (NULL == sem) 
	{
		error_num = HeapReqErr;
		return NULL;
	}

	if (0 != (iRet = sem_init(sem, pshared, value)))
	{
		free(sem);
		error_num = SystemErr;
		return NULL;
	}

	return sem;
}
extern int base_sem_wait(void * sem)
{
	if(NULL == sem)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	iRet = sem_wait((sem_t *)sem);
	if(0 != iRet)
	{
		error_num = SystemErr;
		return SystemErr;
	}
	
	return iRet;
}

extern int base_sem_trywait(void * sem)
{
	if(NULL == sem)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	iRet = sem_trywait((sem_t *)sem);
	if(0 != iRet && EAGAIN == errno)
	{
		error_num = SemBusyErr;
		return SemBusyErr;
	}
	else if(0 != iRet)
	{
		error_num = SystemErr;
		return SystemErr;
	}
	
	return NoErr;
}

extern int base_sem_timedwait(void *sem, base_timeval_t *pstTimeout)
{
	if(NULL == sem || 
		NULL == pstTimeout)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	struct timespec ts;
	ts.tv_sec = pstTimeout->uiSec;
	ts.tv_nsec = pstTimeout->uiMsec * 1000000 + pstTimeout->uiUsec * 1000;

	iRet = sem_timedwait(sem, &ts);
	if(0 != iRet && ETIMEDOUT == errno)
	{
		error_num = TimeOutErr;
		return TimeOutErr;
	}
	else if(0 != iRet)
	{
		error_num = SystemErr;
		return SystemErr;
	}
	return NoErr;
}

extern int base_sem_post(void * sem)
{
	if(NULL == sem)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	iRet = sem_post((sem_t *)sem);
	if(0 != iRet)
	{
		error_num = SystemErr;
		return SystemErr;
	}

	return NoErr;
}


extern int base_sem_destroy(void * sem)
{
	if(NULL == sem)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	iRet = sem_destroy((sem_t *)sem);
	if(0 != iRet)
	{
		error_num = SystemErr;
		return SystemErr;
	}

	free(sem);
	sem = NULL;

	return NoErr;
}


