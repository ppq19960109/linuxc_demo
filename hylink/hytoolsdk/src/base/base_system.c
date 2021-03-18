#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "base_api.h"

#define PIPE_READ_BUFFER_LEN 4096

/*************************************************************
*函数:	base_system
*参数:	pcCmd: 要执行的命令
*		pcResult: 命令返回结果,如果不需要关心返回结果,
*				该值可赋值为NULL,此时iBuffSize的值失去意义.
*		iBuffSize: 用于存储命令返回值的缓存大小
*描述:	system函数
*************************************************************/
extern int
base_system(char *pcCmd, char *pcResult, int iBuffSize)
{
	/*参数定义*/
	int i = 0;
	int iReadSize = 0;
	BASE_FILE *pp =NULL;
	char *pcData = NULL;
	char *pcStr = NULL;
	
	/*参数校验*/
	if(NULL == pcCmd)
	{
		error_num = ParaErr;
		return GeneralErr;
	}

	/*创建管道*/
	pp = base_popen(pcCmd, "r");
	if(!pp)
	{
		error_num = PipeOpenErr;
		return GeneralErr;
	}

	/*判断是否关心命令的执行结果*/
	if(NULL == pcResult ||
		0 == iBuffSize)
	{
		/*关闭管道*/
		base_pclose(pp);
		return NoErr;
	}

	/*获取命令的执行结果*/
	pcData = (char*)base_malloc(PIPE_READ_BUFFER_LEN);
	if(NULL == pcData)
	{
		/*关闭管道*/
		base_pclose(pp);
		return GeneralErr;
	}
	base_memset(pcData, 0x0, PIPE_READ_BUFFER_LEN);
	
	pcStr = pcData;
	do
	{
		i++;
		iReadSize = 0;
		iReadSize = base_fread (pcStr, PIPE_READ_BUFFER_LEN, 1, pp);

		if(1 != iReadSize)
		{
			break;
		}

		pcData =
			(char*)base_realloc(pcData,(i + 1) * PIPE_READ_BUFFER_LEN);
		if(NULL == pcData)
		{
			/*关闭管道*/
			base_pclose(pp);
			return GeneralErr;
		}
		pcStr = pcData + (i * PIPE_READ_BUFFER_LEN);
		base_memset(pcStr, 0x0, PIPE_READ_BUFFER_LEN);	
	}while(1);

	/*返回命令执行结果*/
	base_strncpy(pcResult, pcData, iBuffSize);

	/*关闭管道*/
	base_pclose(pp);
	base_free(pcData);

	return NoErr;
}

