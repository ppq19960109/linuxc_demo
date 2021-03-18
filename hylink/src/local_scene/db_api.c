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
#include <errno.h>

#include "log_api.h"
#include "db_api.h"
#include "base_api.h"
#include "scene_link_list.h"
#include "len.h"

struct DB
{
	sqlite3* fp;
	link_list_class_t *pstDbExecQueue;
	pthread_t iPidDbExec;
};


typedef struct db_exec_node_s
{
	link_list_piece_t stLinkPiece;
	char acSql[SQL_MAX_LEN];
}db_exec_node_t;


/*************************************************************
*函数:	CreateDir
*参数:	pcPathName :目录
*描述:	创建多级目录
*************************************************************/
static int CreateDir(const char *pcPathName)
{
	int i = 0;
	int iLen = 0;
	char acPatchName[PATCH_NAME_MAX_LEN] = {0};
	char *pch = NULL;
	strncpy(acPatchName, pcPathName, PATCH_NAME_MAX_LEN);

	/*找到最后一个‘/’,并在'/'后截断字符串*/
	if(NULL != (pch = strrchr(acPatchName, '/')))
	{
		*(pch + 1) = '\0';
	}
	else
	{
		return -1;
	}

	iLen = strlen(acPatchName);
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
	return 0;
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
	iRet = sqlite3_exec(db->fp, pcSql, NULL, NULL, &pcDbErrMsg);
	if(SQLITE_BUSY == iRet || SQLITE_LOCKED == iRet)
	{
		/*数据库文件或数据被锁*/
		usleep(200);
		goto db_sql_exec_repetition;
	}
	else if(SQLITE_OK != iRet)
	{
		HY_ERROR("The sql(%s) failed to execute: %s(%d)", pcSql, pcDbErrMsg, iRet);
		sqlite3_free(pcDbErrMsg);
		return -1;
	}
	return 0;
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
		HY_ERROR("Parameter error, db_add\n");
		return -1;
	}

	return db_sql_exec(db, pcSql);
}
extern int 
db_add_asyn(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		HY_ERROR("Parameter error, db_add\n");
		return -1;
	}

	int iRet = 0;
	/*添加到执行队列中*/
	db_exec_node_t *pstNewTask = 
		(db_exec_node_t *)calloc(1, sizeof(db_exec_node_t));
	if(NULL == pstNewTask)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(pstNewTask->acSql, 
		pcSql, 
		SQL_MAX_LEN);
		
	iRet = db->pstDbExecQueue->inst_tail(db->pstDbExecQueue, (link_list_piece_t *)pstNewTask);
	
	return iRet;
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
		HY_ERROR("Parameter error, db_del\n");
		return -1;
	}

	return db_sql_exec(db, pcSql);
}
extern int 
db_del_asyn(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		HY_ERROR("Parameter error, db_del\n");
		return -1;
	}

	int iRet = 0;
	/*添加到执行队列中*/
	db_exec_node_t *pstNewTask = 
		(db_exec_node_t *)calloc(1, sizeof(db_exec_node_t));
	if(NULL == pstNewTask)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(pstNewTask->acSql, 
		pcSql, 
		SQL_MAX_LEN);
		
	iRet = db->pstDbExecQueue->inst_tail(db->pstDbExecQueue, (link_list_piece_t *)pstNewTask);
	
	return iRet;
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
		HY_ERROR("Parameter error, db_set\n");
		return -1;
	}

	return db_sql_exec(db, pcSql);
}
extern int 
db_set_asyn(DB*db, const char *pcSql)
{
	if(NULL == db)
	{
		HY_ERROR("Parameter error, db_set\n");
		return -1;
	}

	int iRet = 0;
	/*添加到执行队列中*/
	db_exec_node_t *pstNewTask = 
		(db_exec_node_t *)calloc(1, sizeof(db_exec_node_t));
	if(NULL == pstNewTask)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(pstNewTask->acSql, 
		pcSql, 
		SQL_MAX_LEN);
		
	iRet = db->pstDbExecQueue->inst_tail(db->pstDbExecQueue, (link_list_piece_t *)pstNewTask);
	
	return iRet;
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
		HY_ERROR("Parameter error, db_get\n");
		return -1;
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
	iRet = sqlite3_get_table(db->fp, pcSql, &dbResult, &iRow, &iColumn, &pcDbErrMsg);
	if(SQLITE_BUSY == iRet || SQLITE_LOCKED == iRet)
	{
		/*数据库文件或数据被锁*/
		usleep(200);
		goto db_get_repetition;
	}
	else if(SQLITE_OK != iRet)  
	{  
		HY_ERROR("The sql(%s) failed to execute: %s(%d)\n", pcSql, pcDbErrMsg, iRet);
		sqlite3_free_table(dbResult);
		sqlite3_free(pcDbErrMsg);	
		pcDbErrMsg = NULL;  
		return -1;
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
					if(NULL != dbResult[iCount])
					{
						strncpy(pstData[iCount - iColumn].acData, dbResult[iCount], DB_DATA_MAX_LEN);
					}
					else
					{
						strncpy(pstData[iCount - iColumn].acData, "", DB_DATA_MAX_LEN);
					}
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
	return 0;
	
}


/*************************************************************
*函数:	db_compatibility
*参数:	db :数据库文件指针
*		iCompatibilityType:兼容性处理类型
*返回值:0表示成功，非0表示失败
*描述:	数据库兼容性处理，主要用于前后版本数据库发生改变，此处需要做兼容处理
*************************************************************/
extern int 
db_compatibility(DB*db, int iCompatibilityType)
{
	int iRet = 0;
	if(0 == iCompatibilityType)
	{
		/*兼容处理一*/
		/*处理原因：场景添加延时属性。*/
		/*数据库，scene_list表插入exec_delayed列，默认值为0*/

		/*判断scene_list表是否缺少exec_delayed列*/
		char acSql[1024] = {0};
		int iRow = 0;
		int iRank = 0;
		iRow = 512;
		iRank = 1;
		db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
		if(NULL == astData)
		{
			HY_ERROR("calloc error.\n");
			return -1;
		}
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, 1024, 
			"SELECT id FROM scene_list;");
		iRet = db_get(db, acSql, astData, &iRow, &iRank);
		
		if(0 == iRet)
		{
			/*该表已经存在*/
			iRow = 512;
			iRank = 1;
			memset(acSql, 0x0, sizeof(acSql));
			snprintf(acSql, 1024, 
				"SELECT exec_delayed FROM scene_list;");
			iRet = db_get(db, acSql, astData, &iRow, &iRank);
			
			if(0 != iRet)
			{
				/*exec_delayed列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE scene_list ADD COLUMN exec_delayed CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为exec_delayed列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE scene_list SET exec_delayed='0' WHERE exec_delayed IS NULL;");
					iRet = db_set(db, acSql);
				}
				
			}
		}
		
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
	}
	else if(1 == iCompatibilityType)
	{
		/*兼容处理二*/
		/*处理原因：场景动作添加动作标识ID、动作value编码方式属性。*/
		/*数据库，action_list表插入action_id列以及value_coding          ，action_id默认值为0, value_coding默认值为Original*/

		/*判断action_list表是否缺少action_id列*/
		char acSql[1024] = {0};
		int iRow = 0;
		int iRank = 0;
		iRow = 512;
		iRank = 1;
		db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
		if(NULL == astData)
		{
			HY_ERROR("calloc error.\n");
			return -1;
		}
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, 1024, 
			"SELECT id FROM action_list;");
		iRet = db_get(db, acSql, astData, &iRow, &iRank);
		
		if(0 == iRet)
		{
			/*该表已经存在*/
			iRow = 512;
			iRank = 1;
			memset(acSql, 0x0, sizeof(acSql));
			snprintf(acSql, 1024, 
				"SELECT action_id FROM action_list;");
			iRet = db_get(db, acSql, astData, &iRow, &iRank);
			
			if(0 != iRet)
			{
				/*action_id列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE action_list ADD COLUMN action_id CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为action_id列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE action_list SET action_id='0' WHERE action_id IS NULL;");
					iRet = db_set(db, acSql);
				}
				/*value_coding列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE action_list ADD COLUMN value_coding CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为value_coding列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE action_list SET value_coding='Original' WHERE value_coding IS NULL;");
					iRet = db_set(db, acSql);
				}
			}
		}
		
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
	}
	if(2 == iCompatibilityType)
	{
		/*兼容处理三*/
		/*处理原因：场景添加更新时间、备注属性。*/
		/*数据库，scene_list表插入update_time列、scene_note列，默认值都为0*/

		/*判断scene_list表是否缺少exec_delayed列*/
		char acSql[1024] = {0};
		int iRow = 0;
		int iRank = 0;
		iRow = 512;
		iRank = 1;
		db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
		if(NULL == astData)
		{
			HY_ERROR("calloc error.\n");
			return -1;
		}
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, 1024, 
			"SELECT id FROM scene_list;");
		iRet = db_get(db, acSql, astData, &iRow, &iRank);
		
		if(0 == iRet)
		{
			/*该表已经存在*/
			iRow = 512;
			iRank = 1;
			memset(acSql, 0x0, sizeof(acSql));
			snprintf(acSql, 1024, 
				"SELECT update_time FROM scene_list;");
			iRet = db_get(db, acSql, astData, &iRow, &iRank);
			
			if(0 != iRet)
			{
				/*update_time列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE scene_list ADD COLUMN update_time CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为exec_delayed列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE scene_list SET update_time='0' WHERE update_time IS NULL;");
					iRet = db_set(db, acSql);
				}
				/*scene_note列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE scene_list ADD COLUMN scene_note CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为scene_note列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE scene_list SET scene_note='0' WHERE scene_note IS NULL;");
					iRet = db_set(db, acSql);
				}
			}
		}
		
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
	}
	if(3 == iCompatibilityType)
	{
		/*兼容处理四*/
		/*处理原因：场景事件条件添加“TriggerType” “ContinueTime” “LastValue”属性。
		*场景时间条件添加“TriggerType” “TriggerInterval”属性
		*/
		/*数据库，cond_list表插入trigger_type列、continue_time列、trigger_interval列、last_value列，
		* 默认值分别为InstantOnce, 0, 1, 空*/

		/*判断cond_list表是否缺少trigger_type列*/
		char acSql[1024] = {0};
		int iRow = 0;
		int iRank = 0;
		iRow = 512;
		iRank = 1;
		db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
		if(NULL == astData)
		{
			HY_ERROR("calloc error.\n");
			return -1;
		}
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, 1024, 
			"SELECT id FROM cond_list;");
		iRet = db_get(db, acSql, astData, &iRow, &iRank);
		if(0 == iRet)
		{
			/*该表已经存在*/
			iRow = 512;
			iRank = 1;
			memset(acSql, 0x0, sizeof(acSql));
			snprintf(acSql, 1024, 
				"SELECT trigger_type FROM cond_list;");
			iRet = db_get(db, acSql, astData, &iRow, &iRank);
			if(0 != iRet)
			{
				/*trigger_type列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE cond_list ADD COLUMN trigger_type CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为trigger_type列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE cond_list SET trigger_type='InstantOnce' WHERE trigger_type IS NULL;");
					iRet = db_set(db, acSql);
				}

				/*continue_time列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE cond_list ADD COLUMN continue_time CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为trigger_type列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE cond_list SET continue_time='0' WHERE continue_time IS NULL;");
					iRet = db_set(db, acSql);
				}

				/*trigger_interval列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE cond_list ADD COLUMN trigger_interval CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为trigger_interval列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE cond_list SET trigger_interval='1' WHERE trigger_interval IS NULL;");
					iRet = db_set(db, acSql);
				}

				/*trigger_type列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE cond_list ADD COLUMN trigger_type CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为trigger_type列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE cond_list SET trigger_type='InstantOnce' WHERE trigger_type IS NULL;");
					iRet = db_set(db, acSql);
				}

				/*last_value列不存在，需要添加*/
				memset(acSql, 0x0, sizeof(acSql));
				snprintf(acSql, 1024, 
					"ALTER TABLE cond_list ADD COLUMN last_value CHAR(%d);", DB_DATA_MAX_LEN);
				iRet = db_set(db, acSql);
				if(0 == iRet)
				{
					/*为last_value列付初始值*/
					memset(acSql, 0x0, sizeof(acSql));
					snprintf(acSql, 1024, 
						"UPDATE cond_list SET last_value='' WHERE last_value IS NULL;");
					iRet = db_set(db, acSql);
				}
			}
		}

		if(astData)
		{
			free(astData);
			astData = NULL;
		}
	}
	return iRet;
}

static void*
_db_exec(void *arg)
{
	DB *pDb = (DB *)arg;
	
	/*将线程设置成可取消状态*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	link_list_class_t *pstQueue = 
		pDb->pstDbExecQueue;
	
		
	while(1)
	{
		/*减少cpu的占用*/
		usleep(10 * 1000);
		if(0 == pstQueue->size(pstQueue))
		{
			goto _db_exec_loop_end;
		}
		
		db_exec_node_t *pstPtr = 
			(db_exec_node_t *)(pstQueue->next(
				pstQueue,
				NULL)
			);
		db_exec_node_t *pstHead = 
			(db_exec_node_t *)(pstQueue->head(pstQueue));
		if(NULL == pstPtr || NULL == pstHead)
		{
			continue;
		}
		while(pstPtr && pstHead != pstPtr)
		{
			/*执行任务*/
			db_sql_exec(pDb, pstPtr->acSql);
			
			/*获取下一个结点*/
			db_exec_node_t *pstPtrTmp = pstPtr;
			pstPtr = 
				(db_exec_node_t *)(pstQueue->next(
					pstQueue, 
					(link_list_piece_t *)pstPtr)
				);

			/*删除当前结点*/
			pstQueue->del(pstQueue, (link_list_piece_t *)pstPtrTmp);
		}
_db_exec_loop_end:

		/*线程取消点*/
		pthread_testcancel();
	}
	return NULL;
}

/*************************************************************
*函数:	db_server_start
*参数:	pcPath :数据库文件路径
*返回值:0表示成功，非0表示失败
*描述:	打开数据库,如果数据库文件不存在则创建
*************************************************************/
extern DB* 
db_server_start(const char *pcPath)
{
	int iRet = 0;
	DB *pDb = (DB *)calloc(1, sizeof(DB));
	if(NULL == pDb)
	{
		HY_ERROR("Open DB(%s) failed.", pcPath);
		return NULL;
	}

	/*创建任务队列*/
	pDb->pstDbExecQueue = 
		new_link_list(sizeof(db_exec_node_t));
	if(NULL == pDb->pstDbExecQueue)
	{
		HY_ERROR("Db Exec Queue Error.\n");
        return NULL;
	}
	
	/*打开数据库*/
	/*判断文件是否存在*/
	if(access(pcPath, R_OK)!=0)
	{
		CreateDir(pcPath);
	}
	
	iRet = sqlite3_open(pcPath,&(pDb->fp));
	if(SQLITE_OK != iRet)
	{
		HY_ERROR("Open DB(%s) failed :%s", pcPath, sqlite3_errmsg(pDb->fp));
		return NULL;
	}

	/*创建数据库执行线程*/
	if(0 != base_thread_create(
		&pDb->iPidDbExec, 
		_db_exec, 
		pDb)
	)
	{
		HY_ERROR("pthread_attr_init Error.\n");
        return NULL;
	}
	
	return pDb;
}

/*************************************************************
*函数:	db_server_start
*参数:	pcPath :数据库文件路径
*返回值:0表示成功，非0表示失败
*描述:	打开数据库,如果数据库文件不存在则创建
*************************************************************/
extern int
db_server_stop(DB* db)
{
	if(NULL == db)
	{
		HY_ERROR("Parameter error, db_close\n");
		return -1;
	}
	
	int iRet = 0;
	/*取消线程*/
	base_thread_cancel(db->iPidDbExec);
	sleep(1);
	
	/*销毁队列*/
	destroy_link_list(db->pstDbExecQueue);

	/*关闭数据库*/
	iRet = sqlite3_close(db->fp);

	/*释放数据库描述符*/
	free(db);
	
	return iRet;
}


