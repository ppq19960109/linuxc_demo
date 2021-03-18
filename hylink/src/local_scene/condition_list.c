/***********************************************************
*文件名    : condition_list.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.04
*说   明   : 条件接口，条件分两种，时间条件以及时间条件
*修改记录: 
************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

		
#include "param_check.h"
#include "condition_list.h"
#include "tool.h"

/*数据库条件表名*/
#define DB_TABLE_NAME_COND		"cond_list"

static char* apcWeek[] = {WEEK_SUN, WEEK_MON, WEEK_TUE, WEEK_WED, WEEK_THU, WEEK_FIR, WEEK_SAT};

#pragma pack(1)

/*私有方法定义*/
typedef struct cond_list_private_methods_s  
{  
	int (*cond_add_db)(struct cond_list_class_s *, void *);
	int (*cond_del_db)(struct cond_list_class_s *, void *);
	int (*cond_set_db)(struct cond_list_class_s *, void *);
	int (*cond_get_db)(struct cond_list_class_s *, void *);
	int (*cond_get_list_db)(struct cond_list_class_s *, void *, int*);

	int (*cond_list_update_index)(struct cond_list_class_s *);

	int (*time_cond_time_lose_set_db)(struct cond_list_class_s *,	time_cond_t *);
}cond_list_private_methods_t; 
#pragma pack()

/*打印信息*/
static int 
print_time_cond_info_handle(link_list_piece_t *pNode)
{
	int iCount = 0;
	cond_list_node_t *p = (cond_list_node_t *)pNode;
	cond_set_t *pstInfo = (cond_set_t *)&(p->stCondSet);
	
	time_cond_node_t *p1 = (time_cond_node_t *)(pstInfo->pstCond);
	while(p1)
	{
		HY_INFO("\t%s\t%s\t%s:%s\t%s:%s\t%s\t%s\n", 
			pstInfo->uiCondTrue & 0x1 << iCount ? "true" : "false",
			p1->stCond.acTimeKey,
			p1->stCond.acStartHour,
			p1->stCond.acStartMinu,
			p1->stCond.acEndHour,
			p1->stCond.acEndMinu,
			p1->stCond.acWeek,
			p1->stCond.acRepeat,
			p1->stCond.acLose);
		p1 = p1->next;
		iCount ++;
	}
	
	return NoErr;
}

/*打印信息*/
static int 
print_event_cond_info_handle(link_list_piece_t *pNode)
{
	int iCount = 0;
	cond_list_node_t *p = (cond_list_node_t *)pNode;
	cond_set_t *pstInfo = (cond_set_t *)&(p->stCondSet);
	event_cond_node_t *p1 = (event_cond_node_t *)(pstInfo->pstCond);
	while(p1)
	{
		HY_INFO("\t%s\t%s\t%s\t%s\t%s\n", 
			pstInfo->uiCondTrue & 0x1 << iCount ? "true" : "false",
			p1->stCond.acDevId,
			p1->stCond.acKey,
			p1->stCond.acValue,
			p1->stCond.acActive);
		p1 = p1->next;
		iCount ++;
	}
	return NoErr;
}

static int 
time_cond_time_lose_set_db(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};


	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET  time_lose='%s'\
		WHERE id='%s' and logic='%s' and cond_type='%s' and time_key='%s';\
		", 
		DB_TABLE_NAME_COND,
		pstCondInfo->acLose,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTimeKey);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneSetErr;
	}

	
	return NoErr;
}


static int 
time_cond_add_db(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
		

	/*未查找到条目，使用insert语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		INSERT INTO %s \
			(id, logic, cond_type, time_key, trigger_type, trigger_interval, start_hour, start_minu, end_hour, end_minu, week, repeat, time_lose) \
		VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');\
		", 
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTimeKey,
		pstCondInfo->acTriggerType,
		pstCondInfo->acTriggerInterval,
		pstCondInfo->acStartHour,
		pstCondInfo->acStartMinu, 
		pstCondInfo->acEndHour, 
		pstCondInfo->acEndMinu, 
		pstCondInfo->acWeek, 
		pstCondInfo->acRepeat,
		pstCondInfo->acLose
	);
	iRet = db_add_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("INSERT DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneAddErr;
	}
	
	return NoErr;
}



static int 
event_cond_add_db(
	cond_list_class_t *_this, 
	event_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	

	/*未查找到条目，使用insert语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		INSERT INTO %s \
			(id, logic, cond_type, trigger_type, continue_time, dev_id, key, value, active, last_value) \
		VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');\
		", 
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTriggerType,
		pstCondInfo->acContinueTime,
		pstCondInfo->acDevId,
		pstCondInfo->acKey, 
		pstCondInfo->acValue, 
		pstCondInfo->acActive,
		pstCondInfo->acLastValue
	);
	iRet = db_add_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("INSERT DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneAddErr;
	}
	
	return NoErr;
}

static int 
cond_add_db(
	cond_list_class_t *_this, 
	void *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_add_db(_this, (time_cond_t *)pstCondInfo);
	}
	else
	{
		return event_cond_add_db(_this, (event_cond_t *)pstCondInfo);
	}
}


static int 
time_cond_del_db(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE id='%s' and logic='%s' and cond_type='%s' and time_key='%s';\
		", 
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTimeKey);
	iRet = db_del_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneDelErr;
	}

	return NoErr;
}


static int 
event_cond_del_db(
	cond_list_class_t *_this, 
	event_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE id='%s' and logic='%s' and cond_type='%s' and dev_id='%s' and key='%s';\
		", 
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acDevId,
		pstCondInfo->acKey);
	iRet = db_del_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneDelErr;
	}

	return NoErr;
}

static int 
cond_del_db(
	cond_list_class_t *_this, 
	void *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_del_db(_this, (time_cond_t *)pstCondInfo);
	}
	else
	{
		return event_cond_del_db(_this, (event_cond_t *)pstCondInfo);
	}
}


static int 
time_cond_set_db(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}

	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
		

	/*查找到条目，使用update语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET trigger_type='%s', trigger_interval='%s', start_hour='%s', start_minu='%s', end_hour='%s', end_minu='%s', week='%s', repeat='%s', time_lose='%s'\
		WHERE id='%s' and logic='%s' and cond_type='%s' and time_key='%s';\
		", 
		DB_TABLE_NAME_COND,
		pstCondInfo->acTriggerType,
		pstCondInfo->acTriggerInterval,
		pstCondInfo->acStartHour,
		pstCondInfo->acStartMinu, 
		pstCondInfo->acEndHour, 
		pstCondInfo->acEndMinu, 
		pstCondInfo->acWeek, 
		pstCondInfo->acRepeat,
		pstCondInfo->acLose,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTimeKey);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneAddErr;
	}
	
	return NoErr;
}



static int 
event_cond_set_db(
	cond_list_class_t *_this, 
	event_cond_t *pstCondInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	
	/*查找到条目，使用update语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET trigger_type='%s', continue_time='%s', value='%s', active='%s', last_value='%s'\
		WHERE id='%s' and logic='%s' and cond_type='%s' and dev_id='%s' and key='%s';\
		", 
		DB_TABLE_NAME_COND,
		pstCondInfo->acTriggerType,
		pstCondInfo->acContinueTime,
		pstCondInfo->acValue,
		pstCondInfo->acActive, 
		pstCondInfo->acLastValue,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acDevId, 
		pstCondInfo->acKey);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_COND);
		return SceneAddErr;
	}
	
	return NoErr;
}

static int 
cond_set_db(
	cond_list_class_t *_this, 
	void *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_set_db(_this, (time_cond_t *)pstCondInfo);
	}
	else
	{
		return event_cond_set_db(_this, (event_cond_t *)pstCondInfo);
	}
}

static int 
time_cond_get_db(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 9;
	db_data_entry_t astData[iRow * iRank];
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT trigger_type, trigger_interval, start_hour, start_minu, end_hour, end_minu, week, repeat, time_lose FROM %s\
		WHERE id='%s' and logic='%s' and cond_type='%s' and time_key='%s';",
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acTimeKey);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_COND);
		return SceneGetErr;
	}

	if(NoErr == iRow)
	{
		HY_ERROR("The CondInfo(%s) not found.\n",
			pstCondInfo->acTimeKey);
		return NotFoundErr;
	}

	strncpy(pstCondInfo->acTriggerType, astData[0].acData, TRIGGER_TYPE_MAX_LEN);
	strncpy(pstCondInfo->acTriggerInterval, astData[1].acData, TRIGGER_INTERVAL_MAX_LEN);
	strncpy(pstCondInfo->acStartHour, astData[2].acData, HOUR_MAX_LEN);
	strncpy(pstCondInfo->acStartMinu, astData[3].acData, MINU_MAX_LEN);
	strncpy(pstCondInfo->acEndHour, astData[4].acData, HOUR_MAX_LEN);
	strncpy(pstCondInfo->acEndMinu, astData[5].acData, MINU_MAX_LEN);
	strncpy(pstCondInfo->acWeek, astData[6].acData, WEEK_MAX_LEN);
	strncpy(pstCondInfo->acRepeat, astData[7].acData, STATE_MAX_LEN);
	strncpy(pstCondInfo->acLose, astData[8].acData, STATE_MAX_LEN);

	return NoErr;
}


static int 
event_cond_get_db(
	cond_list_class_t *_this, 
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 5;
	db_data_entry_t astData[iRow * iRank];
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT trigger_type, continue_time, value, active, last_value FROM %s\
		WHERE id='%s' and logic='%s' and cond_type='%s' and dev_id='%s' and key='%s';",
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType,
		pstCondInfo->acDevId, 
		pstCondInfo->acKey);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_COND);
		return SceneGetErr;
	}

	if(NoErr == iRow)
	{
		HY_ERROR("The CondInfo(%s %s) not found.\n",
			pstCondInfo->acDevId, pstCondInfo->acKey);
		return NotFoundErr;
	}
	strncpy(pstCondInfo->acTriggerType, astData[0].acData, TRIGGER_TYPE_MAX_LEN);
	strncpy(pstCondInfo->acContinueTime, astData[1].acData, CONTINUE_TIME_MAX_LEN);
	strncpy(pstCondInfo->acValue, astData[2].acData, KEY_MAX_LEN);
	strncpy(pstCondInfo->acActive, astData[3].acData, ACTIVE_MAX_LEN);
	strncpy(pstCondInfo->acLastValue, astData[4].acData, KEY_MAX_LEN);
	return NoErr;
}


static int 
cond_get_db(cond_list_class_t *_this, void *pstCondInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_get_db(_this, (time_cond_t *)pstCondInfo);
	}
	else
	{
		return event_cond_get_db(_this, (event_cond_t *)pstCondInfo);
	}
}



static int 
time_cond_get_list_db(
	cond_list_class_t *_this, 
	time_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastCondInfo, piCount);
	
	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	time_cond_t *pstEntry = pastCondInfo;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = COND_MAX_NUM;
	iRank = 10;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
	
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT time_key, trigger_type, trigger_interval, start_hour, start_minu, end_hour, end_minu, week, repeat, time_lose FROM %s\
		WHERE id='%s' and logic='%s' and cond_type='%s';",
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType
	);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_COND);
		*piCount = 0;
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
		return NotFoundErr;
	}
	
	for(i = 0; i < iRow && iInfoCount < iInfoMax; ++i)
	{
		if(strcmp(astData[(iRank * i) + 0].acData, ""))
		{
			strncpy(pstEntry->acTimeKey, 
				astData[(iRank * i) + 0].acData, KEY_MAX_LEN);
			strncpy(pstEntry->acTriggerType, 
				astData[(iRank * i) + 1].acData, TRIGGER_TYPE_MAX_LEN);
			strncpy(pstEntry->acTriggerInterval, 
				astData[(iRank * i) + 2].acData, TRIGGER_INTERVAL_MAX_LEN);
			strncpy(pstEntry->acStartHour, 
				astData[(iRank * i) + 3].acData, HOUR_MAX_LEN);
			strncpy(pstEntry->acStartMinu, 
				astData[(iRank * i) + 4].acData, MINU_MAX_LEN);
			strncpy(pstEntry->acEndHour, 
				astData[(iRank * i) + 5].acData, HOUR_MAX_LEN);
			strncpy(pstEntry->acEndMinu, 
				astData[(iRank * i) + 6].acData, MINU_MAX_LEN);
			strncpy(pstEntry->acWeek, 
				astData[(iRank * i) + 7].acData, WEEK_MAX_LEN);
			strncpy(pstEntry->acRepeat, 
				astData[(iRank * i) + 8].acData, STATE_MAX_LEN);
			strncpy(pstEntry->acLose, 
				astData[(iRank * i) + 9].acData, STATE_MAX_LEN);
			iInfoCount ++;
			pstEntry ++;
		}
	}
	*piCount = iInfoCount;
	if(astData)
	{
		free(astData);
		astData = NULL;
	}
	return NoErr;
}


static int 
event_cond_get_list_db(
	cond_list_class_t *_this,
	event_cond_t *pastCondInfo,
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastCondInfo, piCount);
	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	event_cond_t *pstEntry = pastCondInfo;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = COND_MAX_NUM;
	iRank = 7;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT trigger_type, continue_time, dev_id, key, value, active, last_value FROM %s\
		WHERE id='%s' and logic='%s' and cond_type='%s';",
		DB_TABLE_NAME_COND,
		_this->stCondInfo.acId,
		_this->stCondInfo.acLogic,
		_this->stCondInfo.acCondType
	);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_COND);
		*piCount = 0;
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
		return NotFoundErr;
	}
	
	for(i = 0; i < iRow && iInfoCount < iInfoMax; ++i)
	{
		if(strcmp(astData[(iRank * i) + 0].acData, ""))
		{
			strncpy(pstEntry->acTriggerType, 
				astData[(iRank * i) + 0].acData, TRIGGER_TYPE_MAX_LEN);
			strncpy(pstEntry->acContinueTime, 
				astData[(iRank * i) + 1].acData, CONTINUE_TIME_MAX_LEN);
			strncpy(pstEntry->acDevId, 
				astData[(iRank * i) + 2].acData, DEV_ID_MAX_LEN);
			strncpy(pstEntry->acKey, 
				astData[(iRank * i) + 3].acData, KEY_MAX_LEN);
			strncpy(pstEntry->acValue, 
				astData[(iRank * i) + 4].acData, VALUE_MAX_LEN);
			strncpy(pstEntry->acActive, 
				astData[(iRank * i) + 5].acData, ACTIVE_MAX_LEN);
			strncpy(pstEntry->acLastValue, 
				astData[(iRank * i) + 6].acData, ACTIVE_MAX_LEN);
			iInfoCount ++;
			pstEntry ++;
		}
	}
	*piCount = iInfoCount;
	if(astData)
	{
		free(astData);
		astData = NULL;
	}
	return NoErr;
}


static int 
cond_get_list_db(
	cond_list_class_t *_this, 
	void *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_get_list_db(_this, pastCondInfo, piCount);
	}
	else
	{
		return event_cond_get_list_db(_this, pastCondInfo, piCount);
	}
}


static int 
cond_list_update_index(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	int i = 0;
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this
			)
		);
	
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		pstNode->iCondSetIndex = i++;			
		pstNode = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	return NoErr;
}


static int 
cond_size(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->iCount;
}



static int
time_cond_add(
	cond_list_class_t *_this, 
	time_cond_t *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);
	int iRet = 0;
	int iFlag = 0;
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)(_this->acCondlistPrivateMethods);
	
	/*添加*/
	/*查找空闲位置*/
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this
			)
		);
	
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		if(pstNode->stCondSet.ucCondCount < 32)
		{
			/*有空闲位置*/
			iFlag = 1;
			break;
		}
					
		pstNode = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	if(0 == iFlag)
	{
		/*现有的条件集合已满，需要添加新的条件集合*/
		/*判断条件集合个数是否超过32个，如果超过，该条件无法添加*/
		if(_this->stList.size((link_list_class_t *)_this) >= 32)
		{
			HY_ERROR("The list of conditions is full.\n");
			return SceneAddErr;
		}
		/*没有空闲位置，添加条件集合*/
		_this->stList.inst_tail(
			(link_list_class_t *)_this,
			_this->stList.new_node((link_list_class_t *)_this)
		);
		pstNode = 
			(cond_list_node_t *)(_this->stList.tail(
				(link_list_class_t *)_this)
			);
		/*更新条件集合结点下标，保证条件集合编号从0开始依次增加*/
		pPrivateMethods->cond_list_update_index(_this);
	}

	/*添加条件*/
	/*创建条件结点*/
	time_cond_node_t *pNew =
		(time_cond_node_t *)calloc(1, sizeof(time_cond_node_t));
	memcpy(&pNew->stCond , pstCondInfo, sizeof(time_cond_t));
	/*将条件结点添加到条件集合中*/
	time_cond_node_t *p = pstNode->stCondSet.pstCond;
	if(NULL == p)
	{
		p = pNew;
		pNew->iCondIndex = 0;
		pstNode->stCondSet.pstCond = p;
	}
	else
	{
		while(p->next)
		{
			p = p->next;
		}
		
		pNew->next = p->next;
		pNew->prev = p;
		p->next = pNew;
		pNew->iCondIndex =  p->iCondIndex + 1;
	}
	pstNode->stCondSet.ucCondCount++;
	_this->iCount++;

	if(1 == _this->iCount)
	{
		/*此时说明条件列表由空变为非空，则该条件列表的默认真假状态更改为0*/
		_this->uiCondSetTrue = 0;
	}

	/*清空相关真假标志位*/
	/*新添加的条件默认为假*/
	

	
	/*生成条件指针信息*/
	pstCondPtr->pstCondList = (void*)_this;
	pstCondPtr->pCondSetNode = (void*)pstNode;
	pstCondPtr->pCondNode = (void*)pNew;
		


	/*数据库更新*/
	iRet = 
		pPrivateMethods->cond_add_db(_this, pstCondInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Cond Info set failed.\n");
		return iRet;
	}

	return iRet;
}

static int 
event_cond_add(
	cond_list_class_t *_this, 
	event_cond_t *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);
	int iRet = 0;
	int iFlag = 0;
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)(_this->acCondlistPrivateMethods);
	
	/*添加*/
	/*查找空闲位置*/
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this
			)
		);
	
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		if(pstNode->stCondSet.ucCondCount < 32)
		{
			/*有空闲位置*/
			iFlag = 1;
			break;
		}
					
		pstNode = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	if(0 == iFlag)
	{
		/*判断结点个数是否超过32个，如果超过，该条件无法添加*/
		if(_this->stList.size((link_list_class_t *)_this) >= 32)
		{
			HY_ERROR("The list of conditions is full.\n");
			return SceneAddErr;
		}
		
		/*没有空闲位置，添加结点*/
		_this->stList.inst_head(
			(link_list_class_t *)_this,
			_this->stList.new_node((link_list_class_t *)_this)
		);
		pstNode = 
			(cond_list_node_t *)(_this->stList.first(
				(link_list_class_t *)_this)
			);
		/*更新结点下标*/
		pPrivateMethods->cond_list_update_index(_this);
	}
	/*添加*/
	/*生成新结点*/
	event_cond_node_t *pNew =
		(event_cond_node_t *)calloc(1, sizeof(event_cond_node_t));
	memcpy(&pNew->stCond , pstCondInfo, sizeof(event_cond_t));
	/*将结点添加到链表中*/
	event_cond_node_t *p = pstNode->stCondSet.pstCond;
	if(NULL == p)
	{
		p = pNew;
		pNew->iCondIndex = 0;
		pstNode->stCondSet.pstCond = p;
	}
	else
	{
		while(p->next)
		{
			p = p->next;
		}
		
		pNew->next = p->next;
		pNew->prev = p;
		p->next = pNew;
		pNew->iCondIndex = p->iCondIndex + 1;
	}
	
	pstNode->stCondSet.ucCondCount++;
	_this->iCount++;
	
	if(1 == _this->iCount)
	{
		/*此时说明条件列表由空变为非空，则该条件列表的默认真假状态更改为0*/
		_this->uiCondSetTrue = 0;
	}
	
	/*生成指针信息*/
	pstCondPtr->pstCondList = (void*)_this;
	pstCondPtr->pCondSetNode = (void*)pstNode;
	pstCondPtr->pCondNode = (void*)pNew;
		
	/*数据库更新*/
	iRet = 
		pPrivateMethods->cond_add_db(_this, pstCondInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Cond Info set failed.\n");
		return iRet;
	}

	return iRet;
}

static int 
cond_add(
	cond_list_class_t *_this,
	void *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_add(_this, (time_cond_t *)pstCondInfo, pstCondPtr);
	}
	else
	{
		return event_cond_add(_this, (event_cond_t *)pstCondInfo, pstCondPtr);
	}
}

static int 
time_cond_del(
	cond_list_class_t *_this,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondPtr);

	int iRet = 0;
	
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)_this->acCondlistPrivateMethods;
	
	cond_list_node_t *pstNode = pstCondPtr->pCondSetNode;
	if(1 == pstNode->stCondSet.ucCondCount)
	{
		/*由于当前的条件集合仅有这一个条件，所以删除条件后，还需要删除该条件集合*/
		time_cond_node_t *p = (time_cond_node_t *)pstCondPtr->pCondNode;
		/*删除数据库*/
		iRet = 
			pPrivateMethods->cond_del_db(_this, &p->stCond);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Cond Info del failed.\n");
			return SceneDelErr;
		}
		/*删除条件*/
		time_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		_this->iCount--;
		
		if(0 == _this->iCount)
		{
			/*此时说明条件列表由非空变为空，则该条件列表的真假状态更改为默认真假*/
			if(!strncmp(_this->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
			{
				_this->uiCondSetTrue = 1;
			}
			else
			{
				_this->uiCondSetTrue = 0;
			}
		}
		/*删除条件集合*/
		iRet = 
			_this->stList.del(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode
			);
		/*更新条件集合下标*/
		pPrivateMethods->cond_list_update_index(_this);

		
	}
	else
	{
		time_cond_node_t *p = (time_cond_node_t *)pstCondPtr->pCondNode;
		/*删除数据库*/
		iRet = 
			pPrivateMethods->cond_del_db(_this, &p->stCond);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Cond Info del failed.\n");
			return SceneDelErr;
		}
		
		/*删除条件*/
		if(NULL == p->prev)
		{
			/*更新条件结点下标*/
			while(p)
			{
				p->iCondIndex--;
				p = p->next;
			}
			p = (time_cond_node_t *)pstCondPtr->pCondNode;

			
			/*头结点*/
			pstNode->stCondSet.pstCond = p->next;
			if(p->next)
			{
				p->next->prev = NULL;
			}
			free(p);
		}
		else if(NULL == p->next)
		{
			/*尾结点*/
			p->prev->next = NULL;
			free(p);
		}
		else
		{
			/*更新条件结点下标*/
			while(p)
			{
				p->iCondIndex--;
				p = p->next;
			}
			p = (time_cond_node_t *)pstCondPtr->pCondNode;

			
			p->next->prev = p->prev;
			p->prev->next = p->next;

			free(p);
		}
		pstNode->stCondSet.ucCondCount --;
		_this->iCount--;
	}
	return iRet;
}

static int 
event_cond_del(cond_list_class_t *_this, ptr_info_t *pstCondPtr)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCondPtr);

	int iRet = 0;
	
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)_this->acCondlistPrivateMethods;
	
	
	cond_list_node_t *pstNode = pstCondPtr->pCondSetNode;
	if(pstNode->stCondSet.ucCondCount == 1)
	{
		/*由于当前的条件集合仅有这一个条件，所以删除条件后，还需要删除该条件集合*/
		event_cond_node_t *p = (event_cond_node_t *)pstCondPtr->pCondNode;
		/*删除数据库*/
		iRet = 
			pPrivateMethods->cond_del_db(_this, &p->stCond);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Cond Info del failed.\n");
			return SceneDelErr;
		}
		/*删除条件*/
		event_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		_this->iCount--;
		if(0 == _this->iCount)
		{
			/*此时说明条件列表由非空变为空，则该条件列表的真假状态更改为默认真假*/
			if(!strncmp(_this->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
			{
				_this->uiCondSetTrue = 1;
			}
			else
			{
				_this->uiCondSetTrue = 0;
			}
		}
		/*删除结点*/
		iRet = 
			_this->stList.del(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode
			);
		/*更新结点下标*/
		pPrivateMethods->cond_list_update_index(_this);
	}
	else
	{
		event_cond_node_t *p = (event_cond_node_t *)pstCondPtr->pCondNode;
		/*删除数据库*/
		iRet = 
			pPrivateMethods->cond_del_db(_this, &p->stCond);
		if(NoErr != iRet)
		{
			HY_ERROR("DB ActionInfo del failed.\n");
			return SceneDelErr;
		}
		/*删除条件*/
		if(NULL == p->prev)
		{
			/*更新条件结点下标*/
			while(p)
			{
				p->iCondIndex--;
				p = p->next;
			}
			p = (event_cond_node_t *)pstCondPtr->pCondNode;

			
			/*头结点*/
			pstNode->stCondSet.pstCond = p->next;
			if(p->next)
			{
				p->next->prev = NULL;
			}
			free(p);
		}
		else if(NULL == p->next)
		{
			/*尾结点*/
			p->prev->next = NULL;
			free(p);
		}
		else
		{
			/*更新下标*/
			while(p)
			{
				p->iCondIndex--;
				p = p->next;
			}
			p = (event_cond_node_t *)(pstCondPtr->pCondNode);

			
			p->next->prev = p->prev;
			p->prev->next = p->next;

			free(p);
		}
		pstNode->stCondSet.ucCondCount --;
		_this->iCount--;
	}
	return iRet;
}

static int 
cond_del(
	cond_list_class_t *_this, 
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_del(_this, pstCondPtr);
	}
	else
	{
		return event_cond_del(_this, pstCondPtr);
	}
}


static int 
time_cond_set(
	cond_list_class_t *_this,
	time_cond_t *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);
	int iRet = 0;
	
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)(_this->acCondlistPrivateMethods);
	
	/*设置*/
	time_cond_node_t *pstCondNode = (time_cond_node_t*)(pstCondPtr->pCondNode);
	time_cond_t *pstCond = &pstCondNode->stCond;
	strncpy(pstCond->acTriggerType, pstCondInfo->acTriggerType, TRIGGER_TYPE_MAX_LEN);
	strncpy(pstCond->acTriggerInterval, pstCondInfo->acTriggerInterval, TRIGGER_INTERVAL_MAX_LEN);
	strncpy(pstCond->acStartHour, pstCondInfo->acStartHour, HOUR_MAX_LEN);
	strncpy(pstCond->acStartMinu, pstCondInfo->acStartMinu, MINU_MAX_LEN);
	strncpy(pstCond->acEndHour, pstCondInfo->acEndHour, HOUR_MAX_LEN);
	strncpy(pstCond->acEndMinu, pstCondInfo->acEndMinu, MINU_MAX_LEN);
	strncpy(pstCond->acWeek, pstCondInfo->acWeek, WEEK_MAX_LEN);
	strncpy(pstCond->acRepeat, pstCondInfo->acRepeat, STATE_MAX_LEN);

	/*时间条件发生改变，需要将有效无效标志位置为有效*/
	strncpy(pstCond->acLose, "0", STATE_MAX_LEN);
	
	/*数据库更新*/
	iRet = 
		pPrivateMethods->cond_set_db(_this, pstCondInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Cond Info set failed.\n");
		return iRet;
	}

	return iRet;
}

static int 
event_cond_set(
	cond_list_class_t *_this,
	event_cond_t *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);
	int iRet = 0;
	
	cond_list_private_methods_t *pPrivateMethods = 
			(cond_list_private_methods_t *)(_this->acCondlistPrivateMethods);

	/*设置*/
	event_cond_node_t *pstCondNode = (event_cond_node_t*)(pstCondPtr->pCondNode);
	event_cond_t *pstCond = &pstCondNode->stCond;
	strncpy(pstCond->acTriggerType, pstCondInfo->acTriggerType, TRIGGER_TYPE_MAX_LEN);
	strncpy(pstCond->acContinueTime, pstCondInfo->acContinueTime, CONTINUE_TIME_MAX_LEN);
	strncpy(pstCond->acValue, pstCondInfo->acValue, VALUE_MAX_LEN);
	strncpy(pstCond->acActive, pstCondInfo->acActive, ACTIVE_MAX_LEN);
	strncpy(pstCond->acLastValue, pstCondInfo->acLastValue, VALUE_MAX_LEN);

	/*数据库更新*/
	iRet = 
		pPrivateMethods->cond_set_db(_this, pstCondInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Cond Info set failed.\n");
		return iRet;
	}

	return iRet;
}

static int 
cond_set(
	cond_list_class_t *_this, 
	void *pstCondInfo,
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_set(_this, (time_cond_t *)pstCondInfo, pstCondPtr);
	}
	else
	{
		return event_cond_set(_this, (event_cond_t *)pstCondInfo, pstCondPtr);
	}
}


static int
time_cond_get(
	cond_list_class_t *_this, 
	time_cond_t *pstCondInfo, 
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);

	/*获取*/
	time_cond_node_t *pCondNode = (time_cond_node_t *)(pstCondPtr->pCondNode);
	memcpy(pstCondInfo, &pCondNode->stCond, sizeof(time_cond_t));

	return NoErr;
}

static int
event_cond_get(
	cond_list_class_t *_this,
	event_cond_t *pstCondInfo, 
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCondInfo, pstCondPtr);

	/*获取*/
	event_cond_node_t *pCondNode = (event_cond_node_t *)(pstCondPtr->pCondNode);
	memcpy(pstCondInfo, &pCondNode->stCond, sizeof(event_cond_t));

	return NoErr;
}

static int
cond_get(
	cond_list_class_t *_this,
	void *pstCondInfo, 
	ptr_info_t *pstCondPtr
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_get(_this, (time_cond_t *)pstCondInfo, pstCondPtr);
	}
	else
	{
		return event_cond_get(_this, (event_cond_t *)pstCondInfo, pstCondPtr);
	}
}

static int 
time_cond_get_list(
	cond_list_class_t *_this, 
	time_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastCondInfo, piCount);
	
	int iCount = 0;
	int iDevMaxNum = *piCount;
	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		*piCount = 0;
		return NoErr;
	}
	
	
	cond_list_node_t *pstPtr = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	if(NULL == pstPtr)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this)
		);
	if(NULL == pstHead)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		time_cond_node_t *p = pstPtr->stCondSet.pstCond;
		while(p)
		{
			if(iCount < iDevMaxNum)
			{
				memcpy(&pastCondInfo[iCount++], 
					&p->stCond, 
					sizeof(time_cond_t)
				);
			}
			p = p->next;
		}
	
		pstPtr = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	
	*piCount = iCount;

	return NoErr;
}

static int 
event_cond_get_list(
	cond_list_class_t *_this, 
	event_cond_t *pastCondInfo,
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastCondInfo, piCount);
	
	int iCount = 0;
	int iDevMaxNum = *piCount;
	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		*piCount = 0;
		return NoErr;
	}
	
	
	cond_list_node_t *pstPtr = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	if(NULL == pstPtr)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this)
		);
	if(NULL == pstHead)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		event_cond_node_t *p = pstPtr->stCondSet.pstCond;
		while(p)
		{
			if(iCount < iDevMaxNum)
			{
				memcpy(&pastCondInfo[iCount++], 
					&p->stCond, 
					sizeof(event_cond_t)
				);
			}
			p = p->next;
		}
	
		pstPtr = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	
	*piCount = iCount;

	return NoErr;
}

static int 
cond_get_list(
	cond_list_class_t *_this,
	void *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		return time_cond_get_list(_this, (time_cond_t *)pastCondInfo, piCount);
	}
	else
	{
		return event_cond_get_list(_this, (event_cond_t *)pastCondInfo, piCount);
	}
}


static int 
time_cond_true_all(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	int i = 0;
	int j = 0;
	char *pStrtok;
	char *pWeek;
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		if(!strncmp(_this->stCondInfo.acLogic, "Or", LOGIC_MAX_LEN))
		{
			/*如果或条件列表为空，则默认该条件列表为假，
			*因为只有为假才不会对其他的或条件列表产生影响*/
			_this->uiCondSetTrue = 0;
			HY_DEBUG("The Or Time Cond Set is True.\n");
			return _this->uiCondSetTrue;
		}
		else
		{
			/*如果与条件列表为空，则默认该条件列表为真，
			*因为只有为真才不会对其他的与条件列表产生影响*/
			_this->uiCondSetTrue = 1;
			HY_DEBUG("The And Time Cond Set is False.\n");
			return _this->uiCondSetTrue;
		}
	}
	
	/*获取当前时间信息*/
	time_t Now;
	struct tm *pNowTm;
	time(&Now);
	pNowTm = localtime(&Now);

	cond_list_node_t *pstPtr = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);

	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this)
		);

	while(pstPtr && pstHead != pstPtr)
	{
		/*依次判断所有条件集合的真假*/
		time_cond_node_t *p = pstPtr->stCondSet.pstCond;
		while(p)
		{
			HY_DEBUG("pstPtr->iCondSetIndex = %d\n", pstPtr->iCondSetIndex);
			/*依次判断所有条件的真假*/
			if(strncmp(p->stCond.acLose, "1", STATE_MAX_LEN))
			{
				HY_DEBUG("The Time Cond is Effective\n");
				char *pTmp = p->stCond.acWeek;
				/*获取星期信息*/
				while(NULL != (pWeek = strtok_r(pTmp, ",", &pStrtok)))
				{
					
					/*将星期转换为0-6*/
					for(j = 0; j < 7 && 
						strncmp(pWeek, apcWeek[j], WEEK_MAX_LEN); ++j
					);
					
					HY_DEBUG("New Time, Week %d, Time %d:%d\n", 
						pNowTm->tm_wday, pNowTm->tm_hour, pNowTm->tm_min);
					HY_DEBUG("Cond Time, Week %d, Start Time %d:%d, End Time %d:%d\n", 
						j, 
						atoi(p->stCond.acStartHour), atoi(p->stCond.acStartMinu),
						atoi(p->stCond.acEndHour), atoi(p->stCond.acEndMinu));
					HY_DEBUG("Cmp Start Time : %d\n",
						time_cmp(pNowTm->tm_hour, pNowTm->tm_min, atoi(p->stCond.acStartHour), atoi(p->stCond.acStartMinu))
					);
					HY_DEBUG("Cmp End Time : %d\n", 
						time_cmp(pNowTm->tm_hour, pNowTm->tm_min, atoi(p->stCond.acEndHour), atoi(p->stCond.acEndMinu))
					);
					if(pNowTm->tm_wday == j &&
						time_cmp(pNowTm->tm_hour,
							pNowTm->tm_min,
							atoi(p->stCond.acStartHour), 
							atoi(p->stCond.acStartMinu)
						) >= 0 &&
						time_cmp(pNowTm->tm_hour, 
							pNowTm->tm_min, 
							atoi(p->stCond.acEndHour), 
							atoi(p->stCond.acEndMinu)
						) <= 0
					)
					{
						/*当前时间在时间条件范围内，则该时间条件成立，即为真*/
						pstPtr->stCondSet.uiCondTrue |= 
							0x1 << p->iCondIndex;
						HY_DEBUG("The Time Cond is True.\n");
						
						/*或条件，一真为真，该条件集合为真*/
						if(!strncmp(_this->stCondInfo.acLogic, 
								"Or", 
								LOGIC_MAX_LEN)
						)
						{
							_this->uiCondSetTrue |= 
								0x1 << pstPtr->iCondSetIndex;
							HY_DEBUG("The Or Time Cond Set is True.\n");
							goto continue_next;
						}
					}
					else
					{
						/*当前时间在时间条件范围外，则该时间条件不成立，即为假*/
						/*该时间条件为假*/
						pstPtr->stCondSet.uiCondTrue &=
							~(0x1 << p->iCondIndex);
						HY_DEBUG("The Time Cond is False.\n");
						if(0 == atoi(p->stCond.acRepeat) &&
							time_cmp(pNowTm->tm_hour, 
								pNowTm->tm_min, 
								atoi(p->stCond.acEndHour), 
								atoi(p->stCond.acEndMinu)
							) > 0
						)
						{
							/*如果当前时间比结束时间还晚，而且改时间条件不重复，
							*则该条件将永远不可能为真，所以该条件应被置为无效*/
							strncpy(p->stCond.acLose, "1", STATE_MAX_LEN);
						}
						/*与条件集合，一假为假, 该条件集合为假*/
						if(!strncmp(_this->stCondInfo.acLogic,
								"And", 
								LOGIC_MAX_LEN)
						)
						{
							
							_this->uiCondSetTrue &= 
								~(0x1 << pstPtr->iCondSetIndex);
							HY_DEBUG("The And Time Cond Set is False.\n");
							goto continue_next;
						}
					}
					pTmp = NULL;
				}				
			}
			else
			{
				HY_DEBUG("The Time Cond is Lose\n");
				/*时间条件已失效，则该时间条件永远为假*/
				pstPtr->stCondSet.uiCondTrue &= ~(0x1 << i);
				HY_DEBUG("The Time Cond is False.\n");
				/*与条件集合，一假为假， 该条件集合为假*/
				if(!strncmp(_this->stCondInfo.acLogic, "And", LOGIC_MAX_LEN))
				{
					_this->uiCondSetTrue &= ~(0x1 << pstPtr->iCondSetIndex);
					HY_DEBUG("The And Time Cond Set is False.\n");
					goto continue_next;
				}
			}
			p = p->next;
		}
		
		/*程序执行到此处，只有两种情况
		*1 该集合为与集合，且该结点内的所有条件都为真
		*2 该集合为或集合，且该结点内的所有条件都为假
		*/
		if(!strncmp(_this->stCondInfo.acLogic, "And", LOGIC_MAX_LEN))
		{
			_this->uiCondSetTrue |= 0x1 << pstPtr->iCondSetIndex;
			HY_DEBUG("The And Time Cond Set is True.\n");
			goto continue_next;
		}
		else
		{
			_this->uiCondSetTrue &= ~(0x1 << pstPtr->iCondSetIndex);
			HY_DEBUG("The Or Time Cond Set is False.\n");
			goto continue_next;
		}
		
continue_next:
		pstPtr = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	
	return _this->uiCondSetTrue;
}

static int 
cond_print(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, COND_TIME, COND_TYPE_MAX_LEN))
	{
		HY_INFO("Time Cond List:\n");
		return _this->stList.print(
					(link_list_class_t *)_this, 
					print_time_cond_info_handle
				);
	}
	else
	{
		HY_INFO("Event Cond List:\n");
		return _this->stList.print(
				(link_list_class_t *)_this, 
				print_event_cond_info_handle
			);
	}
}

static int 
time_cond_clear(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this
			)
		);
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		time_cond_node_t *p = pstNode->stCondSet.pstCond;
		time_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		pstNode = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	_this->iCount = 0;
	return _this->stList.clear((link_list_class_t *)_this);
}


static int 
event_cond_clear(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(_this->stList.head(
			(link_list_class_t *)_this
			)
		);
	
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(_this->stList.first(
			(link_list_class_t *)_this)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		event_cond_node_t *p = pstNode->stCondSet.pstCond;
		event_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
					
		pstNode = 
			(cond_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	_this->iCount = 0;
	return _this->stList.clear((link_list_class_t *)_this);
}


static int 
cond_clear(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(!strncmp(_this->stCondInfo.acCondType, 
		COND_TIME, 
		COND_TYPE_MAX_LEN)
	)
	{
		return time_cond_clear(_this);
	}
	else
	{
		return event_cond_clear(_this);
	}
}




/*构造函数*/
cond_list_class_t* 
new_cond_list(DB *pDb, cond_info_t *pstCondInfo)
{
	PARAM_CHECK_RETURN_NULL_2(pDb, pstCondInfo);
	int iDataSize = sizeof(cond_list_class_t);

	/*申请空间*/
	cond_list_class_t *pNew = 
		(cond_list_class_t *)calloc(1, sizeof(cond_list_class_t));
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}

	
	/*调用父类的构造函数*/
	link_list_class_t *pstLink = new_link_list(iDataSize);
	if(NULL == pstLink)
	{
		HY_ERROR("New DevList Error.\n");
		free(pNew);
		return NULL;
	}
	/*将父类的数据拷贝到子类中*/
	memcpy(pNew, pstLink, sizeof(link_list_class_t));
	/*pstLink里的某些成员也是动态分配的，而这些内存是要使用的，所以
	*此处指释放pstLink，而不释放成员的分配
	*/
	free(pstLink);

	/*数据库存储路径*/
	pNew->pDb = pDb;
	memcpy(&pNew->stCondInfo, pstCondInfo, sizeof(cond_info_t));
	
	if(!strncmp(pNew->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*如果是或条件列表，条件列表的真假默认为假*/
		pNew->uiCondSetTrue = 0;
	}
	else
	{
		/*如果是与条件列表，条件列表的真假默认为真*/
		pNew->uiCondSetTrue = 1;
	}
	
	cond_list_private_methods_t *pPrivateMethods = 
		(cond_list_private_methods_t *)pNew->acCondlistPrivateMethods;

	pPrivateMethods->cond_add_db = cond_add_db;
	pPrivateMethods->cond_del_db = cond_del_db;
	pPrivateMethods->cond_set_db = cond_set_db;
	pPrivateMethods->cond_get_db = cond_get_db;
	pPrivateMethods->cond_get_list_db = cond_get_list_db;
	pPrivateMethods->cond_list_update_index = 
		cond_list_update_index;
	pPrivateMethods->time_cond_time_lose_set_db = time_cond_time_lose_set_db;
	
	pNew->cond_clear = cond_clear;
	pNew->cond_size = cond_size;
	pNew->cond_add = cond_add;
	pNew->cond_del = cond_del;
	pNew->cond_set = cond_set;
	pNew->cond_get = cond_get;
	pNew->cond_get_list = cond_get_list;
	pNew->cond_true_all = time_cond_true_all;
	pNew->cond_print = cond_print;
	pNew->cond_get_list_db = cond_get_list_db;
	pNew->cond_del_db = cond_del_db;
	pNew->cond_add_db = cond_add_db;
	pNew->cond_set_db = cond_set_db;
	pNew->time_cond_time_lose_set_db = time_cond_time_lose_set_db;
	
	return pNew;

}

/*析构函数*/
int 
destroy_cond_list(cond_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	if(_this->stList.size((link_list_class_t*)_this))
	{
		/*清空链表*/
		_this->cond_clear(_this);
	}
	/*释放头结点*/
	free(_this->stList.pHead);
	_this->stList.pHead = _this->stList.pTail = NULL;
	free(_this);
	_this = NULL;
	return 0;
}
























