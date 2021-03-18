/***********************************************************
*文件名     : error_msg.c
*版   本   : v1.0.0.0
*日   期   : 2019.09.30
*说   明   : 错误信息
*修改记录: 
************************************************************/
#include <string.h>

#include "error_msg.h"
#include "log_api.h"
#include "base_api.h"

#define ERROR_MSG_MAX_LEN 256

ERROR_STATUS error_num = NoErr;
char error_msg_str[ERROR_MSG_MAX_LEN] = {0};

/*************************************************************
*函数:	error_msg
*参数:	error_num:错误号
*返回值:返回错误消息
*描述:	获取错误号代表的错误消息
*************************************************************/
char * error_msg(ERROR_STATUS error_num)
{
	switch(error_num)
	{
		case UnknownErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", UnknownErrMsg);
			break;
		case ParaErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", ParaErrMsg);
			break;
		case HeapReqErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", HeapReqErrMsg);
			break;
		case HeapFreeErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", HeapFreeErrMsg);
			break;
		case SystemErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				SystemErrMsg, strerror(errno), errno);
			break;
		case MutexLockErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", MutexLockErrMsg);
			break;
		case MutexUnLockErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", MutexUnLockErrMsg);
			break;
		case NotFoundErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", NotFoundErrMsg);
			break;
		case ThreadCreateErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				ThreadCreateErrMsg, strerror(errno), errno);
			break;
		case SemBusyErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", SemBusyErrMsg);
			break;
		case TimeOutErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", TimeOutErrMsg);
			break;
		case ListInsertErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", ListInsertErrMsg);
			break;
		case ListDelErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", ListDelErrMsg);
			break;
		case QueuePushErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", QueuePushErrMsg);
			break;
		case QueuePopErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", QueuePopErrMsg);
			break;
		case DBOpenErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", DBOpenErrMsg);
			break;
		case DBReadErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", DBReadErrMsg);
			break;
		case DBWriteErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", DBWriteErrMsg);
			break;
		case DBDelErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", DBDelErrMsg);
			break;
		case EpollInitErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				EpollInitErrMsg, strerror(errno), errno);
			break;
		case EpollCtlErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				EpollCtlErrMsg, strerror(errno), errno);
			break;
		case SessionOpenErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", SessionOpenErrMsg);
			break;
		case SessionSendErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", SessionSendErrMsg);
			break;
		case SessionRecvErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", SessionRecvErrMsg);
			break;
		case FileOpenErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				FileOpenErrMsg, strerror(errno), errno);
			break;
		case FileReadErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				FileReadErrMsg, strerror(errno), errno);
			break;
		case FileWriteErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				FileWriteErrMsg, strerror(errno), errno);
			break;
		case FileCloseErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				FileCloseErrMsg, strerror(errno), errno);
			break;
		case PipeOpenErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				PipeOpenErrMsg, strerror(errno), errno);
			break;
		case PipeReadErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				PipeReadErrMsg, strerror(errno), errno);
			break;
		case PipeWriteErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				PipeWriteErrMsg, strerror(errno), errno);
			break;
		case PipeCloseErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s, %s(%d).\n", 
				PipeCloseErrMsg, strerror(errno), errno);
			break;
		case JsonCreateErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", JsonCreateErrMsg);
			break;
		case JsonFormatErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", JsonFormatErrMsg);
			break;
		case JsonKeyNotFoundErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", JsonKeyNotFoundErrMsg);
			break;
		case JsonValueTypeErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", JsonValueTypeErrMsg);
			break;
		case JsonValueErr:
			base_snprintf(error_msg_str, ERROR_MSG_MAX_LEN, "%s\n", JsonValueErrMsg);
			break;
	}
	return error_msg_str;
}


