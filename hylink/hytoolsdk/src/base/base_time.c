#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "error_msg.h"
#include "base_api.h"

extern int base_time_get(base_timeval_t *pstTimeVal)
{
	if(NULL == pstTimeVal)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	
	int iRet = 0;
	struct timeval tv;
	iRet = gettimeofday(&tv, NULL);
	if(iRet < 0)
	{
		error_num = SystemErr;
		return SystemErr;
	}
	
	pstTimeVal->uiSec = tv.tv_sec;
	pstTimeVal->uiMsec = tv.tv_usec / 1000;
	pstTimeVal->uiUsec = tv.tv_usec % 1000;
	
	return NoErr;
}
extern int base_time_str_get(char *pacTimeStr)
{
	time_t t = time(0);
	char acTimeBuf[32];
	strftime(acTimeBuf, sizeof(acTimeBuf), "%Y-%m-%d %H:%M:%S", localtime(&t));

	strcpy(pacTimeStr, acTimeBuf);
	return NoErr;
}

extern void base_delay_s(unsigned int s)
{
	sleep(s);
}

extern void base_delay_ms(unsigned int ms)
{
	usleep(ms * 1000);
}

extern void base_delay_us(unsigned int us)
{
	usleep(us);
}

