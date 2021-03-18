/***********************************************************
*文件名    : action_list.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.10
*说   明   : 动作接口
*修改记录: 
************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "param_check.h"
#include "action_list.h"
#include "cjson.h"
#include "json_key.h"
#include "misc_api.h"

/*数据库条件表名*/
#define DB_TABLE_NAME_ACTION		"action_list"

#pragma pack(1)

/*私有方法定义*/
typedef struct action_list_private_methods_s  
{  
	/*向数据库中添加动作*/
	int (*action_add_db)(struct action_list_class_s *, action_info_t *);
	/*从数据库中删除动作*/
	int (*action_del_db)(struct action_list_class_s *, action_info_t *);
	/*设置指定动作*/
	int (*action_set_db)(struct action_list_class_s *, action_info_t *);
	/*从数据库中获取指定动作*/
	int (*action_get_db)(struct action_list_class_s *, action_info_t *);
	/*获取数据库中的动作列表*/
	int (*action_get_list_db)(struct action_list_class_s *, action_info_t *, int*);


	
	/*设置结点中的动作信息*/
	action_list_node_t* (*action_info_set)(struct action_list_class_s *, action_list_node_t *, action_info_t *);
	/*获取结点中动作信息*/
	int (*action_info_get)(struct action_list_class_s *, action_list_node_t *, action_info_t *);
}action_list_private_methods_t; 
#pragma pack()


action_cb_t g_astActionCBList[ACTION_CALLBACK_LIST_MAX_NUM] = {{0}};


static int (*handleHyCommandByScene)(char *, int);
void hy_scene_command_register(int (*func)(char *, int))
{
	handleHyCommandByScene=func;
}
/*************************************************************
*函数:	action_compare
*返回值:0表示成功，非0表示失败
*描述:	动作比较
*************************************************************/
static int 
action_compare(link_list_piece_t *pData1, link_list_piece_t *pData2)
{
	action_list_node_t *p1 = (action_list_node_t*)pData1;
	action_list_node_t *p2 = (action_list_node_t*)pData2;
	
	return strncmp(p1->stActionInfo.acActionId, p2->stActionInfo.acActionId, ACTION_ID_MAX_LEN) ||
		strcasecmp(p1->stActionInfo.acDevId, p2->stActionInfo.acDevId) ||
		strncmp(p1->stActionInfo.acKey, p2->stActionInfo.acKey, KEY_MAX_LEN);
}

/*************************************************************
*函数:	print_action_info_handle
*参数:	
*返回值:0表示成功，非0表示失败
*描述:	打印信息
*(注:该接口仅用于测试)
*************************************************************/
static int 
print_action_info_handle(link_list_piece_t *pNode)
{
	action_list_node_t *p = (action_list_node_t *)pNode;
	action_info_t *pstInfo = (action_info_t *)&(p->stActionInfo);
	HY_INFO("\t%s\t%s\t%s\t%s\t%s\t%s\n", 
		pstInfo->acActionId,
		pstInfo->acDevId,
		pstInfo->acKey,
		pstInfo->acValueCoding, 
		pstInfo->acValue,
		pstInfo->acResult);
	return NoErr;
}

/*向数据库中添加动作*/
static int 
action_list_add_db(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	/*未查找到条目，使用insert语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		INSERT INTO %s \
			(id, action_id, dev_id, key, value_coding, value) \
		VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s');\
		", 
		DB_TABLE_NAME_ACTION,
		_this->acId,
		pstActionInfo->acActionId,
		pstActionInfo->acDevId,
		pstActionInfo->acKey,
		pstActionInfo->acValueCoding,
		pstActionInfo->acValue
	);
	iRet = db_add_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("INSERT DB Table(%s) failed.\n", 
			DB_TABLE_NAME_ACTION);
		return SceneAddErr;
	}
	return NoErr;
}
/*从数据库中删除动作*/
static int 
action_list_del_db(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE id='%s' and action_id='%s' and dev_id='%s'and key='%s';\
		", 
		DB_TABLE_NAME_ACTION,
		_this->acId,
		pstActionInfo->acActionId,
		pstActionInfo->acDevId,
		pstActionInfo->acKey);
	iRet = db_del_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_ACTION);
		return SceneDelErr;
	}

	return NoErr;
}
/*设置指定动作*/
static int 
action_list_set_db(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*查找到条目，使用update语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET value_coding='%s', value='%s'\
		WHERE id='%s' and action_id='%s' and dev_id='%s' and key='%s';\
		", 
		DB_TABLE_NAME_ACTION,
		pstActionInfo->acValueCoding,
		pstActionInfo->acValue,
		_this->acId,
		pstActionInfo->acActionId,
		pstActionInfo->acDevId,
		pstActionInfo->acKey);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_ACTION);
		return SceneAddErr;
	}
	return NoErr;
}
/*从数据库中获取指定动作*/
static int 
action_list_get_db(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 2;
	db_data_entry_t astData[iRow * iRank];
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT value_coding, value FROM %s\
		WHERE id='%s' and action_id='%s' and dev_id='%s' and key='%s';",
		DB_TABLE_NAME_ACTION,
		_this->acId,
		pstActionInfo->acActionId,
		pstActionInfo->acDevId,
		pstActionInfo->acKey);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_ACTION);
		return SceneGetErr;
	}

	if(NoErr == iRow)
	{
		HY_ERROR("The ActionInfo(%s %s) not found.\n",
			pstActionInfo->acDevId, pstActionInfo->acKey);
		return NotFoundErr;
	}

	strncpy(pstActionInfo->acValueCoding, astData[0].acData, VALUE_CODING_MAX_LEN);
	strncpy(pstActionInfo->acValue, astData[1].acData, VALUE_MAX_LEN);

	return NoErr;
}
/*获取数据库中的动作列表*/
static int 
action_list_get_list_db(action_list_class_t *_this, action_info_t *pastActionInfo, int *piCount)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastActionInfo, piCount);

	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	action_info_t *pstEntry = pastActionInfo;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = ACTION_MAX_NUM;
	iRank = 5;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
	memset(astData, 0x0, iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT action_id, dev_id, key, value_coding, value FROM %s\
		WHERE id='%s';",
		DB_TABLE_NAME_ACTION,
		_this->acId);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_ACTION);
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
			strncpy(pstEntry->acActionId, 
				astData[(iRank * i) + 0].acData, ACTION_ID_MAX_LEN);
			strncpy(pstEntry->acDevId, 
				astData[(iRank * i) + 1].acData, DEV_ID_MAX_LEN);
			strncpy(pstEntry->acKey, 
				astData[(iRank * i) + 2].acData, KEY_MAX_LEN);
			strncpy(pstEntry->acValueCoding, 
				astData[(iRank * i) + 3].acData, VALUE_CODING_MAX_LEN);
			strncpy(pstEntry->acValue, 
				astData[(iRank * i) + 4].acData, VALUE_MAX_LEN);
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


/*设置结点中的动作信息*/
static action_list_node_t* 
action_info_set(action_list_class_t *_this, action_list_node_t *pstNode, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pstNode, pstActionInfo);
	/*加写锁*/
	_this->stList.write_lock((link_list_class_t *)_this);
	memcpy(&(pstNode->stActionInfo), pstActionInfo, sizeof(action_info_t));
	_this->stList.write_unlock((link_list_class_t *)_this);

	return pstNode;
}
/*获取结点中动作信息*/
static int 
action_info_get(action_list_class_t *_this, action_list_node_t *pstNode, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstNode, pstActionInfo);
	/*加读锁*/
	_this->stList.write_lock((link_list_class_t *)_this);
	memcpy(pstActionInfo, &(pstNode->stActionInfo), sizeof(action_info_t));
	_this->stList.write_unlock((link_list_class_t *)_this);

	return NoErr;
}


/*初始化动作链表，即将数据库中的设备信息加载到内存*/
static int action_list_init(action_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	int iRet = 0;
	int iCount = ACTION_MAX_NUM;
	action_info_t *astInfo = (action_info_t *)calloc(ACTION_MAX_NUM, sizeof(action_info_t));
	if(NULL == astInfo)
	{
		HY_ERROR("Calloc error.\n");
		return HeapReqErr;
	}

	/*开始初始化操作*/
	_this->ucInitFlag = 1;
	
	pPrivateMethods->action_get_list_db(_this, astInfo, &iCount);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("ActionId = %s\n", astInfo[i].acActionId);
		HY_DEBUG("DevId = %s\n", astInfo[i].acDevId);
		HY_DEBUG("Key = %s\n", astInfo[i].acKey);
		HY_DEBUG("ValueCoding = %s\n", astInfo[i].acValueCoding);
		HY_DEBUG("Value = %s\n", astInfo[i].acValue);
	}

	iRet = _this->action_sync(_this, astInfo, iCount);
	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}

	/*结束初始化操作*/
	_this->ucInitFlag = 0;
	
	return iRet;
}


/*同步动作信息*/
static int 
action_list_sync(action_list_class_t *_this, action_info_t *pastActionInfo, int iCount)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pastActionInfo);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iCount);

	int i = 0;
	
	if(NoErr == _this->stList.size((link_list_class_t *)_this))
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->action_add(_this, &pastActionInfo[i]);
			HY_INFO("Add Action(%s %s %s)\n", 
				pastActionInfo[i].acActionId, 
				pastActionInfo[i].acDevId, 
				pastActionInfo[i].acKey);
		}
		return NoErr;
	}
	if(NoErr == iCount)
	{
		_this->action_clear(_this);
		HY_INFO("Clear Action\n");
		return NoErr;
	}
	char *pacFoundFlag = (char *)calloc(iCount ,sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}

	action_list_node_t *pstNode = 
		(action_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	action_list_node_t *pstHead = 
		(action_list_node_t *)(_this->stList.head((link_list_class_t *)_this));
	if(NULL == pstHead)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	while(pstNode && pstHead != pstNode)
	{
		int iFlag = 0;
		for(i = 0; i < iCount; ++i)
		{
			if(!strncmp(pstNode->stActionInfo.acActionId, pastActionInfo[i].acActionId, ACTION_ID_MAX_LEN) &&
				!strcasecmp(pstNode->stActionInfo.acDevId, pastActionInfo[i].acDevId) &&
				!strncmp(pstNode->stActionInfo.acKey, pastActionInfo[i].acKey, KEY_MAX_LEN))
			{
				/*找到,判断数据是否发生改变*/
				if(strncmp(pstNode->stActionInfo.acValueCoding, pastActionInfo[i].acValueCoding, VALUE_CODING_MAX_LEN) ||
					strncmp(pstNode->stActionInfo.acValue, pastActionInfo[i].acValue, VALUE_MAX_LEN) )
				{
					/*信息发生改变*/
					_this->action_set(_this, &pastActionInfo[i]);
					HY_INFO("Set Action(%s %s %s)\n", 
						pastActionInfo[i].acActionId,
						pastActionInfo[i].acDevId, 
						pastActionInfo[i].acKey);
				}
				pacFoundFlag[i] = 1;
				iFlag = 1;
				break;
			}
		}
		if(!iFlag)
		{
			/*未找到，删除*/
			_this->action_del(_this, &pastActionInfo[i]);
			HY_INFO("Del Action(%s %s %s)\n", 
				pastActionInfo[i].acActionId,
				pastActionInfo[i].acDevId,
				pastActionInfo[i].acKey);
		}
	
		pstNode = 
			(action_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->action_add(_this, &pastActionInfo[i]);
			HY_INFO("Add Action(%s %s %s)\n", 
				pastActionInfo[i].acActionId,
				pastActionInfo[i].acDevId,
				pastActionInfo[i].acKey);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;
}

/*清空动作*/
static int 
action_list_clear(action_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->stList.clear((link_list_class_t *)_this);
}

/*添加动作*/
static int 
action_list_add(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	//HY_DEBUG("pstActionInfo->acDevId = %s\n", pstActionInfo->acDevId);
	//HY_DEBUG("pstActionInfo->acKey = %s\n", pstActionInfo->acKey);
	//HY_DEBUG("pstActionInfo->acValue = %s\n", pstActionInfo->acValue);
	int iRet = 0;
	
	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	
	/*为了确保设备信息的唯一性*/
	action_info_t stInfo;
	memset(&stInfo, 0x0, sizeof(action_info_t));
	strncpy(stInfo.acActionId, pstActionInfo->acActionId, ACTION_ID_MAX_LEN);
	strncpy(stInfo.acDevId, pstActionInfo->acDevId, DEV_ID_MAX_LEN);
	strncpy(stInfo.acKey, pstActionInfo->acKey, KEY_MAX_LEN);
	strncpy(stInfo.acValueCoding, pstActionInfo->acValueCoding, VALUE_CODING_MAX_LEN);
	iRet = _this->action_get(_this, &stInfo);
	if(NoErr == iRet)
	{
		/*设置*/
		iRet = _this->action_set(_this, pstActionInfo);
		if(NoErr != iRet)
		{
			HY_ERROR("Action Info add failed.\n");
			return SceneAddErr;
		}

		/*数据库更新*/
		
		iRet = 
			pPrivateMethods->action_set_db(_this, pstActionInfo);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Action Info set failed.\n");
			return iRet;
		}
	}
	else
	{
		/*添加*/
		iRet = 
			_this->stList.inst_tail(
				(link_list_class_t *)_this,
				(link_list_piece_t*)(pPrivateMethods->action_info_set(
					_this, 
					(action_list_node_t *)(_this->stList.new_node((link_list_class_t *)_this)),
					pstActionInfo
				))
			);
		if(NoErr != iRet)
		{
			HY_ERROR("Action list inst failed.\n");
			return SceneAddErr;
		}
		/*数据库更新*/
		
		iRet = 
			pPrivateMethods->action_add_db(_this, pstActionInfo);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Action Info set failed.\n");
			return iRet;
		}
	}

	return iRet;
}

/*删除动作*/
static int 
action_list_del(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);

	int iRet = 0;
	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	/*新建结点用于查找*/
	action_list_node_t *pstGetNode = 
		pPrivateMethods->action_info_set(
			_this, 
			(action_list_node_t *)(_this->stList.new_node((link_list_class_t *)_this)),
			pstActionInfo
		);
	/*删除节点*/
	iRet = 
		_this->stList.del(
			(link_list_class_t *)_this, 
			(link_list_piece_t *)(_this->stList.get(
				(link_list_class_t *)_this, 
				action_compare, 
				(link_list_piece_t *)pstGetNode
				)
			)
		);
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	if(NoErr != iRet)
	{
		HY_ERROR("Node deletion failed.\n");
		return SceneDelErr;
	}
	/*删除数据库*/
	
	iRet = 
		pPrivateMethods->action_del_db(_this, pstActionInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB ActionInfo del failed.\n");
		return SceneDelErr;
	}
	
	return NoErr;
}

/*修改动作*/
static int 
action_list_set(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);

	int iRet = 0;
	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	/*新建结点用于查找*/
	action_list_node_t *pstGetNode = 
		pPrivateMethods->action_info_set(
			_this, 
			(action_list_node_t *)(_this->stList.new_node((link_list_class_t *)_this)),
			pstActionInfo
		);
	/*设置结点*/
	action_list_node_t *pNode =
		pPrivateMethods->action_info_set(
			_this,
			(action_list_node_t *)(_this->stList.get(
				(link_list_class_t *)_this,
				action_compare,
				(link_list_piece_t *)pstGetNode
				)
			),
			pstActionInfo
		);
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	if(NULL == pNode)
	{
		HY_ERROR("Not found the Action.\n");
		return NotFoundErr;
	}

	/*数据库更新*/
	iRet = 
		pPrivateMethods->action_set_db(_this, pstActionInfo);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Action Info set failed.\n");
		return SceneSetErr;
	}
	
	return NoErr;
}

/*获取指定动作*/
static int 
action_list_get(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);

	int iRet = 0;
	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	/*新建结点用于查找*/
	action_list_node_t *pstGetNode = 
		pPrivateMethods->action_info_set(
			_this, 
			(action_list_node_t *)(_this->stList.new_node((link_list_class_t *)_this)),
			pstActionInfo
		);
	
	/*获取*/
	iRet = 
		pPrivateMethods->action_info_get(
			_this,
			(action_list_node_t *)(_this->stList.get(
				(link_list_class_t *)_this, 
				action_compare, 
				(link_list_piece_t *)pstGetNode)
			),
			pstActionInfo
		);
	
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	
	return iRet;
}

/*获取所有动作*/
static int 
action_list_get_list(action_list_class_t *_this, action_info_t *pastActionInfo, int *piCount)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastActionInfo, piCount);

	action_list_private_methods_t *pPrivateMethods = 
			(action_list_private_methods_t *)_this->acActionlistPrivateMethods;
	int iCount = 0;
	int iDevMaxNum = *piCount;
	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		*piCount = 0;
		return NoErr;
	}
	
	
	action_list_node_t *pstPtr = 
		(action_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstPtr)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	action_list_node_t *pstHead = 
		(action_list_node_t *)(_this->stList.head((link_list_class_t *)_this));
	if(NULL == pstHead)
	{
		*piCount = 0;
		return SceneGetErr;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		if(iCount < iDevMaxNum)
		{
			pPrivateMethods->action_info_get(
				_this,
				pstPtr,
				&pastActionInfo[iCount++]
			);
		}
	
		pstPtr = 
			(action_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	
	*piCount = iCount;

	return NoErr;
}



/*执行指定动作*/
static int 
action_list_exec(action_list_class_t *_this, action_info_t *pstActionInfo)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstActionInfo);
	int iRet = 0;
	char *pstr = NULL;
	cJSON *pJson = NULL;
	cJSON *pData = NULL;
	cJSON *pDataItem = NULL;
	char acBuff[BUFF_MAX_LEN] = {0};
	if(!strncmp(pstActionInfo->acKey, JSON_VALUE_KEY_SCENE_ID, KEY_MAX_LEN))
	{
		/*执行场景*/
		/*{
		*    "Command": "Dispatch",
		*    "FrameNumber": "00",
		*    "Type": "Scene",
		*    "Data": [
		*        {
		*            "Op":"ExecScene",
		*			"Id": "1"
		*        }
		*    ]
		*}
		*/
		int iValueType = 0;
		/*场景ID*/
		char acSceneId[INDEX_MAX_LEN] = {0};
		/*场景使能*/
		char acSceneEnable[STATE_MAX_LEN] = {0};
		
		pJson = cJSON_Parse(pstActionInfo->acValue);
		if(NULL == pJson)
		{
			HY_ERROR("The json format error.\n");
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_VALUE_KEY_SCENE_ID, 
			acSceneId, 
			INDEX_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		JSON_value_get(
			JSON_KEY_SCENE_ENABLE, 
			acSceneEnable, 
			STATE_MAX_LEN, 
			NULL,
			&iValueType,
			pJson);
		cJSON_Delete(pJson);

		
			
		pJson = cJSON_CreateObject();
		cJSON_AddStringToObject(pJson, JSON_KEY_COMMAND, JSON_VALUE_COMMAND_DISPATCH);
		cJSON_AddStringToObject(pJson, JSON_KEY_FRAMENUMBER, JSON_VALUE_FRAMENUMBER_00);
		cJSON_AddStringToObject(pJson, JSON_KEY_TYPE, JSON_VALUE_TYPE_SCENE);
		pData = cJSON_CreateArray();
		pDataItem = cJSON_CreateObject();
		cJSON_AddStringToObject(pDataItem, JSON_KEY_OPERATION, JSON_VALUE_OPERATION_EXEC_SCENE);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_SCENE_ID, acSceneId);
		if(strcmp(acSceneEnable, ""))
		{
			cJSON_AddStringToObject(pDataItem, JSON_KEY_SCENE_ENABLE, acSceneEnable);
		}
		cJSON_AddItemToArray(pData, pDataItem);
		cJSON_AddItemToObject(pJson, JSON_KEY_DATA, pData);

		pstr = cJSON_Print(pJson);
		strncpy(acBuff, pstr, strlen(pstr));
		free(pstr);
		
		cJSON_Minify(acBuff);
		cJSON_Delete(pJson);

		/*执行动作*/
		//iRet = hyMsgProcess_AsynQuery(acBuff,BUFF_MAX_LEN);
		iRet = handleHyCommandByScene(acBuff, BUFF_MAX_LEN);
		HY_DEBUG("Action exec: %s\n", acBuff);
	}
	else if(!strncmp(pstActionInfo->acKey, JSON_VALUE_KEY_PUSH_ZIGBEE, KEY_MAX_LEN))
	{
		unsigned char aucCustomData[ZIGBEE_CUSTOM_DATA_MAX_LEN] = {0};
		int iCustomDataLen = 0;
		/*执行zigbee透传协议*/
		if(!strncmp(pstActionInfo->acValueCoding, JSON_VALUE_VALUECOGING_ORIGINAL, VALUE_CODING_MAX_LEN))
		{
			iCustomDataLen = strlen(pstActionInfo->acValue);
			memcpy(aucCustomData, pstActionInfo->acValue, iCustomDataLen);
		}
		else if(!strncmp(pstActionInfo->acValueCoding, JSON_VALUE_VALUECOGING_BASE64, VALUE_CODING_MAX_LEN))
		{
			/*base64解码*/
			iCustomDataLen = base64_decode(pstActionInfo->acValue, aucCustomData);
			
		}
		
		/*发送透传数据*/
		//hySendCustomData(pstActionInfo->acDevId, (char *)aucCustomData, iCustomDataLen);
		iRet = handleHyCommandByScene((char *)aucCustomData, iCustomDataLen);
		HY_DEBUG("Action exec: Push data to zigbee.\n");
		HY_DEBUG("iDataLen = %d\n", iCustomDataLen);
		LOG_HEX(LOG_DEBUG, aucCustomData, 0, iCustomDataLen);
	}
	else if(!strncmp(pstActionInfo->acKey, JSON_VALUE_KEY_SLEEP, KEY_MAX_LEN))
	{
		/*执行睡眠*/
		long lSleepTime = atol(pstActionInfo->acValue);
		usleep(lSleepTime * 1000);
		HY_DEBUG("Action exec: sleep %s ms\n", pstActionInfo->acValue);
	}
	else if(!strncmp(pstActionInfo->acKey, JSON_VALUE_KEY_GROUP, KEY_MAX_LEN))
	{
		/*执行组播指令*/
		/*
		{
		"Command":"Dispatch",
			"FrameNumber":"00",
			"Type":"Multicast",
			"Data":[
				{
					"GroupId":"123456787654310",
					"Type":"A",
					"Key":"Switch_2",
					"Value":"1"
				}
			]
		}
		*/
		int iValueType = 0;
		/*组播ID*/
		char acGroupId[INDEX_MAX_LEN] = {0};

		char acType[TYPE_MAX_LEN] = {0};
		char acKey[KEY_MAX_LEN] = {0};
		char acValue[VALUE_MAX_LEN] = {0};
		
		pJson = cJSON_Parse(pstActionInfo->acValue);
		if(NULL == pJson)
		{
			HY_ERROR("The json format error.\n");
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_KEY_GROUP_ID, 
			acGroupId, 
			INDEX_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_KEY_DATA_TYPE, 
			acType, 
			TYPE_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_KEY_DATA_KEY, 
			acKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_KEY_DATA_VALUE, 
			acValue, 
			VALUE_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		
		cJSON_Delete(pJson);

		
			
		pJson = cJSON_CreateObject();
		cJSON_AddStringToObject(pJson, JSON_KEY_COMMAND, JSON_VALUE_COMMAND_DISPATCH);
		cJSON_AddStringToObject(pJson, JSON_KEY_FRAMENUMBER, JSON_VALUE_FRAMENUMBER_00);
		cJSON_AddStringToObject(pJson, JSON_KEY_TYPE, JSON_VALUE_TYPE_MULTICAST);
		
		pData = cJSON_CreateArray();
		pDataItem = cJSON_CreateObject();
		cJSON_AddStringToObject(pDataItem, JSON_KEY_GROUP_ID, acGroupId);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_KEY, acKey);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_TYPE, acType);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_VALUE, acValue);
		
		cJSON_AddItemToArray(pData, pDataItem);
		cJSON_AddItemToObject(pJson, JSON_KEY_DATA, pData);

		pstr = cJSON_Print(pJson);
		strncpy(acBuff, pstr, strlen(pstr));
		free(pstr);
		
		cJSON_Minify(acBuff);
		cJSON_Delete(pJson);
		
		//iRet = hyMsgProcess_AsynQuery(acBuff,BUFF_MAX_LEN);
		iRet = handleHyCommandByScene(acBuff, BUFF_MAX_LEN);
		HY_DEBUG("Action exec: Grout %s\n", acBuff);
	}
	else if(!strncmp(pstActionInfo->acKey, JSON_VALUE_KEY_CALL_BACK, KEY_MAX_LEN))
	{
		/*执行回调*/
		int iValueType = 0;
		/*回调ID*/
		char acCbId[INDEX_MAX_LEN] = {0};
		char acParam[KEY_MAX_LEN] = {0};
		
		pJson = cJSON_Parse(pstActionInfo->acValue);
		if(NULL == pJson)
		{
			HY_ERROR("The json format error.\n");
			return JsonFormatErr;
		}
		if(NoErr != JSON_value_get(
			JSON_KEY_CALL_BACK_ID, 
			acCbId, 
			INDEX_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		
		if(NoErr != JSON_value_get(
			JSON_KEY_PARAMETER, 
			acParam, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pJson)
		)
		{
			HY_ERROR("The json format error.\n");
			cJSON_Delete(pJson);
			return JsonFormatErr;
		}
		cJSON_Delete(pJson);

		/*查找回调信息*/
		action_cb_t* pCbInfo = action_callback_get(atoi(acCbId));
		if(NULL == pCbInfo)
		{
			HY_ERROR("The callback(%s) was not found.\n", acCbId);
			return NotFoundErr;
		}

		/*调用回调*/
		((ActionCbFun)(pCbInfo->pFun))(acParam, pCbInfo->pUserData);
		HY_DEBUG("Action exec: CallBack %s\n", acParam);
	}
	else
	{
		/*
		*普通动作协议
		*{
			"Command":"Dispatch",
			"FrameNumber":"00",
			"Type":"Ctrl",
			"Data":[
				{
					"DeviceId":"123456787654310",
					"Key":"Switch",
					"Value":"1"
				}
			]
		}
		*/
		pJson = cJSON_CreateObject();
		cJSON_AddStringToObject(pJson, JSON_KEY_COMMAND, JSON_VALUE_COMMAND_DISPATCH);
		cJSON_AddStringToObject(pJson, JSON_KEY_FRAMENUMBER, JSON_VALUE_FRAMENUMBER_00);
		cJSON_AddStringToObject(pJson, JSON_KEY_TYPE, JSON_VALUE_TYPE_CTRL);
		pData = cJSON_CreateArray();
		pDataItem = cJSON_CreateObject();
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_DAVICE_ID, pstActionInfo->acDevId);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_KEY, pstActionInfo->acKey);
		cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_VALUE, pstActionInfo->acValue);
		cJSON_AddItemToArray(pData, pDataItem);
		cJSON_AddItemToObject(pJson, JSON_KEY_DATA, pData);

		pstr = cJSON_Print(pJson);
		strncpy(acBuff, pstr, strlen(pstr));
		free(pstr);
		
		cJSON_Minify(acBuff);
		cJSON_Delete(pJson);
		HY_DEBUG("Action exec: %s\n", acBuff);
		//iRet = hyMsgProcess_AsynQuery(acBuff,BUFF_MAX_LEN);
		iRet = handleHyCommandByScene(acBuff, BUFF_MAX_LEN);
		snprintf(pstActionInfo->acResult, STATE_MAX_LEN, "%d", iRet);
	}

	return iRet;
}

/*执行所有动作*/
static int 
action_list_exec_all(action_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		return NoErr;
	}
	
	
	action_list_node_t *pstPtr = 
		(action_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstPtr)
	{
		return SceneGetErr;
	}
	action_list_node_t *pstHead = 
		(action_list_node_t *)(_this->stList.head((link_list_class_t *)_this));
	if(NULL == pstHead)
	{
		return SceneGetErr;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		_this->action_exec(_this, &pstPtr->stActionInfo);
		pstPtr = 
			(action_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}

	return NoErr;
}



/*打印所有动作，仅用于调试打印*/
static int 
action_list_print(action_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	HY_INFO("Action List:\n");
	return _this->stList.print((link_list_class_t *)_this, print_action_info_handle);
}










/*构造函数*/
action_list_class_t* new_action_list(DB *pDb, char *pcId)
{
	PARAM_CHECK_RETURN_NULL_2(pDb, pcId);
	int iDataSize = sizeof(action_list_node_t);

	/*申请空间*/
	action_list_class_t *pNew = 
		(action_list_class_t *)calloc(1, sizeof(action_list_class_t));
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
	
	strncpy(pNew->acId, pcId, INDEX_MAX_LEN);
	
	action_list_private_methods_t *pPrivateMethods = 
		(action_list_private_methods_t *)pNew->acActionlistPrivateMethods;

	pPrivateMethods->action_add_db = action_list_add_db;
	pPrivateMethods->action_del_db = action_list_del_db;
	pPrivateMethods->action_set_db = action_list_set_db;
	pPrivateMethods->action_get_db = action_list_get_db;
	pPrivateMethods->action_get_list_db = action_list_get_list_db;
	pPrivateMethods->action_info_set = action_info_set;
	pPrivateMethods->action_info_get = action_info_get;

	pNew->action_init = action_list_init;
	pNew->action_sync = action_list_sync;
	pNew->action_clear = action_list_clear;
	pNew->action_add = action_list_add;
	pNew->action_del = action_list_del;
	pNew->action_set = action_list_set;
	pNew->action_get = action_list_get;
	pNew->action_get_list = action_list_get_list;
	pNew->action_exec = action_list_exec;
	pNew->action_exec_all = action_list_exec_all;
	pNew->action_print = action_list_print;
	pNew->action_del_db = action_list_del_db;
	return pNew;
}

/*析构函数*/
int destroy_action_list(action_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	return destroy_link_list((link_list_class_t*)_this);
}

/*动作回调函数注册*/
int action_callback_reg(int iCbid, void *pFun, void *pUserData)
{
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iCbid);
	PARAM_CHECK_RETURN_ERRORNO_1(pFun);
	int i = 0;
	for(i = 0; i < ACTION_CALLBACK_LIST_MAX_NUM; ++i)
	{
		if(g_astActionCBList[i].iCbId <= 0)
		{
			break;
		}
	}
	if(i >= ACTION_CALLBACK_LIST_MAX_NUM)
	{
		HY_ERROR("The callback function exceeded the maximum number(%d) of supported.\n", 
			ACTION_CALLBACK_LIST_MAX_NUM);
		return HeapReqErr;
	}
	g_astActionCBList[i].iCbId = iCbid;
	g_astActionCBList[i].pFun = pFun;
	g_astActionCBList[i].pUserData = pUserData;
	
	return NoErr;
}

/*动作回调函数注册*/
action_cb_t* action_callback_get(int iCbid)
{
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_NULL_1(iCbid);
	
	int i = 0;
	for(i = 0; i < ACTION_CALLBACK_LIST_MAX_NUM; ++i)
	{
		if(g_astActionCBList[i].iCbId == iCbid)
		{
			break;
		}
	}
	if(i >= ACTION_CALLBACK_LIST_MAX_NUM)
	{
		HY_ERROR("Not found.\n");
		return NULL;
	}
	return &g_astActionCBList[i];
}


