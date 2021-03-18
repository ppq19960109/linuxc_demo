/***********************************************************
*文件名     : error_msg.h
*版   本   : v1.0.0.0
*日   期   : 2019.09.30
*说   明   : 错误信息
*修改记录: 
************************************************************/


#ifndef ERROR_MSG_H
#define ERROR_MSG_H

#include <errno.h>

typedef int         ERROR_STATUS;


extern ERROR_STATUS error_num;

#define NoErr                      0       //! No error occurred.
#define GeneralErr                -1       //! General error.
#define InProgressErr              1       //! Operation in progress.

// Generic error codes are in the range -251 to -500.

#define GenericErrorBase           -250   //! Starting error code for all generic errors.

#define UnknownErr                 -250   //! Unknown error occurred.
#define ParaErr                    -251   //! Parameter error.
#define HeapReqErr                 -252   //! Insufficient heap space, request failed.
#define HeapFreeErr                -253   //! Heap space release failed.
#define SystemErr                  -254   //! System error.Specific error view 'errno'
#define MutexLockErr               -255   //! The mutex failed to lock.
#define MutexUnLockErr             -256   //! Mutex unlock failed.
#define NotFoundErr                -257   //! Not Found.
#define ThreadCreateErr            -258   //! Thread creation failed.
#define SemBusyErr                 -259   //! Semaphore busy.
#define TimeOutErr                 -260   //! Time-out error.
#define ListInsertErr              -261   //! List insert failed.
#define ListDelErr                 -262   //! List deletion failed.
#define QueuePushErr               -263   //! Queue push failed.
#define QueuePopErr                -264   //! Queue Pop failed.
#define DBOpenErr                  -265   //! Failed to open database.
#define DBReadErr                  -266   //! Database read failed.
#define DBWriteErr                 -267   //! Database write failed.
#define DBDelErr                   -268   //! Database deletion failed.
#define EpollInitErr               -269   //! Epoll initialization failed.
#define EpollCtlErr                -270   //! Epoll control failed.
#define SessionOpenErr             -271   //! Session open failed.
#define SessionSendErr             -272   //! Failed to send session data.
#define SessionRecvErr             -273   //! Session data reception failed.
#define FileOpenErr                -274   //! Failed to open file.
#define FileReadErr                -275   //! File read failed.
#define FileWriteErr               -276   //! File write failed.
#define FileCloseErr               -277   //! File close failed.
#define PipeOpenErr                -278   //! Failed to open pipe.
#define PipeReadErr                -279   //! Pipe read failed.
#define PipeWriteErr               -280   //! Pipe write failed.
#define PipeCloseErr               -281   //! Pipe close failed.
#define JsonCreateErr              -282   //! Json creation failed.
#define JsonFormatErr              -283   //! Json format error.
#define JsonKeyNotFoundErr         -284   //! The json key was not found.
#define JsonValueTypeErr           -285   //! The json value type error.
#define JsonValueErr               -286   //! The json value error.

#define GenericErrorEnd            -500   //! Last generic error code (inclusive)


/*错误消息*/
#define UnknownErrMsg              "Unknown error occurred."
#define ParaErrMsg                 "Parameter error."
#define HeapReqErrMsg              "Insufficient heap space, request failed."
#define HeapFreeErrMsg             "Heap space release failed."
#define SystemErrMsg               "System error."
#define MutexLockErrMsg            "The mutex failed to lock."
#define MutexUnLockErrMsg          "Mutex unlock failed."
#define NotFoundErrMsg             "Not Found."
#define ThreadCreateErrMsg         "Thread creation failed."
#define SemBusyErrMsg              "Semaphore busy."
#define TimeOutErrMsg              "Time-out error."
#define ListInsertErrMsg           "List insert failed."
#define ListDelErrMsg              "List deletion failed."
#define QueuePushErrMsg            "Queue push failed."
#define QueuePopErrMsg             "Queue Pop failed."
#define DBOpenErrMsg               "Failed to open database."
#define DBReadErrMsg               "Database read failed."
#define DBWriteErrMsg              "Database write failed."
#define DBDelErrMsg                "Database deletion failed."
#define EpollInitErrMsg            "Epoll initialization failed."
#define EpollCtlErrMsg             "Epoll control failed."
#define SessionOpenErrMsg          "Session open failed."
#define SessionSendErrMsg          "Failed to send session data."
#define SessionRecvErrMsg          "Session data reception failed."
#define FileOpenErrMsg             "Failed to open file."
#define FileReadErrMsg             "File read failed."
#define FileWriteErrMsg            "File write failed."
#define FileCloseErrMsg            "File close failed."
#define PipeOpenErrMsg             "Failed to open pipe."
#define PipeReadErrMsg             "Pipe read failed."
#define PipeWriteErrMsg            "Pipe write failed."
#define PipeCloseErrMsg            "Pipe close failed."
#define JsonCreateErrMsg           "Json creation failed."
#define JsonFormatErrMsg           "Json format error."
#define JsonKeyNotFoundErrMsg      "The json key was not found."
#define JsonValueTypeErrMsg        "The json value type error."
#define JsonValueErrMsg            "The json value error."


/*************************************************************
*函数:	error_msg
*参数:	error_num:错误号
*返回值:返回错误消息
*描述:	获取错误号代表的错误消息
*************************************************************/
extern char * error_msg(ERROR_STATUS error_num);




#endif /*ERROR_MSG_H*/

