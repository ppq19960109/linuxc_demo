#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#include "error_msg.h"
#include "base_api.h"


extern void *base_mutex_lock_create(void)
{
	int iRet = 0;
	pthread_mutex_t *mutex = 
		(pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (NULL == mutex) {
		error_num = HeapReqErr;
		return NULL;
	}

	if (0 != (iRet = pthread_mutex_init(mutex, NULL)))
	{
		free(mutex);
		error_num = SystemErr;
		return NULL;
	}

	return mutex;
}
extern int base_mutex_lock_destroy(void *mutex)
{
	int iRet = 0;

	if (NULL == mutex)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	iRet = pthread_mutex_destroy((pthread_mutex_t *)mutex);
	if(0 != iRet)
	{
		error_num = SystemErr;
		free(mutex);
		return SystemErr;
	}

	free(mutex);

	return NoErr;
}

extern int base_mutex_lock(void *mutex)
{
	int iRet = 0;
	if (NULL == mutex)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	iRet = pthread_mutex_lock((pthread_mutex_t *)mutex);
	if(0 != iRet)
	{
		error_num = MutexLockErr;
		return MutexLockErr;
	}
	
	return NoErr;
}

extern int base_mutex_trylock(void *mutex)
{
	int iRet = 0;
	if (NULL == mutex)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	iRet = pthread_mutex_trylock((pthread_mutex_t *)mutex);
	if(0 != iRet)
	{
		error_num = MutexLockErr;
		return MutexLockErr;
	}
	
	return NoErr;
}

extern int base_mutex_unlock( void *mutex)
{
	int iRet = 0;
	
	if (NULL == mutex)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	iRet = pthread_mutex_unlock((pthread_mutex_t *)mutex);
	if(0 != iRet)
	{
		error_num = MutexUnLockErr;
		return MutexUnLockErr;
	}
	
	return NoErr;
}


