/***********************************************************
*文件名     : db_api.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.03
*说   明   : 数据库相关操作接口
*修改记录: 
************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/stat.h> 

#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"
#include "db_api.h"

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
		return ParaErr;
	}

	iLen = base_strlen(acPatchName);
	for(i = 1; i < iLen; ++i)
	{
		if('/' == acPatchName[i])
		{
			acPatchName[i] = 0;
			if(access(acPatchName, R_OK)!=0)
			{
				if(mkdir(acPatchName, 0755)==-1)
				{
					return -1;
				}
			}
		acPatchName[i] = '/';
		}
	}
	return NoErr;
} 


/*************************************************************
*函数:	db_open
*参数:	pcPath :数据库文件路径
*		db :数据库文件指针
*返回值:0表示成功，非0表示失败
*描述:	打开数据库,如果数据库文件不存在则创建
*************************************************************/
extern int 
db_open(char *pcPath, DB**db)
{
	if(NULL == db)
	{
		HY_ERROR("Parameter error, db_open\n");
		error_num = ParaErr;
		return ParaErr;
	}
	/*判断文件是否存在*/
	if(access(pcPath, R_OK)!=0)
	{
		CreateDir(pcPath);
	}
	
	int iRet = 0;
	iRet = sqlite3_open(pcPath,db);
	if(SQLITE_OK != iRet)
	{
		HY_ERROR("Open DB(%s) failed :%s", pcPath, sqlite3_errmsg(*db));
		sqlite3_close(*db);
		error_num = DBOpenErr;
		return DBOpenErr;
	}

	return NoErr;
}

/*************************************************************
*函数:	db_close
*参数:	pcPath :数据库文件路径
*返回值:0表示成功，非0表示失败
*描述:	关闭数据库
*************************************************************/
extern int 
db_close(DB**db)
{
	if(NULL == db)
	{
		error_num = ParaErr;
		return ParaErr;
	}
	int iRet = 0;
	iRet = sqlite3_close(*db);
	if(0 == iRet)
	{
		*db = NULL;
		return NoErr;
	}
	else
	{
		error_num = DBOpenErr;
		return DBOpenErr;
	}
}

/*************************************************************
*函数:	db_sql_exec
*参数:	db :数据库文件指针
*		pcSql:数据库语句
*返回值:0表示成功，非0表示失败
*描述:	执行增删改SQL语句
*************************************************************/
static int 
db_sql_exec(DB*db, const char *pcSql)
{
	int iRet = 0;
	char *pcDbErrMsg = NULL;
db_sql_exec_repetition:
	iRet = sqlite3_exec(db, pcSql, NULL, NULL, &pcDbErrMsg);
	if(SQLITE_BUSY == iRet || SQLITE_LOCKED == iRet)
	{
		/*数据库文件或数据被锁*/
		base_delay_us(200);
		goto db_sql_exec_repetition;
	}
	else if(SQLITE_OK != iRet)
	{
		HY_ERROR("The sql(%s) failed to execute: %s(%d)", pcSql, pcDbErrMsg, iRet);
		sqlite3_free(pcDbErrMsg);
		return GeneralErr;
	}
	//HY_INFO("Exec sql: %s\n", pcSql);
	return NoErr;
}
/*************************************************************
*函数:	db_add
*参数:	db :数据库文件指针
*		pcSql:数据库语句
*返回值:0表示成功，非0表示失败
*描述:	数据库增加条目
*************************************************************/
extern int 
db_add(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
	
	iRet = db_sql_exec(db, pcSql);
	if(0 != iRet)
	{
		error_num = DBWriteErr;
		return DBWriteErr;
	}

	return NoErr;
}
/*************************************************************
*函数:	db_del
*参数:	db :数据库文件指针
*		pcSql:数据库语句
*返回值:0表示成功，非0表示失败
*描述:	数据库删除条目
*************************************************************/
extern int 
db_del(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
	
	iRet = db_sql_exec(db, pcSql);
	if(0 != iRet)
	{
		error_num = DBDelErr;
		return DBDelErr;
	}

	return NoErr;
}

/*************************************************************
*函数:	db_set
*参数:	db :数据库文件指针
*		pcSql:数据库语句
*返回值:0表示成功，非0表示失败
*描述:	修改数据库条目
*************************************************************/
extern int 
db_set(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
	
	iRet = db_sql_exec(db, pcSql);
	if(0 != iRet)
	{
		error_num = DBWriteErr;
		return DBWriteErr;
	}

	return NoErr;
}

/*************************************************************
*函数:	db_get
*参数:	db :数据库文件指针
*		pcSql:数据库语句
*		astData:用于返回数据库数据
*		piRow:传入二维结构体数组的最大行数，传出获取的
*		
*返回值:0表示成功，非0表示失败
*描述:	查询数据库条目
*************************************************************/
extern int 
db_get(DB*db, const char *pcSql, db_data_entry_t *pstData, int* piRow, int* piRank)
{
	if(NULL == db || NULL == pstData || NULL == piRow || NULL == piRank)
	{
		error_num = ParaErr;
		return ParaErr;
	}

	int iRet = 0;
	int i = 0, j = 0;
	int iCount = 0;
	char **dbResult = NULL;
	int iRow = 0;/*结果总行数，包括表头*/
	int iColumn = 0;/*结果总列数*/
	int iMaxRow = *piRow;
	int iMaxColumn = *piRank;
	char *pcDbErrMsg = NULL;
db_get_repetition:	
	iRet = sqlite3_get_table(db, pcSql, &dbResult, &iRow, &iColumn, &pcDbErrMsg);
	if(SQLITE_BUSY == iRet || SQLITE_LOCKED == iRet)
	{
		/*数据库文件或数据被锁*/
		base_delay_us(200);
		goto db_get_repetition;
	}
	else if(SQLITE_OK != iRet)  
	{  
		HY_ERROR("The sql(%s) failed to execute: %s(%d)\n", pcSql, pcDbErrMsg, iRet);
		sqlite3_free_table(dbResult);
		sqlite3_free(pcDbErrMsg);	
		pcDbErrMsg = NULL; 
		error_num = DBReadErr;
		return DBReadErr;
	}
	else
	{
		if(0 != iRow)
		{
			*piRow = iRow;
			*piRank = iColumn;
			/*跳过表头*/
			iCount = iColumn;
			/*打印表头*/
			//HY_INFO("SQL : %s\n", pcSql);
			//HY_INFO("Query Results: \n");
			//HY_INFO("Total: %d\n", iRow);
			for(i = 0; i < iRow && i < iMaxRow; ++i)
			{
				for(j = 0; j < iColumn && j < iMaxColumn; ++j)
				{
					base_strncpy(pstData[iCount - iColumn].acData, dbResult[iCount], DB_DATA_MAX_LEN);
					//HY_INFO("%s: %s\n", dbResult[j], pstData[iCount - iColumn].acData);
					iCount++;
				}
			}
		}
		else
		{
			*piRow = 0;
			*piRank = 0;
		}
		
	}

	sqlite3_free_table(dbResult);
	return NoErr;
	
}





