/***********************************************************
*文件名     : param_check.h
*版   本   : v1.0.0.0
*日   期   : 2018.06.04
*说   明   : 参数校验宏函数接口
*修改记录: 
************************************************************/

#ifndef PARAM_CHECK_H
#define PARAM_CHECK_H

#include "log_api.h"
#include "error_msg.h"

/*************************************************************
*描述:	宏函数参数校验，指针非空，一个参数
*************************************************************/
#define PARAM_CHECK_RETURN_ERRORNO_1(param_0) do{\
	HY_TRACE("\n");\
	if(NULL == param_0)\
	{\
		return ParaErr;\
	}\
}while(0)

#define PARAM_CHECK_RETURN_NULL_1(param_0) do{\
	HY_TRACE("\n");\
	if(NULL == param_0)\
	{\
		return NULL;\
	}\
}while(0)

#define PARAM_CHECK_RETURN_VOID_1(param_0) do{\
	HY_TRACE("\n");\
	if(NULL == param_0)\
	{\
		return;\
	}\
}while(0)
/*************************************************************
*描述:	宏函数参数校验，指针非空，2个参数
*************************************************************/
#define PARAM_CHECK_RETURN_ERRORNO_2(param_0, param_1) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1)\
	{\
		return ParaErr;\
	}\
}while(0)

#define PARAM_CHECK_RETURN_NULL_2(param_0, param_1) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1)\
	{\
		return NULL;\
	}\
}while(0)

/*************************************************************
*描述:	宏函数参数校验，指针非空，3个参数
*************************************************************/
#define PARAM_CHECK_RETURN_ERRORNO_3(param_0, param_1, param_2) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1 ||\
		NULL == param_2)\
	{\
		return ParaErr;\
	}\
}while(0)
#define PARAM_CHECK_RETURN_NULL_3(param_0, param_1, param_2) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1 ||\
		NULL == param_2)\
	{\
		return NULL;\
	}\
}while(0)

/*************************************************************
*描述:	宏函数参数校验，指针非空，4个参数
*************************************************************/
#define PARAM_CHECK_RETURN_ERRORNO_4(param_0, param_1, param_2, param_3) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1 ||\
		NULL == param_2 ||\
		NULL == param_3)\
	{\
		return ParaErr;\
	}\
}while(0)
#define PARAM_CHECK_RETURN_NULL_4(param_0, param_1, param_2, param_3) do{\
	HY_TRACE("\n");\
	if(NULL == param_0 ||\
		NULL == param_1 ||\
		NULL == param_2 ||\
		NULL == param_3)\
	{\
		return NULL;\
	}\
}while(0)

/*************************************************************
*描述:	宏函数参数校验非负数，一个参数
*************************************************************/
#define PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(param_0) do{\
	HY_TRACE("\n");\
	if(param_0 < 0)\
	{\
		return ParaErr;\
	}\
}while(0)
#define PARAM_CHECK_NEGATIVE_NUMBER_RETURN_NULL_1(param_0) do{\
	HY_TRACE("\n");\
	if(param_0 < 0)\
	{\
		return NULL;\
	}\
}while(0)

/*************************************************************
*描述:	宏函数参数校验正数，一个参数
*************************************************************/
#define PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(param_0) do{\
	HY_TRACE("\n");\
	if(param_0 < 0 || 0 == param_0)\
	{\
		return ParaErr;\
	}\
}while(0)
#define PARAM_CHECK_POSITIVE_NUMBER_RETURN_NULL_1(param_0) do{\
	HY_TRACE("\n");\
	if(param_0 < 0 || 0 == param_0)\
	{\
		return NULL;\
	}\
}while(0)


#endif /* PARAM_CHECK_H */

