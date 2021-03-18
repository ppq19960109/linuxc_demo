/***********************************************************
*文件名     : log_api.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.03
*说   明   : 日志系统相关接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <dirent.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "log_api.h"
#include "base_api.h"

int g_iFlieNum = 2;
int g_iFileSize = 1024 * 1024;
int g_iCount = 0;
log_level_t g_log_level;
char g_acLogFilePath[PATCH_NAME_MAX_LEN];
BASE_FILE *g_log_fp;

static char *loglevels[] = {
 "", "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"
};


/*************************************************************
*函数:	CreateDir
*参数:	pcPathName :目录
*描述:	创建多级目录
*************************************************************/
static int CreateDir(char *pcPathName)
{
	int i = 0;
	int iLen = 0;
	char acPatchName[PATCH_NAME_MAX_LEN] = {0};
	char *pch = NULL;
	base_strncpy(acPatchName, pcPathName, PATCH_NAME_MAX_LEN);

	/*找到最后一个‘/’,并在'/'后截断字符串*/
	if(NULL != (pch = base_strrchr(acPatchName, '/')))
	{
		*(pch + 1) = '\0';
	}
	else
	{
		return -1;
	}

	iLen = base_strlen(acPatchName);
	for(i = 1; i < iLen; ++i)
	{
		if('/' == acPatchName[i])
		{
			acPatchName[i] = 0;
			if(base_access(acPatchName, R_OK)!=0)
			{
				if(base_mkdir(acPatchName, 0755)==-1)
				{
					return -1;
				}
			}
		acPatchName[i] = '/';
		}
	}
	return 0;
} 

/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*描述:	初始化日志输出
*************************************************************/
extern void InitLog(log_level_t enLog_Level, char* pcLogFilePath)
{
	g_log_level = enLog_Level;
	if(NULL == pcLogFilePath)
	{
		g_log_fp = LOG_STDOUT_FD;
	}
	else
	{
		/*创建路径*/
		base_strncpy(g_acLogFilePath, pcLogFilePath, PATCH_NAME_MAX_LEN);
		CreateDir(g_acLogFilePath);
		/*创建文件*/
		g_log_fp = base_fopen(g_acLogFilePath, "a+");
		if(NULL == g_log_fp)
		{
			g_log_fp = LOG_STDOUT_FD;
		}
	}
}
/*************************************************************
*函数:	InitLogFileNum
*参数:	iFileNum :日志文件个数
*描述:	初始化日志文件个数，默认2个
*************************************************************/
extern void InitLogFileNum(int iFileNum)
{
	g_iFlieNum = iFileNum;
}
/*************************************************************
*函数:	InitLogFileSize
*参数:	iFileSize :日志文件大小（单位字节）
*描述:	初始化日志文件大小，默认1024*1024（1M）
*************************************************************/
extern void InitLogFileSize(int iFileSize)
{
	g_iFileSize = iFileSize;
}

/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*描述:	初始化日志输出
*************************************************************/
void LOG(log_level_t enLog_Level, const char *pcFormat, ...)
{
	/*判断日志等级*/
	if(g_log_level < enLog_Level)
	{
		return;
	}
	/*判断文件大小*/
	if(LOG_STDOUT_FD != g_log_fp)
	{
		if(g_iFileSize < base_ftell(g_log_fp))
		{
			base_fclose(g_log_fp);
			g_log_fp = NULL;
			if(g_iFlieNum > 1)
			{
				g_iCount ++;
				char acPatchName[PATCH_NAME_MAX_LEN] = {0};
				char acPatchNameNew[PATCH_NAME_MAX_LEN] = {0};
				base_snprintf(acPatchName, 
					PATCH_NAME_MAX_LEN,
					"%s",
					g_acLogFilePath);
				base_snprintf(acPatchNameNew, 
					PATCH_NAME_MAX_LEN,
					"%s.%d", 
					g_acLogFilePath,
					g_iCount);
				base_rename(acPatchName, acPatchNameNew);
				if(g_iCount == g_iFlieNum - 1)
				{
					g_iCount = 0;
				}
			}
			/*重新打开文件*/
			g_log_fp = base_fopen(g_acLogFilePath, "w");
			if(NULL == g_log_fp)
			{
				g_log_fp = LOG_STDOUT_FD;
			}
		}
	}

	/*日志输出*/
	va_list ap;
	char acTimeBuf[32];
	base_time_str_get(acTimeBuf);
	base_fprintf(g_log_fp, "%s [%s] ", acTimeBuf, loglevels[enLog_Level]);
	va_start(ap, pcFormat);
	vfprintf(g_log_fp, pcFormat, ap);
	va_end(ap);
	fflush(g_log_fp);
}

/*************************************************************
*函数:	LOG_HEX
*参数:	pMemory :内存指针
*		iOffset: 起始下标
*		iExtent: 打印长度
*描述:	二进制日志输出
*************************************************************/
extern void LOG_HEX (
	log_level_t enLog_Level, 
	const void * pMemory, 
	int iOffset, 
	int iExtent)
{
	/*判断日志等级*/
	if(g_log_level < enLog_Level)
	{
		return;
	}
	/*判断文件大小*/
	if(LOG_STDOUT_FD != g_log_fp)
	{
		if(g_iFileSize < base_ftell(g_log_fp))
		{
			base_fclose(g_log_fp);
			g_log_fp = NULL;
			if(g_iFlieNum > 1)
			{
				g_iCount ++;
				char acPatchName[PATCH_NAME_MAX_LEN] = {0};
				char acPatchNameNew[PATCH_NAME_MAX_LEN] = {0};
				base_snprintf(acPatchName, 
					PATCH_NAME_MAX_LEN, 
					"%s", 
					g_acLogFilePath);
				base_snprintf(acPatchNameNew, 
					PATCH_NAME_MAX_LEN,
					"%s.%d",
					g_acLogFilePath,
					g_iCount);
				base_rename(acPatchName, acPatchNameNew);
				if(g_iCount == g_iFlieNum - 1)
				{
					g_iCount = 0;
				}
			}
			/*重新打开文件*/
			g_log_fp = base_fopen(g_acLogFilePath, "w");
			if(NULL == g_log_fp)
			{
				g_log_fp = LOG_STDOUT_FD;
			}
		}
	}

	char * origin = (char *)(pMemory);
	unsigned field = sizeof (iExtent) + sizeof (iExtent);
	unsigned block = 0x10;
	int lower = block * (iOffset / block);
	int upper = block + lower;
	int index = 0;
	char buffer [sizeof (iExtent) + sizeof (iExtent) + 72];
	char * output;
	while (lower < iExtent) 
	{
		output = buffer + field;
		for (index = lower; output-- > buffer; index >>= 4) 
		{
			*output = DIGITS_HEX [index & 0x0F];
		}
		output = buffer + field;
		*output++ = ' ';
		for (index = lower; index < upper; index++) 
		{
			if (index < iOffset) 
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			else if (index < iExtent) 
			{
				*output++ = DIGITS_HEX [(origin [index] >> 4) & 0x0F];
				*output++ = DIGITS_HEX [(origin [index] >> 0) & 0x0F];
			}
			else 
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			*output++ = ' ';
		}
		for (index = lower; index < upper; index++) 
		{
			if (index < iOffset) 
			{
				*output++ = ' ';
			}
			else if (index < iExtent) 
			{
				unsigned c = origin [index];
				*output++ = base_isprint (c)? c: '.';
			}
			else 
			{
				*output++ = ' ';
			}
		}
		*output++ = '\n';
		*output++ = '\0';
		base_fputs (buffer, g_log_fp);
		lower += block;
		upper += block;
	}

	output = buffer;
	*output++ = '\n';
	*output++ = '\0';
	base_fputs (buffer, g_log_fp);

	return;

}


