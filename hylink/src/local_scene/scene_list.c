/***********************************************************
*文件名    : scene_list.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.04
*说   明   : 本地化场景接口
*修改记录: 
************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>


#include "param_check.h"
#include "scene_list.h"
#include "circle_queue.h"
#include "ptr_hash.h"
#include "tool.h" 
#include "base_api.h"
#include "cjson.h" 
#include "json_key.h" 
#include "scene_timer.h" 

/*数据库条件表名*/
#define DB_TABLE_NAME_SCENE				"scene_list"
#define DB_TABLE_NAME_BIND_SCENE		"scene_bind_list"
#define DB_TABLE_NAME_ACTION			"action_list"
#define DB_TABLE_NAME_COND				"cond_list"

/*指针hash表，指针类型*/
#define PRT_HASH_TYPE_COND				"cond"
#define PRT_HASH_TYPE_SCENE_BIND		"scene_bind"


static char* apcWeek[] = {WEEK_SUN, WEEK_MON, WEEK_TUE, WEEK_WED, WEEK_THU, WEEK_FIR, WEEK_SAT};

#pragma pack(1)

typedef struct link_queue_node_s
{
	link_list_piece_t stLinkPiece;
	int iExecDelayed;
	int iTime;
	scene_info_t stSceneInfo;
}link_queue_node_t;

/*定时器参数定义*/
typedef struct scene_timer_param_s  
{  
	scene_list_class_t *pstSceneClass;
	ptr_node_t *pstCondPtr;
}scene_timer_param_t; 


/*私有参数定义*/
typedef struct scene_list_private_param_s  
{  
	/*条件哈希表*/
	ptr_hash_table_class_t *pHashTable;
	/*场景动作执行线程*/
	pthread_t pid_exec;
	/*场景条件判断执行线程*/
	pthread_t pid_cond;
	
	/*场景执行队列*/
	link_list_class_t *pstExecQueue;
	
	/*互斥锁*/
	void *mutex;
}scene_list_private_param_t;  

/*私有方法定义*/
typedef struct scene_list_private_methods_s  
{  
	int (*scene_table_db)(struct scene_list_class_s *);
	int (*scene_add_db)(struct scene_list_class_s *, scene_info_t *);
	int (*scene_del_db)(struct scene_list_class_s *, scene_info_t *);
	int (*scene_clear_db)(struct scene_list_class_s *);
	int (*scene_set_db)(struct scene_list_class_s *, scene_info_t *);
	int (*scene_get_db)(struct scene_list_class_s *, scene_info_t *);
	int (*scene_get_list_db)(
		struct scene_list_class_s *, 
		scene_info_t *, 
		int*
	);

	scene_list_node_t* (*scene_info_set)(
		struct scene_list_class_s *, 
		scene_list_node_t *,
		scene_info_t *
	);
	
	int (*scene_info_get)(
		struct scene_list_class_s *, 
		scene_list_node_t *, 
		scene_info_t *
	);
	
	int (*scene_bind_db)(
		struct scene_list_class_s *, 
		scene_info_t *,
		scene_panel_t *
	);

	int (*scene_bind_set_db)(
		struct scene_list_class_s *, 
		scene_info_t *,
		scene_panel_t *
	);
	
	int (*scene_unbind_db)(
		struct scene_list_class_s *, 
		scene_info_t *, 
		scene_panel_t *
	);
	
	int (*scene_bind_get_db)(
		struct scene_list_class_s *, 
		scene_info_t *, 
		scene_panel_t *
	);
	
	int (*scene_bind_get_list_db)(
		struct scene_list_class_s *, 
		scene_info_t *,
		scene_panel_t *, 
		int*
	);
}scene_list_private_methods_t; 
#pragma pack()
/*************************************************************
*函数:	_scene_time_cond_is_true
*返回值:0表示条件为假，非0表示条件为真
*描述:	事件状态持续触发，定时器任务函数
*************************************************************/
int _scene_time_cond_is_true(time_cond_t *pstCond)
{
	int iNewTrue = 0;
	int i = 0;
	
	char *pStrtok;
	char *pWeek;
	/*获取当前时间信息*/
	time_t Now;
	struct tm *pNowTm;
	time(&Now);
	pNowTm = localtime(&Now);
	
	if(!strncmp(pstCond->acLose, "1", STATE_MAX_LEN))
	{
		HY_DEBUG("New Time, Week %d, Time %d:%d\n", 
			pNowTm->tm_wday, pNowTm->tm_hour, pNowTm->tm_min);
		HY_DEBUG("Cond Time, Week %d, Start Time %d:%d, End Time %d:%d\n", 
			i, 
			atoi(pstCond->acStartHour), atoi(pstCond->acStartMinu),
			atoi(pstCond->acEndHour), atoi(pstCond->acEndMinu));
		
		/*该时间条件已经失效*/
		HY_DEBUG("The Time Cond is Lose\n");
		/*时间条件已失效，则该时间条件永远为假*/
		iNewTrue = 0;
		HY_DEBUG("The Time Cond is False.\n");
	}
	else
	{
		HY_DEBUG("All Week : %s\n",pstCond->acWeek);
		/*依次判断所有条件的真假*/
		char acWeek[WEEK_MAX_LEN] = {0};
		strncpy(acWeek, pstCond->acWeek, WEEK_MAX_LEN);
		char *pTmp = acWeek;
		
		/*获取星期信息*/
		while(NULL != (pWeek = strtok_r(pTmp, ",", &pStrtok)))
		{
			/*将星期转换为0-6*/
			for(i = 0; i < 7 && 
				strncmp(pWeek, apcWeek[i], WEEK_MAX_LEN); ++i
			);
			
			HY_DEBUG("New Time, Week %d, Time %d:%d\n", 
				pNowTm->tm_wday, pNowTm->tm_hour, pNowTm->tm_min);
			HY_DEBUG("Cond Time, Week %d, Start Time %d:%d, End Time %d:%d\n", 
				i, 
				atoi(pstCond->acStartHour), atoi(pstCond->acStartMinu),
				atoi(pstCond->acEndHour), atoi(pstCond->acEndMinu));
			HY_DEBUG("Cmp Start Time : %d\n",
				time_cmp(pNowTm->tm_hour, pNowTm->tm_min, atoi(pstCond->acStartHour), atoi(pstCond->acStartMinu))
			);
			HY_DEBUG("Cmp End Time : %d\n", 
				time_cmp(pNowTm->tm_hour, pNowTm->tm_min, atoi(pstCond->acEndHour), atoi(pstCond->acEndMinu))
			);
			if(time_cmp(pNowTm->tm_hour,
					pNowTm->tm_min,
					atoi(pstCond->acStartHour), 
					atoi(pstCond->acStartMinu)
				) < 0
			)
			{
				/*时间未到*/
				iNewTrue += 0;
				HY_DEBUG("The Time Cond is False.\n");
			}
			else if(pNowTm->tm_wday == i &&
				time_cmp(pNowTm->tm_hour,
					pNowTm->tm_min,
					atoi(pstCond->acStartHour), 
					atoi(pstCond->acStartMinu)
				) >= 0 &&
				time_cmp(pNowTm->tm_hour, 
					pNowTm->tm_min, 
					atoi(pstCond->acEndHour), 
					atoi(pstCond->acEndMinu)
				) <= 0
			)
			{
				/*当前时间在时间条件范围内，则该时间条件成立，即为真*/
				iNewTrue += 1;
				HY_DEBUG("The Time Cond is True.\n");
			}
			else
			{
				/*时间已过*/
				iNewTrue += 0;
				HY_DEBUG("The Time Cond is False.\n");

				if(0 == atoi(pstCond->acRepeat))
				{
					
					/*如果当前时间比结束时间还晚，而且改时间条件不重复，
					*则该条件将永远不可能为真，所以该条件应被置为无效*/
					strncpy(pstCond->acLose, "1", STATE_MAX_LEN);
					/*将时间条件置为失效*/
					HY_DEBUG("Set the time condition to lose.\n");
				}
			}
			pTmp = NULL;
		}			
	}

	return iNewTrue == 0 ? 0 : 1;
}

/*************************************************************
*函数:	_scene_event_cond_is_true
*返回值:0表示条件为假，非0表示条件为真
*描述:	事件状态持续触发，定时器任务函数
*************************************************************/
int _scene_event_cond_is_true(event_cond_t *pstCond, event_t *pstEventInfo)
{
	int iRet = 0;
	/*判断事件条件的真假*/
	int iTrue = 0;
	
	if(is_int(pstCond->acValue))
	{
		HY_DEBUG("The Value is Int\n");
		if(!strncmp(
				pstCond->acActive,
				ACTIVE_EQU, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//等于
			iTrue = 
				atoi(pstEventInfo->acValue) == atoi(pstCond->acValue) ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_GTR, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//大于
			iTrue = 
				atoi(pstEventInfo->acValue) > atoi(pstCond->acValue) ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_LSS, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//小于
			iTrue = 
				atoi(pstEventInfo->acValue) < atoi(pstCond->acValue) ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_NOT, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//不等于
			iTrue = 
				atoi(pstEventInfo->acValue) != atoi(pstCond->acValue) ? 1 : 0;
		}
	}
	else if(is_float(pstCond->acValue))
	{
		HY_DEBUG("The Value is Float\n");
		if(!strncmp(
				pstCond->acActive, 
				ACTIVE_EQU, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//等于
			iTrue = 
				fabs(atof(pstEventInfo->acValue) - atof(pstCond->acValue)) < 0.0001 ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_GTR, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//大于
			iTrue = 
				atof(pstEventInfo->acValue) > atof(pstCond->acValue) ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_LSS, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//小于
			iTrue = 
				atof(pstEventInfo->acValue) > atof(pstCond->acValue) ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_NOT, 
				ACTIVE_MAX_LEN
			)
		)
		{	
			//不等于
			iTrue = 
				fabs(atof(pstEventInfo->acValue) - atof(pstCond->acValue)) > 0.0001 ? 1 : 0;
		}
	}
	else
	{
		/*字符串类型*/
		HY_DEBUG("The Value is String\n");
		iRet = strcmp(pstEventInfo->acValue, pstCond->acValue);
		if(!strncmp(
				pstCond->acActive, 
				ACTIVE_EQU, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//等于
			iTrue = iRet == 0 ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_GTR, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//大于
			iTrue = iRet > 0 ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_LSS, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//小于
			iTrue = iRet < 0 ? 1 : 0;
		}
		else if(!strncmp(
				pstCond->acActive,
				ACTIVE_NOT, 
				ACTIVE_MAX_LEN
			)
		)
		{
			//等于
			iTrue = iRet != 0 ? 1 : 0;
		}
	}

	return iTrue;
}

/*************************************************************
*函数:	_scene_continue_trigger_timer_fun
*返回值:0表示成功，非0表示失败
*描述:	事件状态持续触发，定时器任务函数
*************************************************************/
void *_scene_continue_trigger_timer_fun(void *arg)
{
	HY_DEBUG("_scene_continue_trigger_timer_fun\n");
	scene_timer_param_t *pstSceneTimerPara = 
		(scene_timer_param_t *)arg;
	ptr_node_t *pstCondPtr = pstSceneTimerPara->pstCondPtr;
	scene_list_class_t *_this = 
		pstSceneTimerPara->pstSceneClass;
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	if(pstScene->ucSceneCondTrue)
	{
		/*立即触发*/
		_this->scene_exec(_this, pstScene);
	}
	
	return NULL;
}

/*************************************************************
*函数:	scene_compare
*返回值:0表示成功，非0表示失败
*描述:	场景比较
*************************************************************/
static int 
scene_report_scene_exec(
	scene_list_class_t *_this, 
	char *pcSeneId
)
{
	/*上报协议格式*/
	/*
	{
	    "Command": "Report",
	    "FrameNumber": "00",
	    "Type": "Scene",
	    "Data": [
	        {
	            "Op":"ExecScene",
				"Id": "1"
	        }
	    ]
	}
	*/
	char *pstr = NULL;
	char acOutBuff[JSON_BUFF_MAX_LEN] = {0};
	//char acCooMac[IEEE_MAX_LEN] = {0};
	cJSON *pstJson = NULL;
	cJSON *pstDataArry = NULL;
	
	pstJson = cJSON_CreateObject();
	if(NULL == pstJson)
	{
		return HeapReqErr;
	}
	cJSON_AddStringToObject(pstJson, 
		JSON_KEY_COMMAND, 
		JSON_VALUE_COMMAND_REPORT
	);
	cJSON_AddStringToObject(pstJson, 
		JSON_KEY_FRAMENUMBER, 
		JSON_VALUE_FRAMENUMBER_00
	);
	//getCooMac(acCooMac);
	//cJSON_AddStringToObject(pstJson, 
	//	JSON_KEY_GATEWAY_ID, 
	//	acCooMac
	//);
	cJSON_AddStringToObject(pstJson, 
		JSON_KEY_TYPE, 
		JSON_VALUE_TYPE_SCENE
	);
	
	pstDataArry = cJSON_CreateArray();
	if(NULL != pstDataArry)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL != pstData)
		{
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_OPERATION, 
				JSON_VALUE_OPERATION_EXEC_SCENE
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_SCENE_ID, 
				pcSeneId
			);
			cJSON_AddItemToArray(pstDataArry, pstData);
		}
		
		cJSON_AddItemToObject(pstJson,
			JSON_KEY_DATA,
			pstDataArry
		);
	}
	
	pstr = cJSON_Print(pstJson);
	strncpy(acOutBuff, pstr, JSON_BUFF_MAX_LEN);
	cJSON_Minify(acOutBuff);
	free(pstr);
	cJSON_Delete(pstJson);
	
	/*上报*/
	if(NULL != _this->pSceneReportHandler)
	{
		_this->pSceneReportHandler(acOutBuff);
	}
	
	
	return NoErr;
}



/*************************************************************
*函数:	scene_compare
*返回值:0表示成功，非0表示失败
*描述:	场景比较
*************************************************************/
static int 
scene_compare(
	link_list_piece_t *pData1, 
	link_list_piece_t *pData2
)
{
	scene_list_node_t *p1 = (scene_list_node_t*)pData1;
	scene_list_node_t *p2 = (scene_list_node_t*)pData2;
	
	return strncmp(p1->stSceneInfo.acSceneId, 
		p2->stSceneInfo.acSceneId,
		INDEX_MAX_LEN
	);
}

int scene_queue_compare(void *pData1, void *pData2)
{
	scene_info_t *p1 = (scene_info_t *)pData1;
	scene_info_t *p2 = (scene_info_t *)pData2;
	return strncmp(p1->acSceneId, p2->acSceneId, INDEX_MAX_LEN);
}


/*************************************************************
*函数:	print_scene_info_handle
*参数:	
*返回值:0表示成功，非0表示失败
*描述:	打印信息
*(注:该接口仅用于测试)
*************************************************************/
static int 
print_scene_info_handle(link_list_piece_t *pNode)
{
	scene_list_node_t *p = (scene_list_node_t *)pNode;
	scene_info_t *pstInfo = (scene_info_t *)&(p->stSceneInfo);
	HY_INFO("\t%s\t%s\n", pstInfo->acSceneId,
		pstInfo->acSceneName);
	/*打印时间或条件集合*/
	pstInfo->stOrCond.pstTimeCondList->cond_print(
		pstInfo->stOrCond.pstTimeCondList
	);
	
	/*打印事件或条件集合*/
	pstInfo->stOrCond.pstEventCondList->cond_print(
		pstInfo->stOrCond.pstEventCondList
	);
	
	/*打印时间与条件集合*/
	pstInfo->stAndCond.pstTimeCondList->cond_print(
		pstInfo->stAndCond.pstTimeCondList
	);
	
	/*打印事件与条件集合*/
	pstInfo->stAndCond.pstEventCondList->cond_print(
		pstInfo->stAndCond.pstEventCondList
	);
	
	/*打印动作集合*/
	pstInfo->pstAction->action_print(pstInfo->pstAction);
	
	return NoErr;
}

/*************************************************************
*函数:	scene_event_cond_true_init
*参数:	iCondTrue:单一条件的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	初始化条件的相关真假状态
*************************************************************/
static int
scene_event_cond_true_init(
	scene_list_class_t *_this, 
	int iCondTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_event_cond_true_init\n");

	int iNewCondTrue = 0;
	int iOldCondTrue = 0;
	int iNewCondSetTrue = 0;
	int iOldCondSetTrue = 0;
	int iNewCondListTrue = 0;
	int iOldCondListTrue = 0;
	int iNewSceneTrue = 0;
	int iOldSceneTrue = 0;
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);
	event_cond_node_t *pstCondNode = 
		(event_cond_node_t*)(pstCondPtr->stCondPtr.pCondNode);

	cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
	/*条件当前的真假*/
	iNewCondTrue = iCondTrue;
	/*条件原始的真假*/
	iOldCondTrue = 
		pCondSet->uiCondTrue & (0x1 << pstCondNode->iCondIndex) ? 1 : 0;
	HY_DEBUG("CurrNewCondTrue = %d, CurrOldCondTrue = %d\n", iNewCondTrue, iOldCondTrue);

	/*
	*首先更新条件的真假
	*其次由于条件的真假发生改变，所以必须重新判断该条件所在的条件集合的真假
	*/
	/*更新条件的真假*/
	if(iNewCondTrue)
	{
		pCondSet->uiCondTrue |= 
			0x1 << pstCondNode->iCondIndex;
	}
	else
	{
		pCondSet->uiCondTrue &=  
			~(0x1 << pstCondNode->iCondIndex);
	}

	/*重新判断该条件所在的条件集合的真假*/
	/*条件集合当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件真，则该条件集合真*/
		iNewCondSetTrue = 
			pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
	}
	else
	{
		/*与，所有条件为真，则该条件集合真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
		iNewCondSetTrue = 
			(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件集合原始的真假*/
	iOldCondSetTrue = 
		pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;
	HY_DEBUG("CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewCondSetTrue, iOldCondSetTrue);

	/*
	*首先更新条件集合的真假
	*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
	*/
	/*更新条件集合的真假*/
	if(iNewCondSetTrue)
	{
		pstCondClass->uiCondSetTrue |= 
			0x1 << pstCondSetNode->iCondSetIndex;
	}
	else
	{
		pstCondClass->uiCondSetTrue &= 
			~(0x1 << pstCondSetNode->iCondSetIndex);
	}


	/*重新判断该条件集合所在的条件列表的真假*/
	/*条件列表当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件集合真，则该条件列表真*/
		iNewCondListTrue =
			pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
	}
	else
	{
		/*与，所有条件集合为真，则该条件列表真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
		iNewCondListTrue =
			(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件列表原始的真假*/
	cond_logic_t *pCondLogic = NULL;
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
	{
		pCondLogic = &pstScene->stAndCond;
		iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	else
	{
		pCondLogic = &pstScene->stOrCond;
		iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	
	HY_DEBUG("CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewCondListTrue, iOldCondListTrue);
	/*
	*首先更新条件列表的真假
	*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
	*/
	/*更新条件列表的真假*/
	if(iNewCondListTrue)
	{
		pCondLogic->ucCondListTrue |= 0x1 << 1;
	}
	else
	{
		pCondLogic->ucCondListTrue &=  ~(0x1 << 1);
	}

	/*重新判断该条件列表所在的场景条件的真假*/
	cond_logic_t *pCondOr = &pstScene->stOrCond;
	cond_logic_t *pCondAnd = &pstScene->stAndCond;
	/**
	*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件为假
	*/
	if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewSceneTrue = 0;
	}
	/**
	*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为或条件列表的真假
	*/
	else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewSceneTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
	}
	/**
	*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为与条件列表的真假
	*/
	else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewSceneTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	/**
	*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
	*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
	*/
	else
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewSceneTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
	}
	
	iOldSceneTrue = pstScene->ucSceneCondTrue ? 1 : 0;
	
	HY_DEBUG("CurrNewSceneTrue = %d, CurrOldSceneTrue = %d\n", iNewSceneTrue, iOldSceneTrue);
	/*
	*首先更新条件列表的真假
	*/
	pstScene->ucSceneCondTrue = iNewSceneTrue;
	
	return NoErr;
}

/*************************************************************
*函数:	scene_event_cond_true_update
*参数:	iCondTrue:单一条件的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件的相关真假状态
*************************************************************/
static int
scene_event_cond_true_update(
	scene_list_class_t *_this, 
	int iCondTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_cond_true_update\n");

	int iNewCondTrue = 0;
	int iOldCondTrue = 0;
	int iNewCondSetTrue = 0;
	int iOldCondSetTrue = 0;
	int iNewCondListTrue = 0;
	int iOldCondListTrue = 0;
	int iNewSceneTrue = 0;
	int iOldSceneTrue = 0;
	int iEveryTrigger = 0;
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);
	event_cond_node_t *pstCondNode = 
		(event_cond_node_t*)(pstCondPtr->stCondPtr.pCondNode);

	cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
	/*条件当前的真假*/
	iNewCondTrue = iCondTrue;
	/*条件原始的真假*/
	iOldCondTrue = 
		pCondSet->uiCondTrue & (0x1 << pstCondNode->iCondIndex) ? 1 : 0;
	
	if(strstr(pstCondNode->stCond.acTriggerType, "Every"))
	{
		
		/*每次收到事件上报都触发*/
		iEveryTrigger = 1;
	}

	if(iNewCondTrue != iOldCondTrue)
	{
		HY_DEBUG("Cond True is Changed, CurrNewCondTrue = %d, CurrOldCondTrue = %d\n", iNewCondTrue, iOldCondTrue);
		/*如果条件的真假发生改变
		*首先更新条件的真假
		*其次由于条件的真假发生改变，所以必须重新判断该条件所在的条件集合的真假
		*/

		/*更新条件的真假*/
		if(iNewCondTrue)
		{
			
			pCondSet->uiCondTrue |= 
				0x1 << pstCondNode->iCondIndex;
		}
		else
		{
			pCondSet->uiCondTrue &=  
				~(0x1 << pstCondNode->iCondIndex);
		}

		/*重新判断该条件所在的条件集合的真假*/
		/*条件集合当前的真假*/
		if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
		{
			/*或，任意一个条件真，则该条件集合真*/
			iNewCondSetTrue = 
				pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
		}
		else
		{
			/*与，所有条件为真，则该条件集合真*/
			unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
			iNewCondSetTrue = 
				(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
		}
		
		/*条件集合原始的真假*/
		iOldCondSetTrue = 
			pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;
		if(iNewCondSetTrue != iOldCondSetTrue)
		{
			HY_DEBUG("Cond Set True is Changed, CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewCondSetTrue, iOldCondSetTrue);
			/*如果条件集合的真假发生改变
			*首先更新条件集合的真假
			*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
			*/
			/*更新条件集合的真假*/
			if(iNewCondSetTrue)
			{
				pstCondClass->uiCondSetTrue |= 
					0x1 << pstCondSetNode->iCondSetIndex;
			}
			else
			{
				pstCondClass->uiCondSetTrue &= 
					~(0x1 << pstCondSetNode->iCondSetIndex);
			}


			/*重新判断该条件集合所在的条件列表的真假*/
			/*条件列表当前的真假*/
			if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
			{
				/*或，任意一个条件集合真，则该条件列表真*/
				iNewCondListTrue =
					pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
			}
			else
			{
				/*与，所有条件集合为真，则该条件列表真*/
				unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
				iNewCondListTrue =
					(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
			}
			
			/*条件列表原始的真假*/
			cond_logic_t *pCondLogic = NULL;
			if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
			{
				pCondLogic = &pstScene->stAndCond;
				iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
			}
			else
			{
				pCondLogic = &pstScene->stOrCond;
				iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
			}
			
			if(iNewCondListTrue != iOldCondListTrue)
			{
				HY_DEBUG("Cond List True is Changed, CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewCondListTrue, iOldCondListTrue);
				/*如果条件列表的真假发生改变
				*首先更新条件列表的真假
				*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
				*/
				/*更新条件列表的真假*/
				if(iNewCondListTrue)
				{
					pCondLogic->ucCondListTrue |= 0x1 << 1;
				}
				else
				{
					pCondLogic->ucCondListTrue &=  ~(0x1 << 1);
				}

				/*重新判断该条件列表所在的场景条件的真假*/
				cond_logic_t *pCondOr = &pstScene->stOrCond;
				cond_logic_t *pCondAnd = &pstScene->stAndCond;
				/**
				*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件为假
				*/
				if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					iNewSceneTrue = 0;
				}
				/**
				*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件的真假为或条件列表的真假
				*/
				else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					iNewSceneTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
				}
				/**
				*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件的真假为与条件列表的真假
				*/
				else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
					iNewSceneTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
				}
				/**
				*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
				*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
				*/
				else
				{
					unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
					iNewSceneTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
				}
				iOldSceneTrue = pstScene->ucSceneCondTrue ? 1 : 0;
				
				if(iNewSceneTrue != iOldSceneTrue)
				{
					HY_DEBUG("Scene Cond True is Changed, CurrNewSceneTrue = %d, CurrOldSceneTrue = %d\n", iNewSceneTrue, iOldSceneTrue);
					/*如果场景条件列表的真假发生改变
					*首先更新条件列表的真假
					*/
					pstScene->ucSceneCondTrue = iNewSceneTrue;
				}
				else
				{
					HY_DEBUG("Scene Cond True not Changed, CurrSceneTrue = %d\n", iNewSceneTrue);
				}
			}
			else
			{
				HY_DEBUG("Cond List True not Changed, CurrCondListTrue = %d.\n", iNewCondListTrue);
			}
		}
		else
		{
			HY_DEBUG("Cond Set True not Changed, CurrCondSetTrue = %d.\n", iNewCondSetTrue);
		}

		/*条件由假变真，且整个场景条件为真，则执行场景*/
		if(iNewCondTrue && !iOldCondTrue && pstScene->ucSceneCondTrue)
		{
			if(strstr(pstCondNode->stCond.acTriggerType, "Continue"))
			{
				HY_DEBUG("Activation timer, Timeout %ss\n", pstCondNode->stCond.acContinueTime);
				if(atoi(pstCondNode->stCond.acContinueTime) > 0)
				{
					/*状态持续指定时间后触发*/
					/*创建定时器，维护该状态持续判断*/
					scene_timer_param_t stSceneTimerParam = {0};
					stSceneTimerParam.pstSceneClass = _this;
					HY_DEBUG("stSceneTimerParam.pstSceneClass = %p\n", stSceneTimerParam.pstSceneClass);
					stSceneTimerParam.pstCondPtr = pstCondPtr;
					HY_DEBUG("stSceneTimerParam.pstCondPtr = %p\n", stSceneTimerParam.pstCondPtr);
					LOG_HEX(LOG_DEBUG, &stSceneTimerParam, 0, 8);
					scene_task_timer_create(_scene_continue_trigger_timer_fun,
						(void*)&stSceneTimerParam,
						sizeof(scene_timer_param_t),
						(unsigned long)atoi(pstCondNode->stCond.acContinueTime),
						TIMER_TASK_ONCE);
				}
				else
				{
					HY_DEBUG("Push exec queue.\n");
					/*立即触发*/
					_this->scene_exec(_this, pstScene);
				}
			}
			else if(strstr(pstCondNode->stCond.acTriggerType, "Instant"))
			{
				HY_DEBUG("Push exec queue.\n");
				/*立即触发*/
				_this->scene_exec(_this, pstScene);
			}
			
		}
	}
	if(1 == iEveryTrigger && 1 == iNewCondTrue)
	{
		/*重复触发类条件，只有触发一瞬间条件为真，触发完成后，立即回复为假*/
		scene_event_cond_true_update(_this, 0, pstCondPtr);
	}
	return NoErr;
}
/*************************************************************
*函数:	scene_time_cond_true_init
*参数:	iCondTrue:单一条件的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件的相关真假状态
*************************************************************/
static int
scene_time_cond_true_init(
	scene_list_class_t *_this, 
	int iCondTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_time_cond_true_init\n");

	int iNewCondTrue = 0;
	int iOldCondTrue = 0;
	int iNewCondSetTrue = 0;
	int iOldCondSetTrue = 0;
	int iNewCondListTrue = 0;
	int iOldCondListTrue = 0;
	int iNewSceneTrue = 0;
	int iOldSceneTrue = 0;
	
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);
	time_cond_node_t *pstCondNode = 
		(time_cond_node_t*)(pstCondPtr->stCondPtr.pCondNode);

	cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
	/*条件当前的真假*/
	iNewCondTrue = iCondTrue;
	/*条件原始的真假*/
	iOldCondTrue = 
		pCondSet->uiCondTrue & (0x1 << pstCondNode->iCondIndex) ? 1 : 0;
	
	HY_DEBUG("CurrNewCondTrue = %d, CurrOldCondTrue = %d\n", iNewCondTrue, iOldCondTrue);
	/*
	*首先更新条件的真假
	*其次由于条件的真假发生改变，所以必须重新判断该条件所在的条件集合的真假
	*/

	/*更新条件的真假*/
	if(iNewCondTrue)
	{
		
		pCondSet->uiCondTrue |= 
			0x1 << pstCondNode->iCondIndex;
	}
	else
	{
		pCondSet->uiCondTrue &=  
			~(0x1 << pstCondNode->iCondIndex);
	}

	/*重新判断该条件所在的条件集合的真假*/
	/*条件集合当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件真，则该条件集合真*/
		iNewCondSetTrue = 
			pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
	}
	else
	{
		/*与，所有条件为真，则该条件集合真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
		iNewCondSetTrue = 
			(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件集合原始的真假*/
	iOldCondSetTrue = 
		pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;
	HY_DEBUG("CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewCondSetTrue, iOldCondSetTrue);
	/*如果条件集合的真假发生改变
	*首先更新条件集合的真假
	*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
	*/
	/*更新条件集合的真假*/
	if(iNewCondSetTrue)
	{
		pstCondClass->uiCondSetTrue |= 
			0x1 << pstCondSetNode->iCondSetIndex;
	}
	else
	{
		pstCondClass->uiCondSetTrue &= 
			~(0x1 << pstCondSetNode->iCondSetIndex);
	}


	/*重新判断该条件集合所在的条件列表的真假*/
	/*条件列表当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件集合真，则该条件列表真*/
		iNewCondListTrue =
			pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
	}
	else
	{
		/*与，所有条件集合为真，则该条件列表真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
		iNewCondListTrue =
			(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件列表原始的真假*/
	cond_logic_t *pCondLogic = NULL;
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
	{
		pCondLogic = &pstScene->stAndCond;
		iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
	}
	else
	{
		pCondLogic = &pstScene->stOrCond;
		iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
	}
	
	HY_DEBUG("CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewCondListTrue, iOldCondListTrue);
	/*如果条件列表的真假发生改变
	*首先更新条件列表的真假
	*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
	*/
	/*更新条件列表的真假*/
	if(iNewCondListTrue)
	{
		pCondLogic->ucCondListTrue |= 0x1 << 0;
	}
	else
	{
		pCondLogic->ucCondListTrue &=  ~(0x1 << 0);
	}

	/*重新判断该条件列表所在的场景条件的真假*/
	cond_logic_t *pCondOr = &pstScene->stOrCond;
	cond_logic_t *pCondAnd = &pstScene->stAndCond;
	/**
	*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件为假
	*/
	if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewSceneTrue = 0;
	}
	/**
	*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为或条件列表的真假
	*/
	else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewSceneTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
	}
	/**
	*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为与条件列表的真假
	*/
	else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewSceneTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	/**
	*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
	*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
	*/
	else
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewSceneTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
	}
	iOldSceneTrue = pstScene->ucSceneCondTrue ? 1 : 0;
	
	HY_DEBUG("CurrNewSceneTrue = %d, CurrOldSceneTrue = %d\n", iNewSceneTrue, iOldSceneTrue);
	/*如果场景条件列表的真假发生改变
	*首先更新条件列表的真假
	*/
	pstScene->ucSceneCondTrue = iNewSceneTrue;



	
	return NoErr;
}

/*************************************************************
*函数:	scene_time_cond_true_update
*参数:	iCondTrue:单一条件的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件的相关真假状态
*************************************************************/
static int
scene_time_cond_true_update(
	scene_list_class_t *_this, 
	int iCondTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_cond_true_update\n");

	int iNewCondTrue = 0;
	int iOldCondTrue = 0;
	int iNewCondSetTrue = 0;
	int iOldCondSetTrue = 0;
	int iNewCondListTrue = 0;
	int iOldCondListTrue = 0;
	int iNewSceneTrue = 0;
	int iOldSceneTrue = 0;
	int iEveryTrigger = 0;
	
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);
	time_cond_node_t *pstCondNode = 
		(time_cond_node_t*)(pstCondPtr->stCondPtr.pCondNode);

	cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
	/*条件当前的真假*/
	iNewCondTrue = iCondTrue;
	/*条件原始的真假*/
	iOldCondTrue = 
		pCondSet->uiCondTrue & (0x1 << pstCondNode->iCondIndex) ? 1 : 0;
	
	if(strstr(pstCondNode->stCond.acTriggerType, "Every"))
	{
		/*每次收到事件上报都触发*/
		iEveryTrigger = 1;
	}
	
	if(iNewCondTrue != iOldCondTrue)
	{
		HY_DEBUG("Cond True is Changed, CurrNewCondTrue = %d, CurrOldCondTrue = %d\n", iNewCondTrue, iOldCondTrue);
		/*如果条件的真假发生改变
		*首先更新条件的真假
		*其次由于条件的真假发生改变，所以必须重新判断该条件所在的条件集合的真假
		*/

		/*更新条件的真假*/
		if(iNewCondTrue)
		{
			
			pCondSet->uiCondTrue |= 
				0x1 << pstCondNode->iCondIndex;
		}
		else
		{
			pCondSet->uiCondTrue &=  
				~(0x1 << pstCondNode->iCondIndex);
		}

		/*重新判断该条件所在的条件集合的真假*/
		/*条件集合当前的真假*/
		if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
		{
			/*或，任意一个条件真，则该条件集合真*/
			iNewCondSetTrue = 
				pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
		}
		else
		{
			/*与，所有条件为真，则该条件集合真*/
			unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
			iNewCondSetTrue = 
				(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
		}
		
		/*条件集合原始的真假*/
		iOldCondSetTrue = 
			pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;
		if(iNewCondSetTrue != iOldCondSetTrue)
		{
			HY_DEBUG("Cond Set True is Changed, CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewCondSetTrue, iOldCondSetTrue);
			/*如果条件集合的真假发生改变
			*首先更新条件集合的真假
			*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
			*/
			/*更新条件集合的真假*/
			if(iNewCondSetTrue)
			{
				pstCondClass->uiCondSetTrue |= 
					0x1 << pstCondSetNode->iCondSetIndex;
			}
			else
			{
				pstCondClass->uiCondSetTrue &= 
					~(0x1 << pstCondSetNode->iCondSetIndex);
			}


			/*重新判断该条件集合所在的条件列表的真假*/
			/*条件列表当前的真假*/
			if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
			{
				/*或，任意一个条件集合真，则该条件列表真*/
				iNewCondListTrue =
					pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
			}
			else
			{
				/*与，所有条件集合为真，则该条件列表真*/
				unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
				iNewCondListTrue =
					(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
			}
			
			/*条件列表原始的真假*/
			cond_logic_t *pCondLogic = NULL;
			if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
			{
				pCondLogic = &pstScene->stAndCond;
				iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
			}
			else
			{
				pCondLogic = &pstScene->stOrCond;
				iOldCondListTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
			}
			if(iNewCondListTrue != iOldCondListTrue)
			{
				HY_DEBUG("Cond List True is Changed, CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewCondListTrue, iOldCondListTrue);
				/*如果条件列表的真假发生改变
				*首先更新条件列表的真假
				*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
				*/
				/*更新条件列表的真假*/
				if(iNewCondListTrue)
				{
					pCondLogic->ucCondListTrue |= 0x1 << 0;
				}
				else
				{
					pCondLogic->ucCondListTrue &=  ~(0x1 << 0);
				}

				/*重新判断该条件列表所在的场景条件的真假*/
				cond_logic_t *pCondOr = &pstScene->stOrCond;
				cond_logic_t *pCondAnd = &pstScene->stAndCond;
				/**
				*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件为假
				*/
				if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					iNewSceneTrue = 0;
				}
				/**
				*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件的真假为或条件列表的真假
				*/
				else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					iNewSceneTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
				}
				/**
				*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
				*		则，该场景条件的真假为与条件列表的真假
				*/
				else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
					0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
				{
					unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
					iNewSceneTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
				}
				/**
				*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
				*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
				*/
				else
				{
					unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
					iNewSceneTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
				}
				iOldSceneTrue = pstScene->ucSceneCondTrue ? 1 : 0;
				
				if(iNewSceneTrue != iOldSceneTrue)
				{
					HY_DEBUG("Scene Cond True is Changed, CurrNewSceneTrue = %d, CurrOldSceneTrue = %d\n", iNewSceneTrue, iOldSceneTrue);
					/*如果场景条件列表的真假发生改变
					*首先更新条件列表的真假
					*/
					pstScene->ucSceneCondTrue = iNewSceneTrue;
				}
				else
				{
					HY_DEBUG("Scene Cond True not Changed, CurrSceneTrue = %d.\n", iNewSceneTrue);
				}
			}
			else
			{
				HY_DEBUG("Cond List True not Changed, CurrCondListTrue = %d.\n", iNewCondListTrue);
			}
		}
		else
		{
			HY_DEBUG("Cond Set True not Changed, CurrCondListTrue = %d.\n", iNewCondSetTrue);
		}
		/*条件由假变真，且整个场景条件为真，则执行场景*/
		if(iNewCondTrue && !iOldCondTrue && pstScene->ucSceneCondTrue)
		{
			HY_DEBUG("Push exec queue.\n");
			_this->scene_exec(_this, pstScene);
			pstCondNode->stCond.iTriggerInterval = atoi(pstCondNode->stCond.acTriggerInterval) - 1;
		}
	}
	else
	{
		HY_DEBUG("Cond True not Changed.\n");
		HY_DEBUG("CurrTriggerType = %s\n", pstCondNode->stCond.acTriggerType);
		/*如果触发类型是每次收到事件都会触发，且整个场景条件为真，则执行场景*/
		if(1 == iEveryTrigger && iNewCondTrue && pstScene->ucSceneCondTrue)
		{
			if(pstCondNode->stCond.iTriggerInterval <= 0)
			{
				HY_DEBUG("Push exec queue.\n");
				_this->scene_exec(_this, pstScene);
				pstCondNode->stCond.iTriggerInterval = atoi(pstCondNode->stCond.acTriggerInterval) - 1;
			}
			else
			{
				HY_DEBUG("TriggerInterval(%d) not reached.\n", pstCondNode->stCond.iTriggerInterval);
				pstCondNode->stCond.iTriggerInterval --;
			}
		}
	}
	return NoErr;
}
/*************************************************************
*函数:	scene_event_cond_set_true_update
*参数:	iCondSetTrue:条件集合的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件集合的相关真假状态
*************************************************************/
static int
scene_event_cond_set_true_init(
	int iCondSetTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_event_cond_set_true_init\n");
	int iNewTrue = 0;
	int iOldTrue = 0;
	
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);

		
	/*判断该条件所在的条件集合的真假*/
	/*条件集合当前的真假*/
	iNewTrue = 
		iCondSetTrue;
	
	/*条件集合原始的真假*/
	iOldTrue = 
		pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;

	HY_DEBUG("CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewTrue, iOldTrue);
	/*
	*首先更新条件集合的真假
	*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
	*/
	/*更新条件集合的真假*/
	if(iNewTrue)
	{
		pstCondClass->uiCondSetTrue |= 
			0x1 << pstCondSetNode->iCondSetIndex;
	}
	else
	{
		pstCondClass->uiCondSetTrue &= 
			~(0x1 << pstCondSetNode->iCondSetIndex);
	}


	/*重新判断该条件集合所在的条件列表的真假*/
	/*条件列表当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件集合真，则该条件列表真*/
		iNewTrue =
			pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
	}
	else
	{
		/*与，所有条件集合为真，则该条件列表真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
		iNewTrue =
			(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件列表原始的真假*/
	cond_logic_t *pCondLogic = NULL;
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
	{
		pCondLogic = &pstScene->stAndCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	else
	{
		pCondLogic = &pstScene->stOrCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	
	HY_DEBUG("CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewTrue, iOldTrue);
	/*
	*首先更新条件列表的真假
	*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
	*/
	/*更新条件列表的真假*/
	if(iNewTrue)
	{
		pCondLogic->ucCondListTrue |= 0x1 << 1;
	}
	else
	{
		pCondLogic->ucCondListTrue &=  ~(0x1 << 1);
	}

	/*重新判断该条件列表所在的场景条件的真假*/
	cond_logic_t *pCondOr = &pstScene->stOrCond;
	cond_logic_t *pCondAnd = &pstScene->stAndCond;
	/**
	*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件为假
	*/
	if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewTrue = 0;
	}
	/**
	*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为或条件列表的真假
	*/
	else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
	}
	/**
	*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为与条件列表的真假
	*/
	else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	/**
	*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
	*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
	*/
	else
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
	}
	iOldTrue = pstScene->ucSceneCondTrue ? 1 : 0;
	
	HY_DEBUG("CurrNewSceneCondTrue = %d, CurrOldSceneCondTrue = %d\n", iNewTrue, iOldTrue);
	/*如果场景条件列表的真假发生改变
	*首先更新条件列表的真假
	*/
	pstScene->ucSceneCondTrue = iNewTrue;


	return NoErr;
}

/*************************************************************
*函数:	scene_time_cond_set_true_update
*参数:	iCondSetTrue:条件集合的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件集合的相关真假状态
*************************************************************/
static int
scene_time_cond_set_true_init(
	int iCondSetTrue, 
	ptr_node_t *pstCondPtr
)
{
	HY_DEBUG("scene_time_cond_set_true_init\n");
	int iNewTrue = 0;
	int iOldTrue = 0;
	
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
	cond_list_node_t *pstCondSetNode = 
		(cond_list_node_t *)(pstCondPtr->stCondPtr.pCondSetNode);

		
	/*判断该条件所在的条件集合的真假*/
	/*条件集合当前的真假*/
	iNewTrue = 
		iCondSetTrue;
	/*条件集合原始的真假*/
	iOldTrue = 
		pstCondClass->uiCondSetTrue & (0x1 << pstCondSetNode->iCondSetIndex) ? 1 : 0;
	HY_DEBUG("CurrNewCondSetTrue = %d, CurrOldCondSetTrue = %d\n", iNewTrue, iOldTrue);
	/*如果条件集合的真假发生改变
	*首先更新条件集合的真假
	*其次由于条件集合的真假发生改变，所以必须重新判断该条件集合所在的条件列表的真假
	*/
	/*更新条件集合的真假*/
	if(iNewTrue)
	{
		pstCondClass->uiCondSetTrue |= 
			0x1 << pstCondSetNode->iCondSetIndex;
	}
	else
	{
		pstCondClass->uiCondSetTrue &= 
			~(0x1 << pstCondSetNode->iCondSetIndex);
	}


	/*重新判断该条件集合所在的条件列表的真假*/
	/*条件列表当前的真假*/
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_OR, LOGIC_MAX_LEN))
	{
		/*或，任意一个条件集合真，则该条件列表真*/
		iNewTrue =
			pstCondClass->uiCondSetTrue & (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass))) ? 1 : 0;
	}
	else
	{
		/*与，所有条件集合为真，则该条件列表真*/
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pstCondClass->stList.size((link_list_class_t*)(pstCondClass)));
		iNewTrue =
			(pstCondClass->uiCondSetTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	
	/*条件列表原始的真假*/
	cond_logic_t *pCondLogic = NULL;
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
	{
		pCondLogic = &pstScene->stAndCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
	}
	else
	{
		pCondLogic = &pstScene->stOrCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 0) ? 1 : 0;
	}
	
	HY_DEBUG("CurrNewCondListTrue = %d, CurrOldCondListTrue = %d\n", iNewTrue, iOldTrue);
	/*如果条件列表的真假发生改变
	*首先更新条件列表的真假
	*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
	*/
	/*更新条件列表的真假*/
	if(iNewTrue)
	{
		pCondLogic->ucCondListTrue |= 0x1 << 0;
	}
	else
	{
		pCondLogic->ucCondListTrue &=  ~(0x1 << 0);
	}

	/*重新判断该条件列表所在的场景条件的真假*/
	cond_logic_t *pCondOr = &pstScene->stOrCond;
	cond_logic_t *pCondAnd = &pstScene->stAndCond;
	/**
	*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件为假
	*/
	if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewTrue = 0;
	}
	/**
	*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为或条件列表的真假
	*/
	else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		iNewTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
	}
	/**
	*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
	*		则，该场景条件的真假为与条件列表的真假
	*/
	else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
		0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
	}
	/**
	*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
	*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
	*/
	else
	{
		unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
		iNewTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
	}
	iOldTrue = pstScene->ucSceneCondTrue ? 1 : 0;
	
	HY_DEBUG("CurrNewSceneCondTrue = %d, CurrOldSceneCondTrue = %d\n", iNewTrue, iOldTrue);
	/*如果场景条件列表的真假发生改变
	*首先更新条件列表的真假
	*/
	pstScene->ucSceneCondTrue = iNewTrue;


	return NoErr;
}
#if 0
/*************************************************************
*函数:	scene_cond_list_true_update
*参数:	iCondListTrue:条件列表的真假
*		pstCondPtr：条件的相关指针信息
*返回值:0表示成功，非0表示失败
*描述:	更新条件列表的相关真假状态
*************************************************************/
static int
scene_cond_list_true_update(
	int iCondListTrue, 
	ptr_node_t *pstCondPtr
)
{
	int iNewTrue = 0;
	int iOldTrue = 0;
	
	scene_info_t *pstScene = 
		(scene_info_t *)(pstCondPtr->stCondPtr.pstScene);
	cond_list_class_t *pstCondClass = 
		(cond_list_class_t *)(pstCondPtr->stCondPtr.pstCondList);
		

	/*判断该条件集合所在的条件列表的真假*/
	/*条件列表当前的真假*/
	iNewTrue =
		iCondListTrue;
	DEBUG("Cond List is %s\n", iNewTrue == 1 ? "True" : "False");
	/*条件列表原始的真假*/
	cond_logic_t *pCondLogic = NULL;
	if(!strncmp(pstCondClass->stCondInfo.acLogic, LOGIC_AND, LOGIC_MAX_LEN))
	{
		pCondLogic = &pstScene->stAndCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	else
	{
		pCondLogic = &pstScene->stOrCond;
		iOldTrue = pCondLogic->ucCondListTrue & (0x1 << 1) ? 1 : 0;
	}
	
	if(iNewTrue != iOldTrue)
	{
		DEBUG("Cond List True is Changed, iNewTrue = %d, iOldTrue = %d\n", iNewTrue, iOldTrue);
		/*如果条件列表的真假发生改变
		*首先更新条件列表的真假
		*其次由于条件列表的真假发生改变，所以必须重新判断该条件列表所在的场景条件的真假
		*/
		/*更新条件列表的真假*/
		if(iNewTrue)
		{
			pCondLogic->ucCondListTrue |= 0x1 << 1;
		}
		else
		{
			pCondLogic->ucCondListTrue &=  ~(0x1 << 1);
		}

		/*重新判断该条件列表所在的场景条件的真假*/
		cond_logic_t *pCondOr = &pstScene->stOrCond;
		cond_logic_t *pCondAnd = &pstScene->stAndCond;
		/**
		*情况1：如果时间或条件列表、事件或条件列表、时间与条件列表、事件与条件列表都为空，
		*		则，该场景条件为假
		*/
		if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
			0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
		{
			iNewTrue = 0;
		}
		/**
		*情况2：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表都为空，
		*		则，该场景条件的真假为或条件列表的真假
		*/
		else if(0 != pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
			0 == pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
		{
			iNewTrue = pCondOr->ucCondListTrue & (unsigned int)0xFFFFFFFF >> 30 ? 1 : 0;
		}
		/**
		*情况3：如果时间或条件列表、事件或条件列表都为空，并且时间与条件列表、事件与条件列表都为空，
		*		则，该场景条件的真假为与条件列表的真假
		*/
		else if(0 == pCondOr->pstTimeCondList->iCount + pCondOr->pstEventCondList->iCount && 
			0 != pCondAnd->pstTimeCondList->iCount + pCondAnd->pstEventCondList->iCount)
		{
			unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
			iNewTrue = (pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0;
		}
		/**
		*情况4：如果时间或条件列表、事件或条件列表不全为空，并且时间与条件列表、事件与条件列表也不全为空，
		*		则，该场景条件的真假为或条件列表的真假与上与条件列表的真假
		*/
		else
		{
			unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> 30;
			iNewTrue = (pCondOr->ucCondListTrue & uiTmp ? 1 : 0) && ((pCondAnd->ucCondListTrue & uiTmp) == uiTmp ? 1 : 0);
		}
		DEBUG("pCondOr->ucCondListTrue = %x, pCondAnd->ucCondListTrue = %x\n", pCondOr->ucCondListTrue, pCondAnd->ucCondListTrue);
		DEBUG("Scene Cond is %s\n", iNewTrue == 1 ? "True" : "False");
		iOldTrue = pstScene->ucSceneCondTrue ? 1 : 0;
		
		if(iNewTrue != iOldTrue)
		{
			DEBUG("Scene Cond True is Changed, iNewTrue = %d, iOldTrue = %d\n", iNewTrue, iOldTrue);
			/*如果场景条件列表的真假发生改变
			*首先更新条件列表的真假
			*/
			pstScene->ucSceneCondTrue = iNewTrue;
		}
	}
	


	return NoErr;
}
#endif

static int
scene_list_table_db(
	scene_list_class_t *_this
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	int iRow = 0;
	int iRank = 0;
	iRow = 1024;
	iRank = 1;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return -1;
	}
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT id FROM %s;",
		DB_TABLE_NAME_SCENE);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				id varchar(%d),\
				scene_name varchar(%d),\
				scene_enable varchar(%d),\
				exec_delayed varchar(%d),\
				update_time varchar(%d),\
				scene_note varchar(%d)\
			);", 
			DB_TABLE_NAME_SCENE, 
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN);
		iRet = db_add_asyn(_this->pDb, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_SCENE);
		}
	}

	/*创建表*/
	iRow = 1024;
	iRank = 1;
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT id FROM %s;",
		DB_TABLE_NAME_COND);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				id varchar(%d),\
				logic varchar(%d),\
				cond_type varchar(%d),\
				trigger_type varchar(%d),\
				continue_time varchar(%d),\
				trigger_interval varchar(%d),\
				time_key varchar(%d),\
				start_hour varchar(%d),\
				start_minu varchar(%d),\
				end_hour varchar(%d),\
				end_minu varchar(%d),\
				week varchar(%d),\
				repeat varchar(%d),\
				time_lose varchar(%d),\
				dev_id varchar(%d),\
				key varchar(%d),\
				value varchar(%d),\
				active varchar(%d),\
				last_value varchar(%d)\
			);", 
			DB_TABLE_NAME_COND, 
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN);
		iRet = db_add_asyn(_this->pDb, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_COND);
		}
	}

	/*创建表*/
	iRow = 1024;
	iRank = 1;
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT id FROM %s;",
		DB_TABLE_NAME_ACTION);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				id varchar(%d) NOT NULL,\
				action_id varchar(%d),\
				dev_id varchar(%d) NOT NULL,\
				key varchar(%d) NOT NULL,\
				value_coding varchar(%d),\
				value varchar(%d)\
			);", 
			DB_TABLE_NAME_ACTION, 
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN);
		iRet = db_add_asyn(_this->pDb, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_ACTION);
		}
	}

	iRow = 1024;
	iRank = 1;
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT id FROM %s;",
		DB_TABLE_NAME_BIND_SCENE);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		memset(acSql, 0x0, sizeof(acSql));
		snprintf(acSql, SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				id varchar(%d),\
				dev_id varchar(%d),\
				key varchar(%d),\
				value varchar(%d),\
				enable varchar(%d)\
			);", 
			DB_TABLE_NAME_BIND_SCENE, 
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN
			);
		iRet = db_add_asyn(_this->pDb, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_BIND_SCENE);
			return SceneAddErr;
		}
	}
	
	if(astData)
	{
		free(astData);
		astData = NULL;
	}

	return NoErr;
}

static int
scene_list_add_db(
	scene_list_class_t *_this, 
	scene_info_t *pstInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	
	/*使用insert语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		INSERT INTO %s \
			(id, scene_name, scene_enable, exec_delayed, update_time, scene_note) \
		VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s');\
		", 
		DB_TABLE_NAME_SCENE,
		pstInfo->acSceneId,
		pstInfo->acSceneName,
		pstInfo->acSceneEnable,
		pstInfo->acExecDelayed,
		pstInfo->acUpdateTime,
		pstInfo->acNote
	);
	iRet = db_add_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("INSERT DB Table(%s) failed.\n", 
			DB_TABLE_NAME_SCENE);
		return SceneAddErr;
	}
	
	return NoErr;
}
static int 
scene_list_del_db(
	scene_list_class_t *_this, 
	scene_info_t *pstInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);
	
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE id='%s';\
		", 
		DB_TABLE_NAME_SCENE,
		pstInfo->acSceneId);
	iRet = db_del_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_SCENE);
		return SceneDelErr;
	}

	return NoErr;
}

static int 
scene_list_clear_db(
	scene_list_class_t *_this
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DROP TABLE %s;\
		", 
		DB_TABLE_NAME_SCENE);
	iRet = db_del_asyn(_this->pDb, acSql);

	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DROP TABLE %s;\
		", 
		DB_TABLE_NAME_BIND_SCENE);
	iRet = db_del_asyn(_this->pDb, acSql);

	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DROP TABLE %s;\
		", 
		DB_TABLE_NAME_ACTION);
	iRet = db_del_asyn(_this->pDb, acSql);


	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DROP TABLE %s;\
		", 
		DB_TABLE_NAME_COND);
	iRet = db_del_asyn(_this->pDb, acSql);
	
	return iRet;
}

static int 
scene_list_set_db(
	scene_list_class_t *_this, 
	scene_info_t *pstInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);
	
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*查找到条目，使用update语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET scene_name='%s', scene_enable='%s', exec_delayed='%s', update_time='%s', scene_note='%s'\
		WHERE id='%s';\
		", 
		DB_TABLE_NAME_SCENE,
		pstInfo->acSceneName,
		pstInfo->acSceneEnable,
		pstInfo->acExecDelayed,
		pstInfo->acUpdateTime,
		pstInfo->acNote,
		pstInfo->acSceneId);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_SCENE);
		return SceneAddErr;
	}
		
	return NoErr;
}

static int 
scene_list_get_db(
	scene_list_class_t *_this, 
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 5;
	db_data_entry_t astData[iRow * iRank];
	memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT scene_name, scene_enable, exec_delayed, update_time, scene_note FROM %s\
		WHERE id='%s';",
		DB_TABLE_NAME_SCENE,
		pstInfo->acSceneId);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_SCENE);
		return SceneGetErr;
	}

	if(NoErr == iRow)
	{
		HY_ERROR("The Scene Info(%s %s) not found.\n",
			pstInfo->acSceneId);
		return NotFoundErr;
	}

	strncpy(pstInfo->acSceneName, 
		astData[0].acData, NAME_MAX_LEN);
	strncpy(pstInfo->acSceneEnable,
		astData[1].acData, STATE_MAX_LEN);
	strncpy(pstInfo->acExecDelayed,
		astData[2].acData, DELAYED_MAX_LEN);
	strncpy(pstInfo->acUpdateTime,
		astData[3].acData, TIME_MAX_LEN);
	strncpy(pstInfo->acNote,
		astData[4].acData, NOTE_MAX_LEN);
	return NoErr;
}

static int 
scene_list_get_list_db(
	scene_list_class_t *_this, 
	scene_info_t *pastInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastInfo, piCount);

	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	scene_info_t *pstEntry = pastInfo;
	char acSql[SQL_MAX_LEN] = {0};

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = SCENE_MAX_NUM;
	iRank = 6;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT id, scene_name, scene_enable, exec_delayed, update_time, scene_note FROM %s;",
		DB_TABLE_NAME_SCENE);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_SCENE);
		*piCount = 0;
		if(astData)
		{
			free(astData);
			astData = NULL;
		}
		return NotFoundErr;
	}
	HY_DEBUG("iRow = %d, iRank = %d\n", iRow, iRank);
	for(i = 0; i < iRow && iInfoCount < iInfoMax; ++i)
	{
		if(strcmp(astData[(iRank * i) + 0].acData, ""))
		{
			strncpy(pstEntry->acSceneId, 
				astData[(iRank * i) + 0].acData, 
				INDEX_MAX_LEN
			);
			strncpy(pstEntry->acSceneName, 
				astData[(iRank * i) + 1].acData, 
				NAME_MAX_LEN
			);
			strncpy(pstEntry->acSceneEnable, 
				astData[(iRank * i) + 2].acData,
				STATE_MAX_LEN
			);
			strncpy(pstEntry->acExecDelayed, 
				astData[(iRank * i) + 3].acData,
				DELAYED_MAX_LEN
			);
			strncpy(pstEntry->acUpdateTime, 
				astData[(iRank * i) + 4].acData,
				TIME_MAX_LEN
			);
			strncpy(pstEntry->acNote, 
				astData[(iRank * i) + 5].acData,
				NOTE_MAX_LEN
			);
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

static scene_list_node_t* 
scene_list_info_set(
	scene_list_class_t *_this, 
	scene_list_node_t *pstNode, 
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pstNode, pstInfo);
	/*加写锁*/
	_this->stList.write_lock((link_list_class_t *)_this);
	memcpy(&(pstNode->stSceneInfo), pstInfo, sizeof(scene_info_t));
	_this->stList.write_unlock((link_list_class_t *)_this);

	return pstNode;
}

static int 
scene_list_info_get(
	scene_list_class_t *_this, 
	scene_list_node_t *pstNode, 
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstNode, pstInfo);
	/*加读锁*/
	_this->stList.write_lock((link_list_class_t *)_this);
	memcpy(pstInfo, &(pstNode->stSceneInfo), sizeof(scene_info_t));
	_this->stList.write_unlock((link_list_class_t *)_this);

	return NoErr;
}


static int 
scene_bind_db(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	scene_panel_t *pstPanelInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstPanelInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};


	/*未查找到条目，使用insert语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		INSERT INTO %s \
			(id, dev_id, key, value, enable) \
		VALUES \
			('%s', '%s', '%s', '%s', '%s');\
		", 
		DB_TABLE_NAME_BIND_SCENE,
		pstCurrScene->acSceneId,
		pstPanelInfo->acDevId,
		pstPanelInfo->acKey,
		pstPanelInfo->acValue,
		pstPanelInfo->acEnable
	);
	iRet = db_add_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("INSERT DB Table(%s) failed.\n", 
			DB_TABLE_NAME_BIND_SCENE);
		return SceneAddErr;
	}
	
	return NoErr;
}

static int 
scene_bind_set_db(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	scene_panel_t *pstPanelInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstPanelInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};

	/*查找到条目，使用update语句*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		UPDATE %s \
		SET enable='%s'\
		WHERE id='%s' and dev_id='%s' and key='%s' and value='%s';\
		", 
		DB_TABLE_NAME_BIND_SCENE,
		pstPanelInfo->acEnable,
		pstCurrScene->acSceneId,
		pstPanelInfo->acDevId,
		pstPanelInfo->acKey,
		pstPanelInfo->acValue);
	iRet = db_set_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_BIND_SCENE);
		return SceneAddErr;
	}
	
	return NoErr;
}

static int 
scene_unbind_db(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	scene_panel_t *pstPanelInfo
)
{
	/*初始化阶段，不修改数据库*/
	if(1 == _this->ucInitFlag)
	{
		return NoErr;
	}
	
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstPanelInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	

	/*删除条目*/
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE id='%s' and dev_id='%s' and key='%s' and value='%s';", 
		DB_TABLE_NAME_BIND_SCENE,
		pstCurrScene->acSceneId,
		pstPanelInfo->acDevId,
		pstPanelInfo->acKey,
		pstPanelInfo->acValue
	);
	iRet = db_del_asyn(_this->pDb, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_BIND_SCENE);
		return SceneDelErr;
	}
	
	return NoErr;
}

static int 
scene_bind_get_db(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	scene_panel_t *pstPanelInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstPanelInfo);

	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	
	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 1;
	db_data_entry_t astData[iRow * iRank];
	memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT enable FROM %s\
		WHERE id='%s' and dev_id='%s' and key='%s' and value='%s';",
		DB_TABLE_NAME_BIND_SCENE,
		pstCurrScene->acSceneId,
		pstPanelInfo->acDevId,
		pstPanelInfo->acKey,
		pstPanelInfo->acValue
	);
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_BIND_SCENE);
		return SceneGetErr;
	}

	if(NoErr == iRow)
	{
		HY_ERROR("The Scene Bind Info(%s %s %s) not found.\n",
			pstPanelInfo->acDevId,
			pstPanelInfo->acKey,
			pstPanelInfo->acValue
		);
		return NotFoundErr;
	}

	strncpy(pstPanelInfo->acEnable, 
		astData[0].acData, NAME_MAX_LEN);
	
	return NoErr;
}

static int 
scene_bind_get_list_db(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	scene_panel_t *pastPanelInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(_this, pstCurrScene, pastPanelInfo, piCount);

	int i = 0;
	int iRet = 0;
	char acSql[SQL_MAX_LEN] = {0};
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	scene_panel_t *pstEntry = pastPanelInfo;

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = SCENE_BIND_MAX_NUM;
	iRank = 4;
	db_data_entry_t *astData = (db_data_entry_t *)calloc(iRow * iRank, sizeof(db_data_entry_t));
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
	memset(acSql, 0x0, sizeof(acSql));
	snprintf(acSql, SQL_MAX_LEN, 
		"SELECT dev_id, key, value, enable FROM %s\
		WHERE id='%s';",
		DB_TABLE_NAME_BIND_SCENE,
		pstCurrScene->acSceneId
	);
	
	iRet = db_get(_this->pDb, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_BIND_SCENE);
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
			strncpy(pstEntry->acDevId, 
				astData[(iRank * i) + 0].acData, 
				INDEX_MAX_LEN
			);
			strncpy(pstEntry->acKey, 
				astData[(iRank * i) + 1].acData, 
				NAME_MAX_LEN
			);
			strncpy(pstEntry->acValue, 
				astData[(iRank * i) + 2].acData,
				VALUE_MAX_LEN
			);
			strncpy(pstEntry->acEnable, 
				astData[(iRank * i) + 3].acData,
				STATE_MAX_LEN
			);
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


/*锁住场景*/
static void 
scene_list_lock(scene_list_class_t *_this)
{
	PARAM_CHECK_RETURN_VOID_1(_this);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	base_mutex_lock(pPrivateParam->mutex);
}
/*解锁场景*/
static void 
scene_list_unlock(scene_list_class_t *_this)
{
	PARAM_CHECK_RETURN_VOID_1(_this);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	base_mutex_unlock(pPrivateParam->mutex);
}

static int 
scene_list_init(scene_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;

	int iRet = 0;

	/*开始初始化操作*/
	_this->ucInitFlag = 1;
	
	/*初始化数据库表*/
	pPrivateMethods->scene_table_db(_this);

	/*获取场景数据*/
	int iCount = SCENE_MAX_NUM;
	scene_info_t *astInfo = (scene_info_t *)calloc(SCENE_MAX_NUM, sizeof(scene_info_t));
	if(NULL == astInfo)
	{
		HY_ERROR("calloc error.\n");
		iRet = HeapReqErr;
		goto scene_list_init_end;
	}

	pPrivateMethods->scene_get_list_db(_this, astInfo, &iCount);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("SceneId = %s\n", astInfo[i].acSceneId);
		HY_DEBUG("SceneName = %s\n", astInfo[i].acSceneName);
		HY_DEBUG("SceneEnable = %s\n", astInfo[i].acSceneEnable);
		HY_DEBUG("ExecDelayed = %s\n", astInfo[i].acExecDelayed);
		HY_DEBUG("acUpdateTime = %s\n", astInfo[i].acUpdateTime);
		HY_DEBUG("acNote = %s\n", astInfo[i].acNote);
	}
	
	iRet = 
		_this->scene_sync(_this, astInfo, iCount);
	if(NoErr != iRet)
	{
		HY_ERROR("Scene Sync faild\n");
		goto scene_list_init_end;
	}
	
	
	
	scene_list_node_t *pstNode = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);
	while(pstNode && pstHead != pstNode)
	{
		/*同步该场景下所有时间或条件*/
		_this->time_or_cond_init(
			_this, 
			&pstNode->stSceneInfo
		);
		
		/*同步该场景下所有事件或条件*/
		_this->event_or_cond_init(
			_this, 
			&pstNode->stSceneInfo
		);
		
		/*同步该场景下所有时间与条件*/
		_this->time_and_cond_init(
			_this,
			&pstNode->stSceneInfo
		);
		
		/*同步该场景下所有事件与条件*/
		_this->event_and_cond_init(
			_this, 
			&pstNode->stSceneInfo
		);
		
		/*同步该场景下所有动作*/
		pstNode->stSceneInfo.pstAction->action_init(
			pstNode->stSceneInfo.pstAction
		);

		/*同步该场景下所有绑定的场景面板*/
		_this->scene_bind_init(
			_this, 
			&pstNode->stSceneInfo
		);
		
		pstNode = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
scene_list_init_end:
	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}
	
	/*结束初始化操作*/
	_this->ucInitFlag = 0;
	
	return iRet;
}

static int 
scene_list_sync(
	scene_list_class_t *_this, 
	scene_info_t *pastInfo,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pastInfo);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iCount);

	int i = 0;
	
	if(0 == 
		_this->stList.size(
			(link_list_class_t *)_this
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->scene_add(_this, &pastInfo[i]);
			HY_INFO("Add Scene(%s)\n", pastInfo[i].acSceneId);
		}
		
		return NoErr;
	}
	if(0 == iCount)
	{
		_this->scene_clear(_this, 1);
		HY_INFO("Clear Scene\n");
		return NoErr;
	}
	char *pacFoundFlag =
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}
	

	scene_list_node_t *pstNode = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);
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
			if(!strncmp(
					pstNode->stSceneInfo.acSceneId,
					pastInfo[i].acSceneId, 
					INDEX_MAX_LEN
				)
			)
			{
				/*找到,判断数据是否发生改变*/
				if(strncmp(
						pstNode->stSceneInfo.acSceneName,
						pastInfo[i].acSceneName,
						NAME_MAX_LEN
					) ||
					strncmp(
						pstNode->stSceneInfo.acSceneEnable,
						pastInfo[i].acSceneEnable,
						STATE_MAX_LEN
					) ||
					strncmp(
						pstNode->stSceneInfo.acExecDelayed,
						pastInfo[i].acExecDelayed,
						DELAYED_MAX_LEN
					) ||
					strncmp(
						pstNode->stSceneInfo.acUpdateTime,
						pastInfo[i].acUpdateTime,
						TIME_MAX_LEN
					) ||
					strncmp(
						pstNode->stSceneInfo.acNote,
						pastInfo[i].acNote,
						NOTE_MAX_LEN
					)
				)
				{
					/*信息发生改变*/
					strncpy(
						pstNode->stSceneInfo.acSceneName,
						pastInfo[i].acSceneName,
						NAME_MAX_LEN
					);
					strncpy(
						pstNode->stSceneInfo.acSceneEnable, 
						pastInfo[i].acSceneEnable,
						STATE_MAX_LEN
					);
					strncpy(
						pstNode->stSceneInfo.acExecDelayed, 
						pastInfo[i].acExecDelayed,
						DELAYED_MAX_LEN
					);
					strncpy(
						pstNode->stSceneInfo.acUpdateTime, 
						pastInfo[i].acUpdateTime,
						TIME_MAX_LEN
					);
					strncpy(
						pstNode->stSceneInfo.acNote, 
						pastInfo[i].acNote,
						NOT_WITHIN
					);
					_this->scene_set(_this, &pastInfo[i]);
					HY_INFO("Set Scene(%s)\n", pastInfo[i].acSceneId);
				}
				pacFoundFlag[i] = 1;
				iFlag = 1;
				break;
			}
		}
		if(!iFlag)
		{
			/*未找到，删除*/
			_this->scene_del(_this, pastInfo[i].acSceneId);
			HY_INFO("Del Scene(%s)\n", pastInfo[i].acSceneId);
		}
	
		pstNode = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->scene_add(_this, &pastInfo[i]);
			HY_INFO("Add Scene(%s)\n", pastInfo[i].acSceneId);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;
}

static int 
scene_list_clear(scene_list_class_t *_this, int iClearDbFlag)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	scene_list_node_t *pstNode = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);
	while(pstNode && pstHead != pstNode)
	{
		/*删除该场景下所有时间或条件*/
		_this->time_or_cond_clear(
			_this, 
			&pstNode->stSceneInfo,
			1
		);

		destroy_cond_list(
			pstNode->stSceneInfo.stOrCond.pstTimeCondList
		);
		
		/*删除该场景下所有事件或条件*/
		_this->event_or_cond_clear(
			_this, 
			&pstNode->stSceneInfo,
			1
		);
		
		destroy_cond_list(
			pstNode->stSceneInfo.stOrCond.pstEventCondList
		);
		
		/*删除该场景下所有时间与条件*/
		_this->time_and_cond_clear(
			_this,
			&pstNode->stSceneInfo,
			1
		);
		
		destroy_cond_list(
			pstNode->stSceneInfo.stAndCond.pstTimeCondList
		);
		
		/*删除该场景下所有事件与条件*/
		_this->event_and_cond_clear(
			_this,
			&pstNode->stSceneInfo,
			1
		);
		
		destroy_cond_list(
			pstNode->stSceneInfo.stAndCond.pstEventCondList
		);
		
		/*删除该场景下所有动作*/
		_this->action_clear(
			_this,
			&pstNode->stSceneInfo,
			1
		);
		destroy_action_list(pstNode->stSceneInfo.pstAction);
		/*删除该场景下绑定的所有场景面板*/
		_this->scene_bind_clear(
			_this,
			&pstNode->stSceneInfo,
			1
		);
		
		/*删除场景数据库*/
		if(1 == iClearDbFlag)
		{
			pPrivateMethods->scene_del_db(_this, &pstNode->stSceneInfo);
		}
		
		pstNode = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
		
	return _this->stList.clear((link_list_class_t *)_this);
}

/*删除所有场景中，该设备的相关信息*/
static int 
scene_list_dev_unregister(scene_list_class_t *_this, char *pDevId)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	int i = 0;
	int iCount = COND_MAX_NUM;
	event_cond_t *astCondInfo = (event_cond_t *)calloc(COND_MAX_NUM, sizeof(event_cond_t));
	if(NULL == astCondInfo)
	{
		HY_ERROR("Calloc error.");
		return HeapReqErr;
	}

	action_info_t *astActionInfo = (action_info_t *)calloc(ACTION_MAX_NUM, sizeof(action_info_t));
	if(NULL == astActionInfo)
	{
		HY_ERROR("Calloc error.\n");
		return HeapReqErr;
	}
	scene_panel_t *astPanelInfo	= (scene_panel_t *)calloc(SCENE_BIND_MAX_NUM, sizeof(scene_panel_t));
	if(NULL == astActionInfo)
	{
		HY_ERROR("Calloc error.\n");
		return HeapReqErr;
	}
	scene_list_node_t *pstNode = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);

	/*遍历所有场景*/
	while(pstNode && pstHead != pstNode)
	{
		/*获取事件或条件列表*/
		iCount = COND_MAX_NUM;
		memset(astCondInfo, 0x0, COND_MAX_NUM * sizeof(event_cond_t));
		
		_this->event_or_cond_get_list(
			_this, 
			&pstNode->stSceneInfo,
			astCondInfo, 
			&iCount
		);
		HY_DEBUG("iCount = %d\n", iCount);
		for(i = 0; i < iCount; ++i)
		{
			HY_DEBUG("TriggerType = %s\n", astCondInfo[i].acTriggerType);
			HY_DEBUG("ContinueTime = %s\n", astCondInfo[i].acContinueTime);
			HY_DEBUG("DevId = %s\n", astCondInfo[i].acDevId);
			HY_DEBUG("Key = %s\n", astCondInfo[i].acKey);
			HY_DEBUG("Value = %s\n", astCondInfo[i].acValue);
			HY_DEBUG("Active = %s\n", astCondInfo[i].acActive);
			HY_DEBUG("LastValue = %s\n", astCondInfo[i].acLastValue);
			
			if(!strncmp(astCondInfo[i].acDevId, pDevId, DEV_ID_MAX_LEN))
			{
				/*删除该事件或条件*/
				HY_DEBUG("Event Or Cond Del\n");
				_this->event_or_cond_del(_this, &pstNode->stSceneInfo, &astCondInfo[i]);
			}
		}
		/*获取事件与条件列表*/
		iCount = COND_MAX_NUM;
		memset(astCondInfo, 0x0, COND_MAX_NUM * sizeof(event_cond_t));
		_this->event_and_cond_get_list(
			_this, 
			&pstNode->stSceneInfo,
			astCondInfo, 
			&iCount
		);
		HY_DEBUG("iCount = %d\n", iCount);
		for(i = 0; i < iCount; ++i)
		{
			HY_DEBUG("TriggerType = %s\n", astCondInfo[i].acTriggerType);
			HY_DEBUG("ContinueTime = %s\n", astCondInfo[i].acContinueTime);
			HY_DEBUG("DevId = %s\n", astCondInfo[i].acDevId);
			HY_DEBUG("Key = %s\n", astCondInfo[i].acKey);
			HY_DEBUG("Value = %s\n", astCondInfo[i].acValue);
			HY_DEBUG("Active = %s\n", astCondInfo[i].acActive);
			HY_DEBUG("LastValue = %s\n", astCondInfo[i].acLastValue);

			if(!strncmp(astCondInfo[i].acDevId, pDevId, DEV_ID_MAX_LEN))
			{
				/*删除该事件与条件*/
				HY_DEBUG("Event And Cond Del\n");
				_this->event_and_cond_del(_this, &pstNode->stSceneInfo, &astCondInfo[i]);
			}
		}

		
		/*获取动作列表*/
		iCount = ACTION_MAX_NUM;
		memset(astActionInfo, 0x0, ACTION_MAX_NUM * sizeof(action_info_t));
		action_list_class_t *pstCurrAction = pstNode->stSceneInfo.pstAction;
		pstCurrAction->action_get_list(pstCurrAction, astActionInfo, &iCount);

		HY_DEBUG("iCount = %d\n", iCount);
		for(i = 0; i < iCount; ++i)
		{
			HY_DEBUG("ActionId = %s\n", astActionInfo[i].acActionId);
			HY_DEBUG("DevId = %s\n", astActionInfo[i].acDevId);
			HY_DEBUG("Key = %s\n", astActionInfo[i].acKey);
			HY_DEBUG("ValueCoding = %s\n", astActionInfo[i].acValueCoding);
			HY_DEBUG("Value = %s\n", astActionInfo[i].acValue);

			if(!strncmp(astActionInfo[i].acDevId, pDevId, DEV_ID_MAX_LEN))
			{
				/*删除该动作*/
				HY_DEBUG("Action Del\n");
				pstCurrAction->action_del(pstCurrAction, &astActionInfo[i]);
			}
		}

		/*获取场景实体面板绑定列表*/
		iCount = SCENE_BIND_MAX_NUM;
		memset(astPanelInfo, 0x0, SCENE_BIND_MAX_NUM * sizeof(scene_panel_t));
		_this->scene_bind_get_list(_this, &pstNode->stSceneInfo, astPanelInfo, &iCount);

		HY_DEBUG("iCount = %d\n", iCount);
		for(i = 0; i < iCount; ++i)
		{
			HY_DEBUG("DevId = %s\n", astPanelInfo[i].acDevId);
			HY_DEBUG("Key = %s\n", astPanelInfo[i].acKey);
			HY_DEBUG("Value = %s\n", astPanelInfo[i].acValue);
			HY_DEBUG("Enable = %s\n", astPanelInfo[i].acEnable);

			if(!strncmp(astPanelInfo[i].acDevId, pDevId, DEV_ID_MAX_LEN))
			{
				/*解除场景面板的绑定*/
				HY_DEBUG("Scene Unbind\n");
				_this->scene_unbind(_this, &pstNode->stSceneInfo, &astPanelInfo[i]);
			}
		}
		
		pstNode = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	if(astCondInfo)
	{
		free(astCondInfo);
		astCondInfo = NULL;
	}
	
	if(astActionInfo)
	{
		free(astActionInfo);
		astActionInfo = NULL;
	}

	return NoErr;
}

static scene_info_t * 
scene_list_add(
	scene_list_class_t *_this,
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pstInfo);
	int iRet = 0;

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;

	/*为了确保设备信息的唯一性*/
	//scene_info_t *pstSeneGet = 
	//	_this->scene_get(_this, pstInfo->acSceneId);
	//if(NULL != pstSeneGet)
	//{
		/*设置*/
	//	iRet = _this->scene_set(_this, pstInfo);
	//	if(NoErr != iRet)
	//	{
	//		ERROR("Scene Info add failed.\n");
	//		return NULL;
	//	}
	//	return pstSeneGet;
	//}
	//else
	//修改：将原来的添加修改功能改为单一的添加功能
	{
		scene_info_t stScene;
		memset(&stScene, 0x0, sizeof(scene_info_t));
		strncpy(stScene.acSceneId, 
			pstInfo->acSceneId, INDEX_MAX_LEN);
		strncpy(stScene.acSceneName, 
			pstInfo->acSceneName, NAME_MAX_LEN);
		strncpy(stScene.acSceneEnable, 
			pstInfo->acSceneEnable, STATE_MAX_LEN);
		strncpy(stScene.acExecDelayed, 
			pstInfo->acExecDelayed, DELAYED_MAX_LEN);
		strncpy(stScene.acUpdateTime, 
			pstInfo->acUpdateTime, TIME_MAX_LEN);
		strncpy(stScene.acNote, 
			pstInfo->acNote, NOTE_MAX_LEN);
		scene_list_node_t *pNewNode = 
			pPrivateMethods->scene_info_set(
					_this, 
					(scene_list_node_t *)(
						_this->stList.new_node(
							(link_list_class_t *)_this
						)
					),
					&stScene
				);
		
		cond_info_t stCondInfo;
		
		/*初始化时间或条件集合*/
		memset(&stCondInfo, 0x0, sizeof(cond_info_t));
		memcpy(stCondInfo.acId, 
			pstInfo->acSceneId, INDEX_MAX_LEN);
		memcpy(stCondInfo.acLogic,
			LOGIC_OR, LOGIC_MAX_LEN);
		memcpy(stCondInfo.acCondType,
			COND_TIME, COND_TYPE_MAX_LEN);
		pNewNode->stSceneInfo.stOrCond.pstTimeCondList = 
			new_cond_list(_this->pDb, &stCondInfo);
		if(NULL == 
			pNewNode->stSceneInfo.stOrCond.pstTimeCondList)
		{
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			HY_ERROR("New the 'or' cond set of time error.\n");
			return NULL;
		}
		
		/*初始化事件或条件集合*/
		memset(&stCondInfo, 0x0, sizeof(cond_info_t));
		memcpy(stCondInfo.acId, 
			pstInfo->acSceneId, INDEX_MAX_LEN);
		memcpy(stCondInfo.acLogic, 
			LOGIC_OR, LOGIC_MAX_LEN);
		memcpy(stCondInfo.acCondType,
			COND_EVENT, COND_TYPE_MAX_LEN);
		pNewNode->stSceneInfo.stOrCond.pstEventCondList = 
			new_cond_list(_this->pDb, &stCondInfo);
		if(NULL == 
			pNewNode->stSceneInfo.stOrCond.pstEventCondList)
		{
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstTimeCondList
			);
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			
			HY_ERROR("New the 'or' cond set of event error.\n");
			return NULL;
		}
		
		/*初始化时间与条件集合*/
		memset(&stCondInfo, 0x0, sizeof(cond_info_t));
		memcpy(stCondInfo.acId, 
			pstInfo->acSceneId, INDEX_MAX_LEN);
		memcpy(stCondInfo.acLogic,
			LOGIC_AND, LOGIC_MAX_LEN);
		memcpy(stCondInfo.acCondType,
			COND_TIME, COND_TYPE_MAX_LEN);
		pNewNode->stSceneInfo.stAndCond.pstTimeCondList = 
			new_cond_list(_this->pDb, &stCondInfo);
		if(NULL ==
			pNewNode->stSceneInfo.stAndCond.pstTimeCondList)
		{
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstEventCondList
			);
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			
			HY_ERROR("New the 'and' cond set of time error.\n");
			return NULL;
		}
		
		/*初始化事件与条件集合*/
		memset(&stCondInfo, 0x0, sizeof(cond_info_t));
		memcpy(stCondInfo.acId,
			pstInfo->acSceneId, INDEX_MAX_LEN);
		memcpy(stCondInfo.acLogic, 
			LOGIC_AND, LOGIC_MAX_LEN);
		memcpy(stCondInfo.acCondType, 
			COND_EVENT, COND_TYPE_MAX_LEN);
		pNewNode->stSceneInfo.stAndCond.pstEventCondList = 
			new_cond_list(_this->pDb, &stCondInfo);
		if(NULL == 
			pNewNode->stSceneInfo.stAndCond.pstTimeCondList)
		{
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstEventCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stAndCond.pstTimeCondList
			);
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			
			HY_ERROR("New the 'and' cond set of event error.\n");
			return NULL;
		}
		/*初始化动作链表*/
		pNewNode->stSceneInfo.pstAction = 
			new_action_list(_this->pDb, pstInfo->acSceneId);
		if(NULL == pNewNode->stSceneInfo.pstAction)
		{
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstEventCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stAndCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stAndCond.pstEventCondList
			);
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			
			HY_ERROR("New the 'and' cond set of event error.\n");
			return NULL;
		}
		/*添加*/
		iRet = 
			_this->stList.inst_head(
				(link_list_class_t *)_this,
				(link_list_piece_t*)(pNewNode)
			);
		if(NoErr != iRet)
		{
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stOrCond.pstEventCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stAndCond.pstTimeCondList
			);
			destroy_cond_list(
				pNewNode->stSceneInfo.stAndCond.pstEventCondList
			);
			destroy_action_list(pNewNode->stSceneInfo.pstAction);
			_this->stList.destroy_node(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pNewNode
			);
			
			HY_ERROR("Scene list inst failed.\n");
			return NULL;
		}
		pNewNode->stSceneInfo.stOrCond.ucCondListTrue = 0;
		pNewNode->stSceneInfo.stAndCond.ucCondListTrue = 0x3;
		/*数据库更新*/
		iRet = 
			pPrivateMethods->scene_add_db(_this, &stScene);
		if(NoErr != iRet)
		{
			HY_ERROR("DB Scene Info set failed.\n");
			return NULL;
		}
		return &pNewNode->stSceneInfo;
	}
}

static int 
scene_list_del(
	scene_list_class_t *_this,
	char *pcSeneId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcSeneId);

	int iRet = 0;
	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	/*新建结点用于查找*/
	scene_info_t stScene;
	memset(&stScene, 0x0, sizeof(scene_info_t));
	strncpy(stScene.acSceneId, pcSeneId, INDEX_MAX_LEN);
	
	scene_list_node_t *pstGetNode = 
		pPrivateMethods->scene_info_set(
			_this, 
			(scene_list_node_t *)(
				_this->stList.new_node(
					(link_list_class_t *)_this
				)
			),
			&stScene
		);
	/*查找结点*/
	scene_list_node_t *pstDelNode = 
		(scene_list_node_t *)(_this->stList.get(
				(link_list_class_t *)_this, 
				scene_compare, 
				(link_list_piece_t *)pstGetNode
			)
		);
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	
	if(NULL == pstDelNode)
	{
		HY_ERROR("Not found the Scene.\n");
		return NotFoundErr;
	}
	/*删除该场景下所有时间或条件*/
	_this->time_or_cond_clear(
		_this, 
		&pstDelNode->stSceneInfo,
		1
	);

	destroy_cond_list(
		pstDelNode->stSceneInfo.stOrCond.pstTimeCondList
	);
	
	/*删除该场景下所有事件或条件*/
	_this->event_or_cond_clear(
		_this, 
		&pstDelNode->stSceneInfo,
		1
	);
	
	destroy_cond_list(
		pstDelNode->stSceneInfo.stOrCond.pstEventCondList
	);
	
	/*删除该场景下所有时间与条件*/
	_this->time_and_cond_clear(
		_this,
		&pstDelNode->stSceneInfo,
		1
	);
	
	destroy_cond_list(
		pstDelNode->stSceneInfo.stAndCond.pstTimeCondList
	);
	
	/*删除该场景下所有事件与条件*/
	_this->event_and_cond_clear(
		_this,
		&pstDelNode->stSceneInfo,
		1
	);
	
	destroy_cond_list(
		pstDelNode->stSceneInfo.stAndCond.pstEventCondList
	);
	
	/*删除该场景下所有动作*/
	_this->action_clear(
		_this,
		&pstDelNode->stSceneInfo,
		1
	);
	destroy_action_list(pstDelNode->stSceneInfo.pstAction);
	
	/*删除该场景下绑定的所有场景面板*/
	_this->scene_bind_clear(
		_this,
		&pstDelNode->stSceneInfo,
		1
	);
	
	
	

	/*删除节点*/
	iRet = 
		_this->stList.del(
			(link_list_class_t *)_this, 
			(link_list_piece_t *)pstDelNode
		);
	
	if(NoErr != iRet)
	{
		HY_ERROR("Node deletion failed.\n");
		return SceneDelErr;
	}
	/*删除数据库*/
	
	iRet = 
		pPrivateMethods->scene_del_db(_this, &stScene);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Scene Info del failed.\n");
		return SceneDelErr;
	}
	
	return NoErr;
}

static int 
scene_list_set(
	scene_list_class_t *_this,
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);

	int iRet = 0;
	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	/*新建结点用于查找*/
	scene_info_t stScene;
	memset(&stScene, 0x0, sizeof(scene_info_t));
	strncpy(stScene.acSceneId, 
		pstInfo->acSceneId, INDEX_MAX_LEN);
	strncpy(stScene.acSceneName,
		pstInfo->acSceneName, NAME_MAX_LEN);
	strncpy(stScene.acSceneEnable,
		pstInfo->acSceneEnable, STATE_MAX_LEN);
	strncpy(stScene.acExecDelayed,
		pstInfo->acExecDelayed, DELAYED_MAX_LEN);
	strncpy(stScene.acUpdateTime,
		pstInfo->acUpdateTime, TIME_MAX_LEN);
	strncpy(stScene.acNote,
		pstInfo->acNote, NOTE_MAX_LEN);
	scene_list_node_t *pstGetNode = 
		pPrivateMethods->scene_info_set(
			_this, 
			(scene_list_node_t *)(
				_this->stList.new_node(
					(link_list_class_t *)_this
				)
			),
			&stScene
		);
	/*设置结点*/
	scene_list_node_t *pNode = 
		(scene_list_node_t *)(_this->stList.get(
				(link_list_class_t *)_this,
				scene_compare,
				(link_list_piece_t *)pstGetNode
				)
			);
	if(NULL != pNode)
	{
		strncpy(
			pNode->stSceneInfo.acSceneName,
			pstInfo->acSceneName,
			NAME_MAX_LEN
		);
		strncpy(
			pNode->stSceneInfo.acSceneEnable, 
			pstInfo->acSceneEnable, 
			STATE_MAX_LEN
		);
		strncpy(
			pNode->stSceneInfo.acExecDelayed, 
			pstInfo->acExecDelayed, 
			DELAYED_MAX_LEN
		);
		strncpy(
			pNode->stSceneInfo.acUpdateTime, 
			pstInfo->acUpdateTime, 
			TIME_MAX_LEN
		);
		strncpy(
			pNode->stSceneInfo.acNote, 
			pstInfo->acNote, 
			NOTE_MAX_LEN
		);
	}
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	if(NULL == pNode)
	{
		HY_ERROR("Not found the Scene.\n");
		return NotFoundErr;
	}

	/*数据库更新*/
	iRet = 
		pPrivateMethods->scene_set_db(_this, &stScene);
	if(NoErr != iRet)
	{
		HY_ERROR("DB Scene Info set failed.\n");
		return SceneSetErr;
	}
	
	return NoErr;
}

static scene_info_t* 
scene_list_get(
	scene_list_class_t *_this,
	char *pcSeneId
)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pcSeneId);

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	/*新建结点用于查找*/
	scene_info_t stScene;
	memset(&stScene, 0x0, sizeof(scene_info_t));
	strncpy(stScene.acSceneId, pcSeneId, INDEX_MAX_LEN);
	
	scene_list_node_t *pstGetNode = 
		pPrivateMethods->scene_info_set(
			_this, 
			(scene_list_node_t *)(
				_this->stList.new_node(
					(link_list_class_t *)_this
				)
			),
			&stScene
		);
	/*获取*/
	scene_list_node_t *pstNode = 
		(scene_list_node_t *)(_this->stList.get(
			(link_list_class_t *)_this, 
			scene_compare, 
			(link_list_piece_t *)pstGetNode)
		);
	
	/*释放新创建的结点*/
	_this->stList.destroy_node(
		(link_list_class_t *)_this, 
		(link_list_piece_t *)pstGetNode
	);
	
	if(NULL == pstNode)
	{
		HY_ERROR("Not found the Scene.\n");
		return NULL;
	}
	return &pstNode->stSceneInfo;
	
}

static int 
scene_list_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pastInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pastInfo, piCount);

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	int iCount = 0;
	int iDevMaxNum = *piCount;
	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		*piCount = 0;
		return NoErr;
	}
	
	
	scene_list_node_t *pstPtr = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstPtr)
	{
		*piCount = 0;
		HY_ERROR("Get Fist Node Error.\n");
		return SceneGetErr;
	}
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);
	if(NULL == pstHead)
	{
		*piCount = 0;
		HY_ERROR("Get Head Node Error.\n");
		return SceneGetErr;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		if(iCount < iDevMaxNum)
		{
			pPrivateMethods->scene_info_get(
				_this,
				pstPtr,
				&pastInfo[iCount++]
			);
		}
	
		pstPtr = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	
	*piCount = iCount;

	return NoErr;
}

/*初始化场景绑定*/
static int 
scene_bind_init(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(
		_this, pstCurrScene);

	scene_list_private_methods_t *pPrivateMethods = 
			(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;

	int iRet = 0;
	int iCount = SCENE_BIND_MAX_NUM;
	scene_panel_t *astInfo = (scene_panel_t *)calloc(iCount, sizeof(scene_panel_t));
	if(NULL == astInfo)
	{
		HY_ERROR("calloc error.\n");
		return HeapReqErr;
	}
		
	/*从数据库中获取条件列表*/
	pPrivateMethods->scene_bind_get_list_db(
		_this,
		pstCurrScene,
		astInfo, 
		&iCount
	);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("DevId = %s\n", 
			astInfo[i].acDevId);
		HY_DEBUG("Key = %s\n", 
			astInfo[i].acKey);
		HY_DEBUG("Value = %s\n", 
			astInfo[i].acValue);
		HY_DEBUG("Enable = %s\n", 
			astInfo[i].acEnable);
	}

	iRet = _this->scene_bind_sync(
			_this, 
			pstCurrScene,
			astInfo,
			iCount
		);

	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}
	return iRet;
}

/*同步场景绑定*/
static int 
scene_bind_sync(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	scene_panel_t *pastPanelInfo, 
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, pstCurrScene, pastPanelInfo);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(
		iCount
	);

	int i = 0;
	
	if(NULL == pstCurrScene->pstPanel)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->scene_bind(
				_this, 
				pstCurrScene, 
				&pastPanelInfo[i]
			);
			HY_INFO("Scene Bind The Panel(%s %s %s)\n",
				pastPanelInfo[i].acDevId,
				pastPanelInfo[i].acKey,
				pastPanelInfo[i].acValue
			);
		}
		return NoErr;
	}
	if(0 == iCount)
	{
		_this->scene_bind_clear(_this, pstCurrScene, 1);
		HY_INFO("Clear Scene Bind\n");
		return NoErr;
	}
	
	char *pacFoundFlag = 
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}

	scene_panel_node_t *p = 
		pstCurrScene->pstPanel;
	while(p)
	{
		scene_panel_t *pstPanel = &p->stPanel;

		int iFlag = 0;
		for(i = 0; i < iCount; ++i)
		{
			if(!strncmp(
				pstPanel->acDevId,
				pastPanelInfo[i].acDevId,
				DEV_ID_MAX_LEN) &&
				!strncmp(
				pstPanel->acKey,
				pastPanelInfo[i].acKey,
				KEY_MAX_LEN) &&
				!strncmp(
				pstPanel->acValue,
				pastPanelInfo[i].acValue,
				VALUE_MAX_LEN)
			)
			{
				/*找到,无需增加*/
				pacFoundFlag[i] = 1;
				iFlag = 1;
				break;
			}
			
		}
		if(!iFlag)
		{
			/*未找到，删除*/
			_this->scene_unbind(
				_this, 
				pstCurrScene,
				&pastPanelInfo[i]
			);
			HY_INFO("Scene Unbind The Panel(%s %s %s)\n",
				pastPanelInfo[i].acDevId,
				pastPanelInfo[i].acKey,
				pastPanelInfo[i].acValue
			);
		}
		p = p->next;
	}
	

	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->scene_bind(
				_this, 
				pstCurrScene,
				&pastPanelInfo[i]
			);
			HY_INFO("Scene Bind The Panel(%s %s %s)\n",
				pastPanelInfo[i].acDevId,
				pastPanelInfo[i].acKey,
				pastPanelInfo[i].acValue
			);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;
}

/*场景绑定实体场景面板*/
static int 
scene_bind(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	scene_panel_t *pstPanelInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, pstCurrScene, pstPanelInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	scene_list_private_methods_t *pPrivateMethods = 
		(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该绑定信息是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, "%s%s%s%s", 
		pstPanelInfo->acDevId, 
		pstPanelInfo->acKey,
		pstPanelInfo->acValue,
		PRT_HASH_TYPE_SCENE_BIND
	);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		scene_panel_node_t *p = 
			pHashValue->stCondPtr.pScenePanelNode;
		if(strncmp(p->stPanel.acEnable, 
			pstPanelInfo->acEnable, 
			STATE_MAX_LEN)
		)
		{
			strncpy(p->stPanel.acEnable, 
				pstPanelInfo->acEnable,
				STATE_MAX_LEN
			);
			/*更新数据库*/
			iRet = 
				pPrivateMethods->scene_bind_set_db(
					_this,
					pstCurrScene,
					pstPanelInfo
				);
			if(NoErr != iRet)
			{
				HY_ERROR("DB SceneBindInfo Add failed.\n");
				return SceneDelErr;
			}
		}
		return NoErr;
	}
	else
	{
		/*添加*/
		pHashValue= 
			(ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == pHashValue)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}

		
		/*生成新结点*/
		scene_panel_node_t *pNew =
			(scene_panel_node_t *)calloc(1, sizeof(scene_panel_node_t));
		memcpy(&pNew->stPanel , pstPanelInfo, sizeof(scene_panel_t));
		/*将结点添加到链表中*/
		scene_panel_node_t *p = pstCurrScene->pstPanel;
		if(NULL == p)
		{
			pstCurrScene->pstPanel = pNew;
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
		}

		/*添加到数据库中*/
		iRet = 
			pPrivateMethods->scene_bind_db(
				_this, 
				pstCurrScene, 
				pstPanelInfo
			);
		if(NoErr != iRet)
		{
			HY_ERROR("DB SceneBindInfo Add failed.\n");
			return SceneDelErr;
		}
		
		strncpy(pHashValue->stCondPtr.acId, 
			pstCurrScene->acSceneId, INDEX_MAX_LEN);
		pHashValue->stCondPtr.pstScene = (void*)pstCurrScene;
		pHashValue->stCondPtr.pScenePanelNode = (void *)pNew;
		
		iRet = 
			pPrivateParam->pHashTable->ptr_hash_inst(
				pPrivateParam->pHashTable,
				acHashKey, 
				pHashValue
			);
	}

	return iRet;
}


/*场景解绑实体场景面板*/
static int 
scene_unbind(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	scene_panel_t *pstPanelInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, pstCurrScene, pstPanelInfo);
	
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	
	scene_list_private_methods_t *pPrivateMethods = 
		(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, "%s%s%s%s", 
		pstPanelInfo->acDevId, 
		pstPanelInfo->acKey,
		pstPanelInfo->acValue,
		PRT_HASH_TYPE_SCENE_BIND
	);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/**/
		scene_panel_node_t *p = 
			(scene_panel_node_t *)(pHashValue->stCondPtr.pScenePanelNode);
		/*删除数据库*/
		iRet = 
			pPrivateMethods->scene_unbind_db(
				_this, 
				pstCurrScene, 
				&p->stPanel
			);
		if(NoErr != iRet)
		{
			HY_ERROR("DB SceneBindInfo del failed.\n");
			return SceneDelErr;
		}
		
		/*删除条件*/
		if(NULL == p->prev)
		{
			/*头结点*/
			pstCurrScene->pstPanel = p->next;
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
			p->next->prev = p->prev;
			p->prev->next = p->next;
			free(p);
		}
		iRet += pPrivateParam->pHashTable->ptr_hash_del(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
		);
	}
	else
	{
		HY_ERROR("Not Found The Panel Info\n");
		return NotFoundErr;
	}
	return iRet;
}

/*获取场景绑定的实体场景面板*/
static int 
scene_bind_get_list(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	scene_panel_t *pastPanelInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this, pstCurrScene, pastPanelInfo, piCount);
	
	int iCount = 0;
	int iDevMaxNum = *piCount;

	scene_panel_node_t *p = pstCurrScene->pstPanel;
	while(p)
	{
		if(iCount < iDevMaxNum)
		{
			memcpy(&pastPanelInfo[iCount++], 
				&p->stPanel, 
				sizeof(scene_panel_t)
			);
		}
		p = p->next;
	}
	
	*piCount = iCount;

	return NoErr;
}

/*清空场景绑定*/
static int 
scene_bind_clear(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene,
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(
		_this, pstCurrScene);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	
	scene_list_private_methods_t *pPrivateMethods = 
		(scene_list_private_methods_t *)_this->acScenelistPrivateMethods;
	
	char acHashKey[KEY_MAX_LEN] = {0};
	/*释放结点*/
	scene_panel_node_t *p = pstCurrScene->pstPanel;
	scene_panel_node_t *p1;
	while(p)
	{
		p1 = p;
		p = p->next;
		/*删除Hash*/
		snprintf(
			acHashKey,
			KEY_MAX_LEN,
			"%s%s%s%s", 
			p1->stPanel.acDevId, 
			p1->stPanel.acKey,
			p1->stPanel.acValue,
			PRT_HASH_TYPE_SCENE_BIND
		);
		pPrivateParam->pHashTable->ptr_hash_del(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
		);
		if(1 == iClearDbFlag)
		{
			/*删除数据库*/
			pPrivateMethods->scene_unbind_db(
				_this, 
				pstCurrScene, 
				&p1->stPanel
			);
		}
		/*释放结点*/
		free(p1);
	}

	return NoErr;
}


/*时间或集合*/
/*初始化条件*/
static int 
scene_time_or_cond_init(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stOrCond.pstTimeCondList;

	int iRet = 0;
	int iCount = COND_MAX_NUM;
	time_cond_t *astInfo = (time_cond_t *)calloc(COND_MAX_NUM, sizeof(time_cond_t));
	if(NULL == astInfo)
	{
		HY_ERROR("Calloc error\n");
		return HeapReqErr;
	}

	/*开始初始化操作*/
	pstCondClass->ucInitFlag = 1;
	
	/*从数据库中获取条件列表*/
	pstCondClass->cond_get_list_db(
		pstCondClass,
		astInfo, 
		&iCount
	);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("TriggerType = %s\n", 
			astInfo[i].acTriggerType);
		HY_DEBUG("TriggerInterval = %s\n", 
			astInfo[i].acTriggerInterval);
		HY_DEBUG("TimeKey = %s\n", 
			astInfo[i].acTimeKey);
		HY_DEBUG("Start = %s:%s\n", 
			astInfo[i].acStartHour, astInfo[i].acStartMinu);
		HY_DEBUG("End = %s:%s\n", 
			astInfo[i].acEndHour, astInfo[i].acEndMinu);
		HY_DEBUG("Week = %s\n", 
			astInfo[i].acWeek);
		HY_DEBUG("Repeat = %s\n",
			astInfo[i].acRepeat);
		HY_DEBUG("acLose = %s\n",
			astInfo[i].acLose);
	}

	iRet = _this->time_or_cond_sync(
			_this, 
			pstCurrScene,
			astInfo,
			iCount
		);

	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}

	/*结束初始化操作*/
	pstCondClass->ucInitFlag = 0;
	
	return iRet;
}
/*同步条件*/
static int 
scene_time_or_cond_sync(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	time_cond_t *pastCondInfo,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, 
		pstCurrScene, 
		pastCondInfo
	);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(
		iCount
	);

	int i = 0;
	cond_list_class_t *pstCondClass =
		pstCurrScene->stOrCond.pstTimeCondList;
	
	if(NoErr ==
		pstCondClass->stList.size(
			(link_list_class_t *)pstCondClass
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->time_or_cond_add(
				_this, 
				pstCurrScene, 
				&pastCondInfo[i]
			);
			HY_INFO("Add Time Or Cond(%s)\n",
				pastCondInfo[i].acTimeKey);
		}
		return NoErr;
	}
	if(NoErr == iCount)
	{
		_this->time_or_cond_clear(_this, pstCurrScene, 1);
		HY_INFO("Clear Time Or Cond\n");
		return NoErr;
	}
	char *pacFoundFlag = 
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}

	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(pstCondClass->stList.next(
			(link_list_class_t *)pstCondClass,
			NULL)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(
			pstCondClass->stList.head(
				(link_list_class_t *)pstCondClass
			)
		);
	if(NULL == pstHead)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	while(pstNode && pstHead != pstNode)
	{
		time_cond_node_t *p = 
			(time_cond_node_t *)(pstNode->stCondSet.pstCond);
		while(p)
		{
			time_cond_t *pstCond = &p->stCond;

			int iFlag = 0;
			for(i = 0; i < iCount; ++i)
			{
				if(!strncmp(
					pstCond->acTimeKey,
					pastCondInfo[i].acTimeKey,
					KEY_MAX_LEN)
				)
				{
					/*找到,判断数据是否发生改变*/
					if(strncmp(
							pstCond->acTriggerType,
							pastCondInfo[i].acTriggerType,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acTriggerInterval,
							pastCondInfo[i].acTriggerInterval,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acStartHour,
							pastCondInfo[i].acStartHour,
							HOUR_MAX_LEN
						) || 
						strncmp(
							pstCond->acStartMinu, 
							pastCondInfo[i].acStartMinu,
							MINU_MAX_LEN
						) ||
						strncmp(
							pstCond->acEndHour, 
							pastCondInfo[i].acEndHour,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acEndMinu,
							pastCondInfo[i].acEndMinu,
							MINU_MAX_LEN
						) ||
						strncmp(
							pstCond->acWeek,
							pastCondInfo[i].acWeek,
							WEEK_MAX_LEN
						) ||
						strncmp(
							pstCond->acRepeat, 
							pastCondInfo[i].acRepeat,
							STATE_MAX_LEN
						) ||
						strncmp(
							pstCond->acLose, 
							pastCondInfo[i].acLose,
							STATE_MAX_LEN
						) 
					)
					{
						/*信息发生改变*/
						_this->time_or_cond_add(
							_this, 
							pstCurrScene, 
							&pastCondInfo[i]
						);
						HY_INFO("Set Time Or Cond(%s)\n",
							pastCondInfo[i].acTimeKey);
					}
					pacFoundFlag[i] = 1;
					iFlag = 1;
					break;
				}
				
			}
			if(!iFlag)
			{
				/*未找到，删除*/
				_this->time_or_cond_del(
					_this, 
					pstCurrScene,
					&pastCondInfo[i]
				);
				HY_INFO("Del Time Or Cond(%s)\n",
					pastCondInfo[i].acTimeKey);
			}
			p = p->next;
		}
	
		pstNode = 
			(cond_list_node_t *)(
				pstCondClass->stList.next(
					(link_list_class_t *)pstCondClass, 
					(link_list_piece_t *)pstNode
				)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->time_or_cond_add(
				_this, 
				pstCurrScene,
				&pastCondInfo[i]
			);
			HY_INFO("Add Time Or Cond(%s)\n", 
				pastCondInfo[i].acTimeKey);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;

}

/*添加条件*/
static int 
scene_time_or_cond_add(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, "%s%s", pstCondInfo->acTimeKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		iRet =
			pstCurrScene->stOrCond.pstTimeCondList->cond_set(
				pstCurrScene->stOrCond.pstTimeCondList, 
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
	}
	else
	{
		/*添加*/
		pHashValue= 
			(ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == pHashValue)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		iRet =
			pstCurrScene->stOrCond.pstTimeCondList->cond_add(
				pstCurrScene->stOrCond.pstTimeCondList,
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
		strncpy(pHashValue->stCondPtr.acId, 
			pstCurrScene->acSceneId, INDEX_MAX_LEN);
		pHashValue->stCondPtr.pstScene = pstCurrScene;
		
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_inst(
				pPrivateParam->pHashTable,
				acHashKey, 
				pHashValue
			);
	}

	if(NoErr == iRet)
	{
		/*添加、修改成功，更新真假状态位。新添加或修改的条件初始认为都为假*/
		scene_time_cond_true_init(_this, 0, pHashValue);
	}
	
	return iRet;
}

/*删除条件*/
static int 
scene_time_or_cond_del(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN,
		"%s%s", pstCondInfo->acTimeKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		iRet = pstCurrScene->stOrCond.pstTimeCondList->cond_del(
			pstCurrScene->stOrCond.pstTimeCondList, 
			&pHashValue->stCondPtr
		);
		if(NoErr == iRet)
		{
			if(0 == 
				pstCurrScene->stOrCond.pstTimeCondList->cond_size(
					pstCurrScene->stOrCond.pstTimeCondList
				)
			)
			{
				/*当条件列表由非空变为空，则将条件集合真假标志位置假，条件列表真假标志位置假*/
				pstCurrScene->stOrCond.pstTimeCondList->uiCondSetTrue = 0;
				pstCurrScene->stOrCond.ucCondListTrue &= ~(0x1 << 0);
			}
			else
			{
				/*删除成功，更新真假状态位。*/
				cond_list_node_t *pstCondSetNode = 
					(cond_list_node_t *)(pHashValue->stCondPtr.pCondSetNode);

				cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
				int iTrue = 
					pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
				scene_time_cond_set_true_init(iTrue, pHashValue);
			}
			
		}
		iRet += pPrivateParam->pHashTable->ptr_hash_del(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
		);
	}
	else
	{
		HY_ERROR("Not Found The Cond\n");
		return NotFoundErr;
	}
	return iRet;
}

/*获取所有条件*/
static int 
scene_time_or_cond_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this,
		pstCurrScene, 
		pastCondInfo,
		piCount
	);
	return 
		pstCurrScene->stOrCond.pstTimeCondList->cond_get_list(
				pstCurrScene->stOrCond.pstTimeCondList, 
				(void*)pastCondInfo, 
				piCount
			);
}
/*删除所有条件*/
static int 
scene_time_or_cond_clear(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrScene);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	
	char acHashKey[KEY_MAX_LEN] = {0};
	
	cond_list_class_t *pstCondClass =
		pstCurrScene->stOrCond.pstTimeCondList;
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(pstCondClass->stList.head(
			(link_list_class_t *)pstCondClass
			)
		);
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(pstCondClass->stList.first(
			(link_list_class_t *)pstCondClass)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		time_cond_node_t *p = 
			pstNode->stCondSet.pstCond;
		time_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*删除Hash*/
			snprintf(acHashKey, KEY_MAX_LEN, 
				"%s%s", p1->stCond.acTimeKey, PRT_HASH_TYPE_COND);
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
			if(1 == iClearDbFlag)
			{
				/*删除数据库*/
				pstCondClass->cond_del_db(pstCondClass, &p1->stCond);
			}
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		pstNode = 
			(cond_list_node_t *)(pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	pstCondClass->iCount = 0;
	pstCondClass->uiCondSetTrue = 0;
	
	pstCondClass->stList.clear(
		(link_list_class_t *)pstCondClass
	);
	
	return 0;
}






/*事件或集合*/
/*初始化条件*/
static int 
scene_event_or_cond_init(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stOrCond.pstEventCondList;
	int iRet = 0;
	int iCount = COND_MAX_NUM;
	event_cond_t *astInfo = (event_cond_t *)calloc(COND_MAX_NUM, sizeof(event_cond_t));
	if(NULL == astInfo)
	{
		HY_ERROR("Calloc error.");
		return HeapReqErr;
	}

	/*开始初始化操作*/
	pstCondClass->ucInitFlag = 1;
	
	pstCondClass->cond_get_list_db(
		pstCondClass,
		astInfo,
		&iCount
	);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("TriggerType = %s\n", astInfo[i].acTriggerType);
		HY_DEBUG("ContinueTime = %s\n", astInfo[i].acContinueTime);
		HY_DEBUG("DevId = %s\n", astInfo[i].acDevId);
		HY_DEBUG("Key = %s\n", astInfo[i].acKey);
		HY_DEBUG("Value = %s\n", astInfo[i].acValue);
		HY_DEBUG("Active = %s\n", astInfo[i].acActive);
		HY_DEBUG("acLastValue = %s\n", astInfo[i].acLastValue);
	}

	iRet = _this->event_or_cond_sync(
		_this, 
		pstCurrScene,
		astInfo, 
		iCount
	);

	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}

	/*结束初始化操作*/
	pstCondClass->ucInitFlag = 0;
	
	return iRet;

}
/*同步条件*/
static int 
scene_event_or_cond_sync(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pastCondInfo,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, 
		pstCurrScene,
		pastCondInfo
	);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(
		iCount
	);

	int i = 0;
	cond_list_class_t *pstCondClass =
		pstCurrScene->stOrCond.pstEventCondList;
	
	if(NoErr == 
		pstCondClass->stList.size(
			(link_list_class_t *)pstCondClass
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->event_or_cond_add(
				_this,
				pstCurrScene, 
				&pastCondInfo[i]
			);
			HY_INFO("Add Event Or Cond(%s %s)\n",
				pastCondInfo[i].acDevId, pastCondInfo[i].acKey);
		}
		return NoErr;
	}
	if(NoErr == iCount)
	{
		_this->event_or_cond_clear(_this, pstCurrScene, 1);
		HY_INFO("Clear Event Or Cond\n");
		return NoErr;
	}
	char *pacFoundFlag = 
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return HeapReqErr;
	}

	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(
			pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass,
				NULL
			)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		HY_ERROR("Get first Node Error\n");
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(
			pstCondClass->stList.head(
				(link_list_class_t *)pstCondClass
			)
		);
	if(NULL == pstHead)
	{
		free(pacFoundFlag);
		HY_ERROR("Get head node error\n");
		return SceneGetErr;
	}
	while(pstNode && pstHead != pstNode)
	{
		event_cond_node_t *p = 
			(event_cond_node_t *)(pstNode->stCondSet.pstCond);
		while(p)
		{
			event_cond_t *pstCond = &p->stCond;

			int iFlag = 0;
			for(i = 0; i < iCount; ++i)
			{
				if(!strncmp(
						pstCond->acDevId,
						pastCondInfo[i].acDevId,
						DEV_ID_MAX_LEN
					) &&
					!strncmp(
						pstCond->acKey,
						pastCondInfo[i].acKey,
						KEY_MAX_LEN
					)
				)
				{
					/*找到,判断数据是否发生改变*/
					if(strncmp(
							pstCond->acTriggerType,
							pastCondInfo[i].acTriggerType, 
							VALUE_MAX_LEN
						) ||
						strncmp(
							pstCond->acContinueTime,
							pastCondInfo[i].acContinueTime, 
							VALUE_MAX_LEN
						) ||
						strncmp(
							pstCond->acValue,
							pastCondInfo[i].acValue, 
							VALUE_MAX_LEN
						) || 
						strncmp(
							pstCond->acActive, 
							pastCondInfo[i].acActive,
							ACTIVE_MAX_LEN
						) || 
						strncmp(
							pstCond->acLastValue, 
							pastCondInfo[i].acLastValue,
							ACTIVE_MAX_LEN
						)
					)
					{
						/*信息发生改变*/
						_this->event_or_cond_add(
							_this,
							pstCurrScene, 
							&pastCondInfo[i]
						);
						HY_INFO("Set Event Or Cond(%s %s)\n",
							pastCondInfo[i].acDevId, pastCondInfo[i].acKey);
					}
					pacFoundFlag[i] = 1;
					iFlag = 1;
					break;
				}
				
			}
			if(!iFlag)
			{
				/*未找到，删除*/
				_this->event_or_cond_del(
					_this, 
					pstCurrScene,
					&pastCondInfo[i]
				);
				HY_INFO("Del Event Or Cond(%s %s)\n",
					pastCondInfo[i].acDevId,
					pastCondInfo[i].acKey
				);
			}
			p = p->next;
		}
	
		pstNode = 
			(cond_list_node_t *)(pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass, 
				(link_list_piece_t *)pstNode)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		/*对手动触发条件进行特殊处理*/
		if(!strncmp(pastCondInfo[i].acKey, JSON_VALUE_KEY_MANUAL, KEY_MAX_LEN))
		{
			pacFoundFlag[i] = pstCurrScene->cManualTriggerFlag;
		}
		
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->event_or_cond_add(
				_this, 
				pstCurrScene,
				&pastCondInfo[i]
			);
			HY_INFO("Add Event Or Cond(%s %s)\n",
				pastCondInfo[i].acDevId, 
				pastCondInfo[i].acKey
			);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;

}

/*获取条件*/
static int 
scene_event_or_cond_get(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	char acHashKey[KEY_MAX_LEN] = {0};
		
	/*判断该条件是否存在Hash表中*/
	snprintf(
		acHashKey, 
		KEY_MAX_LEN, 
		"%s%s%s", 
		pstCondInfo->acDevId, 
		pstCondInfo->acKey,
		PRT_HASH_TYPE_COND
	);
	
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		ptr_info_t* pstPtrInfo = &(pHashValue->stCondPtr);
		event_cond_node_t *pstCondNode = (event_cond_node_t*)(pstPtrInfo->pCondNode);
		event_cond_t *pstCond = &pstCondNode->stCond;

		memcpy(pstCondInfo, pstCond, sizeof(event_cond_t));
		return 0;
	}
	else
	{
		HY_ERROR("Cond Not Found.\n");
		return -1;
	}

}

/*添加条件*/
static int 
scene_event_or_cond_add(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};

	if(!strncmp(pstCondInfo->acKey, JSON_VALUE_KEY_MANUAL, KEY_MAX_LEN))
	{
		/*手动触发条件*/
		/*手动触发条件，不存hash，只存内存与数据库*/
		if(0 == pstCurrScene->cManualTriggerFlag)
		{
		
			/*添加*/
			cond_list_class_t *pstCondList = pstCurrScene->stOrCond.pstEventCondList;
			
			iRet = pstCondList->cond_add_db(pstCondList, pstCondInfo);
			if(NoErr != iRet)
			{
				HY_ERROR("Failed to add manual trigger condition.\n");
				return iRet;
			}

			pstCurrScene->cManualTriggerFlag = 1;

			
			HY_INFO("Add manual activation conditions for the scene(%s)\n", pstCurrScene->acSceneId);
		}
		
		
		return NoErr;
	}
		
	/*判断该条件是否存在Hash表中*/
	snprintf(
		acHashKey, 
		KEY_MAX_LEN, 
		"%s%s%s", 
		pstCondInfo->acDevId, 
		pstCondInfo->acKey,
		PRT_HASH_TYPE_COND
	);
	
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		iRet = 
			pstCurrScene->stOrCond.pstEventCondList->cond_set(
				pstCurrScene->stOrCond.pstEventCondList, 
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
	}
	else
	{
		/*添加*/
		pHashValue = 
			(ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == pHashValue)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		iRet =
			pstCurrScene->stOrCond.pstEventCondList->cond_add(
				pstCurrScene->stOrCond.pstEventCondList,
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
		strncpy(pHashValue->stCondPtr.acId,
			pstCurrScene->acSceneId, INDEX_MAX_LEN);
		pHashValue->stCondPtr.pstScene = pstCurrScene;
		
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_inst(
				pPrivateParam->pHashTable,
				acHashKey, 
				pHashValue
			);
		//pPrivateParam->pHashTable->ptr_hash_print(
		//	pPrivateParam->pHashTable
		//);
	}

	if(NoErr == iRet)
	{
		/*添加、修改成功，更新初始的真假状态位*/
		int iTrue = 0;
		if(0 == strlen(pstCondInfo->acLastValue))
		{
			iTrue = 0;
		}
		else
		{
			event_t stEventInfo;
			strncpy(stEventInfo.acDevId, pstCondInfo->acDevId, DEV_ID_MAX_LEN);
			strncpy(stEventInfo.acKey, pstCondInfo->acKey, KEY_MAX_LEN);
			strncpy(stEventInfo.acValue, pstCondInfo->acLastValue, VALUE_MAX_LEN);
			iTrue = _scene_event_cond_is_true(pstCondInfo, &stEventInfo);
		}
		scene_event_cond_true_init(_this, iTrue, pHashValue);
	}
	
	return iRet;
}

/*删除条件*/
static int 
scene_event_or_cond_del(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	
	if(!strncmp(pstCondInfo->acKey, JSON_VALUE_KEY_MANUAL, KEY_MAX_LEN))
	{
		/*手动触发条件*/
		/*删除手动触发条件，删除内存与数据库*/
		if(1 == pstCurrScene->cManualTriggerFlag)
		{
			/*删除*/
			cond_list_class_t *pstCondList = pstCurrScene->stOrCond.pstEventCondList;
			iRet = pstCondList->cond_del_db(pstCondList, pstCondInfo);
			if(NoErr != iRet)
			{
				HY_ERROR("Failed to del manual trigger condition.\n");
				return iRet;
			}

			pstCurrScene->cManualTriggerFlag = 0;
		}
		

		return NoErr;
	}
	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN,
		"%s%s%s", pstCondInfo->acDevId, pstCondInfo->acKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		iRet = 
			pstCurrScene->stOrCond.pstEventCondList->cond_del(
				pstCurrScene->stOrCond.pstEventCondList, 
				&pHashValue->stCondPtr
			);
		if(NoErr == iRet)
		{
			if(0 == 
				pstCurrScene->stOrCond.pstEventCondList->cond_size(
					pstCurrScene->stOrCond.pstEventCondList
				)
			)
			{
				/*当条件列表由非空变为空，则将条件集合真假标志位置假，条件列表真假标志位置假*/
				pstCurrScene->stOrCond.pstEventCondList->uiCondSetTrue = 0;
				pstCurrScene->stOrCond.ucCondListTrue &= ~(0x1 << 1);
			}
			else
			{
				/*删除成功，更新真假状态位。*/
				cond_list_node_t *pstCondSetNode = 
					(cond_list_node_t *)(pHashValue->stCondPtr.pCondSetNode);

				cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
				int iTrue = 
					pCondSet->uiCondTrue & (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount) ? 1 : 0;
				scene_event_cond_set_true_init(iTrue, pHashValue);
			}
		}
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
	}
	else
	{
		HY_ERROR("Not Found The Cond\n");
		return NotFoundErr;
	}
	return NoErr;

}

/*获取所有条件*/
static int 
scene_event_or_cond_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	event_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this, 
		pstCurrScene,
		pastCondInfo,
		piCount
	);
	return 
		pstCurrScene->stOrCond.pstEventCondList->cond_get_list(
				pstCurrScene->stOrCond.pstEventCondList, 
				(void*)pastCondInfo,
				piCount
			);
}
/*删除所有条件*/
static int 
scene_event_or_cond_clear(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrScene);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
		
	char acHashKey[KEY_MAX_LEN] = {0};
	
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stOrCond.pstEventCondList;
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(
			pstCondClass->stList.head(
				(link_list_class_t *)pstCondClass
			)
		);
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(pstCondClass->stList.first(
			(link_list_class_t *)pstCondClass)
		);
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		event_cond_node_t *p = 
			pstNode->stCondSet.pstCond;
		event_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*删除Hash*/
			snprintf(
				acHashKey, 
				KEY_MAX_LEN,
				"%s%s%s", 
				p1->stCond.acDevId, 
				p1->stCond.acKey,
				PRT_HASH_TYPE_COND
			);
			if(1 == iClearDbFlag)
			{
				/*删除数据库*/
				pstCondClass->cond_del_db(pstCondClass, &p1->stCond);
			}
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		pstNode = 
			(cond_list_node_t *)(pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass, 
				(link_list_piece_t *)pstNode)
			);
	}
	pstCondClass->iCount = 0;
	pstCondClass->uiCondSetTrue = 0;
	
	/*删除数据库中手动条件*/
	if(1 == pstCurrScene->cManualTriggerFlag && 1 == iClearDbFlag)
	{
		event_cond_t stCondInfo = {0};
	
		strcpy(stCondInfo.acDevId, "0000000000000000");
		strcpy(stCondInfo.acKey, "Manual");
		pstCondClass->cond_del_db(pstCondClass, (void *)&stCondInfo);
	}
	
	return 
		pstCondClass->stList.clear(
			(link_list_class_t *)pstCondClass
		);
}







/*时间与集合*/
/*初始化条件*/
static int 
scene_time_and_cond_init(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stAndCond.pstTimeCondList;
	int iRet = 0;
	int iCount = COND_MAX_NUM;
	time_cond_t *astInfo = (time_cond_t *)calloc(COND_MAX_NUM, sizeof(time_cond_t));
	if(NULL == astInfo)
	{
		HY_ERROR("Calloc error\n");
		return HeapReqErr;
	}

	/*开始初始化操作*/
	pstCondClass->ucInitFlag = 1;
	
	pstCondClass->cond_get_list_db(
		pstCondClass,
		astInfo,
		&iCount
	);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("TriggerType = %s\n",
			astInfo[i].acTriggerType);
		HY_DEBUG("TriggerInterval = %s\n",
			astInfo[i].acTriggerInterval);
		HY_DEBUG("TimeKey = %s\n",
			astInfo[i].acTimeKey);
		HY_DEBUG("Start = %s:%s\n", 
			astInfo[i].acStartHour, astInfo[i].acStartMinu);
		HY_DEBUG("End = %s:%s\n", 
			astInfo[i].acEndHour, astInfo[i].acEndMinu);
		HY_DEBUG("Week = %s\n", 
			astInfo[i].acWeek);
		HY_DEBUG("Repeat = %s\n", 
			astInfo[i].acRepeat);
		HY_DEBUG("acLose = %s\n", 
			astInfo[i].acLose);
	}
	iRet = _this->time_and_cond_sync(
			_this, 
			pstCurrScene, 
			astInfo, 
			iCount
		);
	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}

	/*结束初始化操作*/
	pstCondClass->ucInitFlag = 0;
	
	return iRet;

}
/*同步条件*/
static int 
scene_time_and_cond_sync(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	time_cond_t *pastCondInfo,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this,
		pstCurrScene, 
		pastCondInfo
	);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(
		iCount
	);

	int i = 0;
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stAndCond.pstTimeCondList;
	
	if(NoErr == 
		pstCondClass->stList.size(
			(link_list_class_t *)pstCondClass
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->time_and_cond_add(
				_this, 
				pstCurrScene,
				&pastCondInfo[i]
			);
			HY_INFO("Add Time And Cond(%s)\n", 
				pastCondInfo[i].acTimeKey);
		}
		return NoErr;
	}
	if(NoErr == iCount)
	{
		_this->time_and_cond_clear(
			_this, 
			pstCurrScene,
			1
		);
		HY_INFO("Clear Time And Cond\n");
		return NoErr;
	}
	char *pacFoundFlag =
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}

	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(
			pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass,
				NULL
			)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(
			pstCondClass->stList.head(
				(link_list_class_t *)pstCondClass
			)
		);
	if(NULL == pstHead)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	while(pstNode && pstHead != pstNode)
	{
		time_cond_node_t *p =
			(time_cond_node_t *)(
				pstNode->stCondSet.pstCond
			);
		while(p)
		{
			time_cond_t *pstCond = &p->stCond;

			int iFlag = 0;
			for(i = 0; i < iCount; ++i)
			{
				if(!strncmp(
						pstCond->acTimeKey, 
						pastCondInfo[i].acTimeKey,
						KEY_MAX_LEN
					)
				)
				{
					/*找到,判断数据是否发生改变*/
					if(strncmp(
							pstCond->acTriggerType,
							pastCondInfo[i].acTriggerType,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acTriggerInterval,
							pastCondInfo[i].acTriggerInterval,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acStartHour,
							pastCondInfo[i].acStartHour,
							HOUR_MAX_LEN
						) || 
						strncmp(
							pstCond->acStartMinu,
							pastCondInfo[i].acStartMinu,
							MINU_MAX_LEN
						) ||
						strncmp(
							pstCond->acEndHour,
							pastCondInfo[i].acEndHour,
							HOUR_MAX_LEN
						) ||
						strncmp(
							pstCond->acEndMinu,
							pastCondInfo[i].acEndMinu,
							MINU_MAX_LEN
						) ||
						strncmp(
							pstCond->acWeek,
							pastCondInfo[i].acWeek, 
							WEEK_MAX_LEN
						) ||
						strncmp(
							pstCond->acRepeat,
							pastCondInfo[i].acRepeat,
							STATE_MAX_LEN
						) ||
						strncmp(
							pstCond->acLose,
							pastCondInfo[i].acLose,
							STATE_MAX_LEN
						) 
					)
					{
						/*信息发生改变*/
						_this->time_and_cond_add(
							_this, 
							pstCurrScene,
							&pastCondInfo[i]
						);
						HY_INFO("Set Time And Cond(%s)\n", 
							pastCondInfo[i].acTimeKey);
					}
					pacFoundFlag[i] = 1;
					iFlag = 1;
					break;
				}
				
			}
			if(!iFlag)
			{
				/*未找到，删除*/
				_this->time_and_cond_del(
					_this, 
					pstCurrScene, 
					&pastCondInfo[i]
				);
				HY_INFO("Del Time And Cond(%s)\n",
					pastCondInfo[i].acTimeKey);
			}
			p = p->next;
		}
	
		pstNode = 
			(cond_list_node_t *)(
				pstCondClass->stList.next(
					(link_list_class_t *)pstCondClass, 
					(link_list_piece_t *)pstNode
				)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->time_and_cond_add(
				_this, 
				pstCurrScene, 
				&pastCondInfo[i]
			);
			HY_INFO("Add Time And Cond(%s)\n",
				pastCondInfo[i].acTimeKey);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;

}

/*添加条件*/
static int 
scene_time_and_cond_add(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, 
		"%s%s", pstCondInfo->acTimeKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		iRet = 
			pstCurrScene->stAndCond.pstTimeCondList->cond_set(
				pstCurrScene->stAndCond.pstTimeCondList, 
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
	}
	else
	{
		/*添加*/
		pHashValue = 
			(ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == pHashValue)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		iRet = 
			pstCurrScene->stAndCond.pstTimeCondList->cond_add(
				pstCurrScene->stAndCond.pstTimeCondList,
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
		strncpy(
			pHashValue->stCondPtr.acId,
			pstCurrScene->acSceneId,
			INDEX_MAX_LEN
		);
		pHashValue->stCondPtr.pstScene = pstCurrScene;
		
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_inst(
				pPrivateParam->pHashTable,
				acHashKey, 
				pHashValue
			);
	}
	
	if(NoErr == iRet)
	{
		/*添加、修改成功，更新真假状态位。新添加或修改的条件初始认为都为假*/
		scene_time_cond_true_init(_this, 0, pHashValue);
	}
	
	return iRet;
}

/*删除条件*/
static int 
scene_time_and_cond_del(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN,
		"%s%s", pstCondInfo->acTimeKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		iRet = 
			pstCurrScene->stAndCond.pstTimeCondList->cond_del(
				pstCurrScene->stAndCond.pstTimeCondList, 
				&pHashValue->stCondPtr
			);
		if(NoErr == iRet)
		{
			if(0 == 
				pstCurrScene->stAndCond.pstTimeCondList->cond_size(
					pstCurrScene->stAndCond.pstTimeCondList
				)
			)
			{
				/*当条件列表由非空变为空，则将条件集合真假标志位置假，条件列表真假标志位置真*/
				pstCurrScene->stAndCond.pstTimeCondList->uiCondSetTrue = 0;
				pstCurrScene->stAndCond.ucCondListTrue |= 0x1 << 0;
			}
			else
			{
				/*删除成功，更新真假状态位。*/
				cond_list_node_t *pstCondSetNode = 
					(cond_list_node_t *)(pHashValue->stCondPtr.pCondSetNode);

				cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
				unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
				int iTrue = 
					(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
				scene_time_cond_set_true_init(iTrue, pHashValue);
			}
		}
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
	}
	else
	{
		HY_ERROR("Not Found The Cond\n");
		return NotFoundErr;
	}
	return iRet;
}

/*获取所有条件*/
static int 
scene_time_and_cond_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	time_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this,
		pstCurrScene,
		pastCondInfo,
		piCount
	);
	return 
		pstCurrScene->stAndCond.pstTimeCondList->cond_get_list(
				pstCurrScene->stAndCond.pstTimeCondList, 
				(void*)pastCondInfo,
				piCount
			);
}
/*删除所有条件*/
static int 
scene_time_and_cond_clear(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrScene);
		scene_list_private_param_t *pPrivateParam = 
			(scene_list_private_param_t *)_this->acScenelistPrivateParam;
		
	char acHashKey[KEY_MAX_LEN] = {0};
	
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stAndCond.pstTimeCondList;
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(pstCondClass->stList.head(
			(link_list_class_t *)pstCondClass
			)
		);
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(pstCondClass->stList.first(
			(link_list_class_t *)pstCondClass)
		);
	
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		time_cond_node_t *p = 
			pstNode->stCondSet.pstCond;
		time_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*删除Hash*/
			snprintf(acHashKey, KEY_MAX_LEN, 
				"%s%s", p1->stCond.acTimeKey, PRT_HASH_TYPE_COND);
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
			if(1 == iClearDbFlag)
			{
				/*删除数据库*/
				pstCondClass->cond_del_db(pstCondClass, &p1->stCond);
			}
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		pstNode = 
			(cond_list_node_t *)(
				pstCondClass->stList.next(
					(link_list_class_t *)pstCondClass, 
					(link_list_piece_t *)pstNode
				)
			);
	}
	pstCondClass->iCount = 0;
	pstCondClass->uiCondSetTrue = 0;
	
	return 
		pstCondClass->stList.clear(
			(link_list_class_t *)pstCondClass
		);
}









/*事件与集合*/
/*初始化条件*/
static int 
scene_event_and_cond_init(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stAndCond.pstEventCondList;
	int iRet = 0;
	int iCount = COND_MAX_NUM;
	event_cond_t *astInfo = (event_cond_t *)calloc(COND_MAX_NUM, sizeof(event_cond_t));
	if(NULL == astInfo)
	{
		HY_ERROR("Calloc error\n");
		return HeapReqErr;
	}

	/*开始初始化操作*/
	pstCondClass->ucInitFlag = 1;
	
	pstCondClass->cond_get_list_db(
		pstCondClass, 
		astInfo,
		&iCount
	);

	int i = 0;
	HY_DEBUG("iCount = %d\n", iCount);
	for(i = 0; i < iCount; ++i)
	{
		HY_DEBUG("TriggerType = %s\n", astInfo[i].acTriggerType);
		HY_DEBUG("ContinueTime = %s\n", astInfo[i].acContinueTime);
		HY_DEBUG("DevId = %s\n", astInfo[i].acDevId);
		HY_DEBUG("Key = %s\n", astInfo[i].acKey);
		HY_DEBUG("Value = %s\n", astInfo[i].acValue);
		HY_DEBUG("Active = %s\n", astInfo[i].acActive);
		HY_DEBUG("LastValue = %s\n", astInfo[i].acLastValue);
	}

	iRet = _this->event_and_cond_sync(
			_this,
			pstCurrScene, 
			astInfo, 
			iCount
		);

	if(astInfo)
	{
		free(astInfo);
		astInfo = NULL;
	}

	/*结束初始化操作*/
	pstCondClass->ucInitFlag = 0;
		
	return iRet;

}
/*同步条件*/
static int 
scene_event_and_cond_sync(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pastCondInfo,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, 
		pstCurrScene, 
		pastCondInfo
	);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(
		iCount
	);

	int i = 0;
	cond_list_class_t *pstCondClass =
		pstCurrScene->stAndCond.pstEventCondList;
	
	if(NoErr ==
		pstCondClass->stList.size(
			(link_list_class_t *)pstCondClass
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->event_and_cond_add(
				_this, 
				pstCurrScene, 
				&pastCondInfo[i]
			);
			HY_INFO("Add Event And Cond(%s %s)\n",
				pastCondInfo[i].acDevId,
				pastCondInfo[i].acKey
			);
		}
		return NoErr;
	}
	if(NoErr == iCount)
	{
		_this->event_and_cond_clear(
			_this, 
			pstCurrScene,
			1
		);
		HY_INFO("Clear Event And Cond\n");
		return NoErr;
	}
	char *pacFoundFlag =
		(char *)calloc(iCount, sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return HeapReqErr;
	}

	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(
			pstCondClass->stList.next(
				(link_list_class_t *)pstCondClass,
				NULL
			)
		);
	if(NULL == pstNode)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(
			pstCondClass->stList.head(
				(link_list_class_t *)pstCondClass
			)
		);
	if(NULL == pstHead)
	{
		free(pacFoundFlag);
		return SceneGetErr;
	}
	while(pstNode && pstHead != pstNode)
	{
		event_cond_node_t *p =
			(event_cond_node_t *)(pstNode->stCondSet.pstCond);
		while(p)
		{
			event_cond_t *pstCond = &p->stCond;

			int iFlag = 0;
			for(i = 0; i < iCount; ++i)
			{
				if(!strncmp(
						pstCond->acDevId,
						pastCondInfo[i].acDevId,
						DEV_ID_MAX_LEN
					) &&
					!strncmp(
						pstCond->acKey, 
						pastCondInfo[i].acKey,
						KEY_MAX_LEN
					)
				)
				{
					/*找到,判断数据是否发生改变*/
					if(strncmp(
							pstCond->acTriggerType, 
							pastCondInfo[i].acTriggerType,
							VALUE_MAX_LEN
						) || 
						strncmp(
							pstCond->acContinueTime, 
							pastCondInfo[i].acContinueTime,
							VALUE_MAX_LEN
						) || 
						strncmp(
							pstCond->acValue, 
							pastCondInfo[i].acValue,
							VALUE_MAX_LEN
						) || 
						strncmp(
							pstCond->acActive,
							pastCondInfo[i].acActive,
							ACTIVE_MAX_LEN
						) || 
						strncmp(
							pstCond->acLastValue,
							pastCondInfo[i].acLastValue,
							ACTIVE_MAX_LEN
						) 
					)
					{
						/*信息发生改变*/
						_this->event_and_cond_add(
							_this, 
							pstCurrScene, 
							&pastCondInfo[i]
						);
						HY_INFO("Set Event And Cond(%s %s)\n",
							pastCondInfo[i].acDevId,
							pastCondInfo[i].acKey
						);
					}
					pacFoundFlag[i] = 1;
					iFlag = 1;
					break;
				}
				
			}
			if(!iFlag)
			{
				/*未找到，删除*/
				_this->event_and_cond_del(
					_this, 
					pstCurrScene,
					&pastCondInfo[i]
				);
				HY_INFO("Del Event And Cond(%s %s)\n", 
					pastCondInfo[i].acDevId,
					pastCondInfo[i].acKey
				);
			}
			p = p->next;
		}
	
		pstNode = 
			(cond_list_node_t *)(
				pstCondClass->stList.next(
					(link_list_class_t *)pstCondClass, 
					(link_list_piece_t *)pstNode
				)
			);
	}
	
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->event_and_cond_add(
				_this, 
				pstCurrScene,
				&pastCondInfo[i]
			);
			HY_INFO("Add Event And Cond(%s %s)\n", 
				pastCondInfo[i].acDevId, 
				pastCondInfo[i].acKey
			);
		}
	}
	free(pacFoundFlag);
	
	return NoErr;

}
/*获取条件*/
static int 
scene_event_and_cond_get(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;

	char acHashKey[KEY_MAX_LEN] = {0};
		
	/*判断该条件是否存在Hash表中*/
	snprintf(
		acHashKey, 
		KEY_MAX_LEN, 
		"%s%s%s", 
		pstCondInfo->acDevId, 
		pstCondInfo->acKey,
		PRT_HASH_TYPE_COND
	);
	
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		ptr_info_t* pstPtrInfo = &(pHashValue->stCondPtr);
		event_cond_node_t *pstCondNode = (event_cond_node_t*)(pstPtrInfo->pCondNode);
		event_cond_t *pstCond = &pstCondNode->stCond;

		memcpy(pstCondInfo, pstCond, sizeof(event_cond_t));
		return 0;
	}
	else
	{
		HY_ERROR("Cond Not Found.\n");
		return -1;
	}

}

/*添加条件*/
static int 
scene_event_and_cond_add(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};
	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, 
		"%s%s%s", pstCondInfo->acDevId, pstCondInfo->acKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		/*修改*/
		iRet = 
			pstCurrScene->stAndCond.pstEventCondList->cond_set(
				pstCurrScene->stAndCond.pstEventCondList, 
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
	}
	else
	{
		/*添加*/
		pHashValue = 
			(ptr_node_t *)calloc(1, sizeof(ptr_node_t));
		if(NULL == pHashValue)
		{
			HY_ERROR("Malloc error: %s\n", strerror(errno));
			return HeapReqErr;
		}
		iRet = 
			pstCurrScene->stAndCond.pstEventCondList->cond_add(
				pstCurrScene->stAndCond.pstEventCondList,
				(void*)pstCondInfo, 
				&pHashValue->stCondPtr
			);
		strncpy(pHashValue->stCondPtr.acId, 
			pstCurrScene->acSceneId, INDEX_MAX_LEN);
		pHashValue->stCondPtr.pstScene = pstCurrScene;
		
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_inst(
				pPrivateParam->pHashTable,
				acHashKey, 
				pHashValue
			);
	}
	
	if(NoErr == iRet)
	{
		if(1 == 
			pstCurrScene->stAndCond.pstEventCondList->cond_size(
				pstCurrScene->stAndCond.pstEventCondList
			)
		)
		{
			/*当条件列表由空变为非空时，则将条件列表的真假状态置为假*/
			pstCurrScene->stAndCond.ucCondListTrue &= ~(0x1 << 1);
		}
		else
		{
			/*添加、修改成功，更新初始的真假状态位*/
			int iTrue = 0;
			if(0 == strlen(pstCondInfo->acLastValue))
			{
				iTrue = 0;
			}
			else
			{
				event_t stEventInfo;
				strncpy(stEventInfo.acDevId, pstCondInfo->acDevId, DEV_ID_MAX_LEN);
				strncpy(stEventInfo.acKey, pstCondInfo->acKey, KEY_MAX_LEN);
				strncpy(stEventInfo.acValue, pstCondInfo->acLastValue, VALUE_MAX_LEN);
				iTrue = _scene_event_cond_is_true(pstCondInfo, &stEventInfo);
			}
			
			scene_event_cond_true_init(_this, iTrue, pHashValue);
		}
	}
	
	return iRet;
}

/*删除条件*/
static int 
scene_event_and_cond_del(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pstCondInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrScene, pstCondInfo);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	int iRet = 0;
	char acHashKey[KEY_MAX_LEN] = {0};	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, 
		"%s%s%s", pstCondInfo->acDevId, pstCondInfo->acKey, PRT_HASH_TYPE_COND);
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find(
			pPrivateParam->pHashTable, 
			acHashKey,
			pstCurrScene->acSceneId
	);
	if(NULL != pHashValue)
	{
		iRet =
			pstCurrScene->stAndCond.pstEventCondList->cond_del(
				pstCurrScene->stAndCond.pstEventCondList, 
				&pHashValue->stCondPtr
			);
		if(NoErr == iRet)
		{
			if(0 == 
				pstCurrScene->stAndCond.pstEventCondList->cond_size(
					pstCurrScene->stAndCond.pstEventCondList
				)
			)
			{
				/*当条件列表由非空变为空，则将条件集合真假标志位置假，条件列表真假标志位置真*/
				pstCurrScene->stAndCond.pstEventCondList->uiCondSetTrue = 0;
				pstCurrScene->stAndCond.ucCondListTrue |= 0x1 << 0;
			}
			else
			{
				/*删除成功，更新真假状态位。*/
				cond_list_node_t *pstCondSetNode = 
					(cond_list_node_t *)(pHashValue->stCondPtr.pCondSetNode);

				cond_set_t *pCondSet = &pstCondSetNode->stCondSet;
				unsigned int uiTmp = (unsigned int)0xFFFFFFFF >> (32 - pCondSet->ucCondCount);
				int iTrue = 
					(pCondSet->uiCondTrue & uiTmp) == uiTmp ? 1 : 0;
				scene_event_cond_set_true_init(iTrue, pHashValue);
			}
		}
		iRet += 
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
	}
	else
	{
		HY_ERROR("Not Found The Cond\n");
		return NotFoundErr;
	}
	return iRet;
}


/*获取所有条件*/
static int 
scene_event_and_cond_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	event_cond_t *pastCondInfo, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this,
		pstCurrScene,
		pastCondInfo,
		piCount
	);
	return 
		pstCurrScene->stAndCond.pstEventCondList->cond_get_list(
				pstCurrScene->stAndCond.pstEventCondList, 
				(void*)pastCondInfo, 
				piCount
			);
}
/*删除所有条件*/
static int 
scene_event_and_cond_clear(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrScene);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
		
	char acHashKey[KEY_MAX_LEN] = {0};
	
	cond_list_class_t *pstCondClass = 
		pstCurrScene->stAndCond.pstEventCondList;
	cond_list_node_t *pstHead = 
		(cond_list_node_t *)(pstCondClass->stList.head(
			(link_list_class_t *)pstCondClass
			)
		);
	cond_list_node_t *pstNode = 
		(cond_list_node_t *)(pstCondClass->stList.first(
			(link_list_class_t *)pstCondClass)
		);
	while(pstNode && pstHead != pstNode)
	{
		/*释放结点*/
		event_cond_node_t *p = 
			pstNode->stCondSet.pstCond;
		event_cond_node_t *p1;
		while(p)
		{
			p1 = p;
			p = p->next;
			/*删除Hash*/
			snprintf(
				acHashKey,
				KEY_MAX_LEN,
				"%s%s%s", 
				p1->stCond.acDevId, 
				p1->stCond.acKey,
				PRT_HASH_TYPE_COND
			);
			pPrivateParam->pHashTable->ptr_hash_del(
				pPrivateParam->pHashTable, 
				acHashKey,
				pstCurrScene->acSceneId
			);
			if(1 == iClearDbFlag)
			{
				/*删除数据库*/
				pstCondClass->cond_del_db(pstCondClass, &p1->stCond);
			}
			/*释放结点*/
			free(p1);
		}
		pstNode->stCondSet.ucCondCount = 0;
		pstNode = 
			(cond_list_node_t *)(
				pstCondClass->stList.next(
					(link_list_class_t *)pstCondClass, 
					(link_list_piece_t *)pstNode
				)
			);
	}
	pstCondClass->iCount = 0;
	pstCondClass->uiCondSetTrue = 0;
	
	return 
		pstCondClass->stList.clear(
			(link_list_class_t *)pstCondClass
		);
}








/*动作*/
/*添加动作*/
static int 
scene_action_add(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene,
	action_info_t *pstActionInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this,
		pstCurrScene,
		pstActionInfo
	);
	return pstCurrScene->pstAction->action_add(
				pstCurrScene->pstAction, 
				pstActionInfo
			);
}

/*删除动作*/
static int 
scene_action_del(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	action_info_t *pstActionInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(
		_this, 
		pstCurrScene,
		pstActionInfo
	);
	return pstCurrScene->pstAction->action_del(
				pstCurrScene->pstAction,
				pstActionInfo
			);
}


/*获取所有动作*/
static int 
scene_action_get_list(
	scene_list_class_t *_this, 
	scene_info_t *pstCurrScene, 
	action_info_t *pastActionInfo,
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(
		_this,
		pstCurrScene,
		pastActionInfo,
		piCount
	);
	return 
		pstCurrScene->pstAction->action_get_list(
			pstCurrScene->pstAction, 
			pastActionInfo,
			piCount
		);
}

/*清空所有动作*/
static int 
scene_action_clear(
	scene_list_class_t *_this,
	scene_info_t *pstCurrScene, 
	int iClearDbFlag
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(
		_this
	);
	action_list_class_t *pstActionClass = 
			pstCurrScene->pstAction;
	if(1 == iClearDbFlag)
	{
		action_list_node_t *pstHead = 
			(action_list_node_t *)(pstActionClass->stList.head(
				(link_list_class_t *)pstActionClass
				)
			);
		action_list_node_t *pstNode = 
			(action_list_node_t *)(pstActionClass->stList.first(
				(link_list_class_t *)pstActionClass)
			);
		while(pstNode && pstHead != pstNode)
		{
			/*删除数据库*/
			pstActionClass->action_del_db(pstActionClass, &pstNode->stActionInfo);
			
			pstNode = 
				(action_list_node_t *)(
					pstActionClass->stList.next(
						(link_list_class_t *)pstActionClass, 
						(link_list_piece_t *)pstNode
					)
				);
		}
	}
	
	return 
		pstActionClass->stList.clear(
			(link_list_class_t *)pstActionClass
		);
}







static int 
scene_list_exec(
	scene_list_class_t *_this,
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);

	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	link_list_class_t *pstQueue = 
		pPrivateParam->pstExecQueue;
	
	/*将该场景添加到执行队列中*/
	link_queue_node_t *pstQueueValue = 
		(link_queue_node_t *)calloc(1, sizeof(link_queue_node_t));
	if(NULL == pstQueueValue)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return HeapReqErr;
	}
	pstQueueValue->iExecDelayed = atoi(pstInfo->acExecDelayed);
	pstQueueValue->iTime = time(NULL);
	memcpy(&(pstQueueValue->stSceneInfo), pstInfo, sizeof(scene_info_t));
	
	pstQueue->inst_tail(pstQueue, (link_list_piece_t *)pstQueueValue);

	HY_INFO("Scene(%s) join the execution queue\n", pstInfo->acSceneId);
	return NoErr;
}


static int 
scene_list_active(
	scene_list_class_t *_this,
	scene_info_t *pstInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInfo);
	cond_list_class_t *pstOrTimeCondList = 
		pstInfo->stOrCond.pstTimeCondList;
	cond_list_class_t *pstOrEventCondList = 
		pstInfo->stOrCond.pstEventCondList;
	cond_list_class_t *pstAndTimeCondList = 
		pstInfo->stAndCond.pstTimeCondList;
	cond_list_class_t *pstAndEventCondList = 
		pstInfo->stAndCond.pstEventCondList;

	/*判断场景是否有手动触发条件*/
	if(1 == pstInfo->cManualTriggerFlag)
	{
		/*有手动触发条件*/
		/*判断场景是否使能*/
		if(!strncmp(pstInfo->acSceneEnable, "1", STATE_MAX_LEN))
		{
			/*使能*/
			/*执行场景*/
			return _this->scene_exec(_this, pstInfo);
		}
		else
		{
			/*不使能*/
			;//do nothing
		}
	}
	else
	{
		/*无手动触发条件*/
		/*判断场景是否使能*/
		if(!strncmp(pstInfo->acSceneEnable, "1", STATE_MAX_LEN))
		{
			/*使能*/
			/*判断场景条件是否为空*/
			if(0 == pstOrTimeCondList->cond_size(pstOrTimeCondList) + 
				pstOrEventCondList->cond_size(pstOrEventCondList) + 
				pstAndTimeCondList->cond_size(pstAndTimeCondList) + 
				pstAndEventCondList->cond_size(pstAndEventCondList)
			)
			{
				/*为空*/
				return _this->scene_exec(_this, pstInfo);
			}
			else
			{
				/*不为空，即该场景为自动场景*/
				;//do nothing
			}
		}
		else
		{
			/*不使能*/
			;//do nothing
		}
		
	}

	return NoErr;
}


/*场景上报注册接口*/
static int scene_report_reg(
	scene_list_class_t *_this, 
	SceneReportFun pSceneReportHandler
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pSceneReportHandler);

	_this->pSceneReportHandler = pSceneReportHandler;

	return NoErr;
}

static int 
scene_list_event(
	scene_list_class_t *_this,
	event_t *pstEventInfo
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstEventInfo);

	HY_DEBUG("event : %s, %s, %s\n", 
		pstEventInfo->acDevId, 
		pstEventInfo->acKey, 
		pstEventInfo->acValue
	);
	int iRet = 0;
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	
	char acHashKey[KEY_MAX_LEN] = {0};


	/*条件事件处理*/

	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, "%s%s%s",
		pstEventInfo->acDevId, pstEventInfo->acKey, PRT_HASH_TYPE_COND);
	
	ptr_node_t *pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find_all(
			pPrivateParam->pHashTable, 
			acHashKey
		);
	if(NULL != pHashValue)
	{
		ptr_node_t *p = pHashValue->next;
		while(p)
		{
			/*由于同一个条件可以在不同的场景中存在，所以一个事件的发生有可能改变多个场景条件的真假*/
			scene_info_t *pstScene = 
				(scene_info_t *)(p->stCondPtr.pstScene);
			event_cond_node_t *pstCondNode = 
				(event_cond_node_t*)(p->stCondPtr.pCondNode);
			/*如果该场景未激活，则直接跳出*/
			if(!strncmp(
					pstScene->acSceneEnable,
					"0", 
					STATE_MAX_LEN
				)
			)
			{
				p = p->next;
				continue;
			}

			int iTrue = 0;
			event_cond_t *pstCond = &(pstCondNode->stCond);
			iTrue = _scene_event_cond_is_true(pstCond, pstEventInfo);
			
			HY_DEBUG("Event Cond is %s\n", iTrue == 1 ? "True" : "False");
			/*更新事件状态*/
			if(strncmp(pstCond->acLastValue, pstEventInfo->acValue, VALUE_MAX_LEN))
			{
				strncpy(pstCond->acLastValue, pstEventInfo->acValue, VALUE_MAX_LEN);

				/*存储数据库*/
				cond_list_class_t *pstCondClass = 
					(cond_list_class_t *)(p->stCondPtr.pstCondList);
	
				iRet = pstCondClass->cond_set_db(pstCondClass, pstCond);
				if(NoErr != iRet)
				{
					HY_ERROR("Failed to add manual trigger condition.\n");
					return iRet;
				}
			}
			/*更新条件相关真假*/
			scene_event_cond_true_update(_this, iTrue, p);
			
			p = p->next;
		}

		return NoErr;
	}


	/*实体场景面板处理*/
	
	/*判断该条件是否存在Hash表中*/
	snprintf(acHashKey, KEY_MAX_LEN, "%s%s%s%s",
		pstEventInfo->acDevId, 
		pstEventInfo->acKey, 
		pstEventInfo->acValue, 
		PRT_HASH_TYPE_SCENE_BIND
	);
	
	pHashValue = 
		pPrivateParam->pHashTable->ptr_hash_find_all(
			pPrivateParam->pHashTable, 
			acHashKey
		);
	if(NULL != pHashValue)
	{
		ptr_node_t *p = pHashValue->next;
		while(p)
		{
			scene_info_t *pstScene = 
				(scene_info_t *)(p->stCondPtr.pstScene);
			
			scene_panel_node_t *pstPanelNode = 
				(scene_panel_node_t *)(p->stCondPtr.pScenePanelNode);
			//if(strncmp(
			//		pstScene->acSceneEnable,
			//		pstPanelNode->stPanel.acEnable, 
			//		STATE_MAX_LEN
			//	)
			//)
			//{
				/*更改场景使能*/
			//	strncpy(
			//		pstScene->acSceneEnable,
			//		pstPanelNode->stPanel.acEnable,
			//		STATE_MAX_LEN);
				/*数据库更新*/
			//	pPrivateMethods->scene_add_db(_this, pstScene);
			//}

			if(!strncmp(
					pstPanelNode->stPanel.acEnable,
					"1", 
					STATE_MAX_LEN
				)
			)
			{
				/*激活场景*/
				_this->scene_active(_this, pstScene);
			}
			p = p->next;
		}

		return NoErr;
	}
	return NoErr;
}

static int 
scene_list_time_true_all(
	scene_list_class_t *_this
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	
	if(0 == _this->stList.size((link_list_class_t *)_this))
	{
		HY_ERROR("The Scene List is Empty.\n");
		return NoErr;
	}

	/*遍历所有场景*/
	scene_list_node_t *pstPtr = 
		(scene_list_node_t *)(_this->stList.next(
			(link_list_class_t *)_this,
			NULL)
		);
	if(NULL == pstPtr)
	{
		HY_ERROR("Get Fist Node Error.\n");
		return SceneGetErr;
	}
	scene_list_node_t *pstHead = 
		(scene_list_node_t *)(
			_this->stList.head(
				(link_list_class_t *)_this
			)
		);
	if(NULL == pstHead)
	{
		HY_ERROR("Get Head Node Error.\n");
		return SceneGetErr;
	}
	
	while(pstPtr && pstHead != pstPtr)
	{
		{
			/*时间或条件列表*/
			cond_list_class_t *pstOrTimeCondList = pstPtr->stSceneInfo.stOrCond.pstTimeCondList;
			
			/*遍历所有或时间条件集合*/
			cond_list_node_t *pstCondSetNodePtr = 
				(cond_list_node_t *)(pstOrTimeCondList->stList.first(
					(link_list_class_t *)pstOrTimeCondList)
				);

			cond_list_node_t *pstCondSetHead = 
				(cond_list_node_t *)(pstOrTimeCondList->stList.head(
					(link_list_class_t *)pstOrTimeCondList)
				);

			while(pstCondSetNodePtr && pstCondSetHead != pstCondSetNodePtr)
			{
				/*遍历所有的或时间条件*/
				time_cond_node_t *p = pstCondSetNodePtr->stCondSet.pstCond;
				while(p)
				{
					int iNewTrue = _scene_time_cond_is_true(&p->stCond);
					
					/*更新时间条件*/
					ptr_node_t stCondPtrNode;
					memset(&stCondPtrNode, 0x0, sizeof(ptr_node_t));
					strncpy(stCondPtrNode.stCondPtr.acId, pstPtr->stSceneInfo.acSceneId, INDEX_MAX_LEN);
					stCondPtrNode.stCondPtr.pstScene = (void*)(&pstPtr->stSceneInfo);
					stCondPtrNode.stCondPtr.pstCondList = (void*)pstOrTimeCondList;
					stCondPtrNode.stCondPtr.pCondSetNode = (void*)pstCondSetNodePtr;
					stCondPtrNode.stCondPtr.pCondNode = (void*)p;

					scene_time_cond_true_update(
						_this, 
						iNewTrue, 
						&stCondPtrNode
					);
							
					/*next cond*/
					p = p->next;
				}

				/*next cond set*/
				pstCondSetNodePtr = 
					(cond_list_node_t *)(_this->stList.next(
						(link_list_class_t *)_this, 
						(link_list_piece_t *)pstCondSetNodePtr)
					);
			}
		}
		{
			/*时间与条件列表*/
			cond_list_class_t *pstAndTimeCondList = pstPtr->stSceneInfo.stAndCond.pstTimeCondList;
			/*遍历所有与时间条件集合*/
			cond_list_node_t *pstCondSetNodePtr = 
				(cond_list_node_t *)(pstAndTimeCondList->stList.first(
					(link_list_class_t *)pstAndTimeCondList)
				);

			cond_list_node_t *pstCondSetHead = 
				(cond_list_node_t *)(pstAndTimeCondList->stList.head(
					(link_list_class_t *)pstAndTimeCondList)
				);

			while(pstCondSetNodePtr && pstCondSetHead != pstCondSetNodePtr)
			{
				/*遍历所有的或时间条件*/
				time_cond_node_t *p = pstCondSetNodePtr->stCondSet.pstCond;
				while(p)
				{
					int iNewTrue = _scene_time_cond_is_true(&p->stCond);

					/*更新时间条件*/
					ptr_node_t stCondPtrNode;
					memset(&stCondPtrNode, 0x0, sizeof(ptr_node_t));
					strncpy(stCondPtrNode.stCondPtr.acId, pstPtr->stSceneInfo.acSceneId, INDEX_MAX_LEN);
					stCondPtrNode.stCondPtr.pstScene = (void*)(&pstPtr->stSceneInfo);
					stCondPtrNode.stCondPtr.pstCondList = (void*)pstAndTimeCondList;
					stCondPtrNode.stCondPtr.pCondSetNode = (void*)pstCondSetNodePtr;
					stCondPtrNode.stCondPtr.pCondNode = (void*)p;

					scene_time_cond_true_update(
						_this, 
						iNewTrue, 
						&stCondPtrNode
					);
	
					/*next cond*/
					p = p->next;
				}

				/*next cond set*/
				pstCondSetNodePtr = 
					(cond_list_node_t *)(_this->stList.next(
						(link_list_class_t *)_this, 
						(link_list_piece_t *)pstCondSetNodePtr)
					);
			}
		}
		/*next cond list*/
		pstPtr = 
			(scene_list_node_t *)(_this->stList.next(
				(link_list_class_t *)_this, 
				(link_list_piece_t *)pstPtr)
			);
	}
	return NoErr;
}

static int 
scene_list_print(scene_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return 
		_this->stList.print(
			(link_list_class_t *)_this,
			print_scene_info_handle
		);
}


/*场景条件判断线程执行函数*/
static void*
scene_cond(void *arg)
{
	scene_list_class_t *pstSceneClass =
		(scene_list_class_t *)arg;
	/*将线程设置成可取消状态*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	int iTimeCondCheck = 0;

	while(1)
	{
		/*减少cpu的占用*/
		sleep(1);
		
#if 1		
		/*当前时间*/
		time_t iNow = time(NULL);

		/*时间条件真假判断，每1分钟一次*/
		if(iNow - iTimeCondCheck >= 60)
		{
			/*锁住场景*/
			pstSceneClass->scene_lock(pstSceneClass);
			HY_DEBUG("Time Cond Check\n");
			if(pstSceneClass->stList.size((link_list_class_t*)pstSceneClass) > 0)
			{
				pstSceneClass->scene_time_true_all(pstSceneClass);
			}
			/*解锁场景*/
			pstSceneClass->scene_unlock(pstSceneClass);
			
			/*复位时间标志位*/
			if(iNow - iTimeCondCheck >= 60)
			{
				iTimeCondCheck = iNow;
			}
		}
#endif	
		/*线程取消点*/
		pthread_testcancel();
	}
	return NULL;
}


/*场景线程执行函数*/
static void*
scene_exec(void *arg)
{
	scene_list_class_t *pstSceneClass =
		(scene_list_class_t *)arg;
	
	/*将线程设置成可取消状态*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	scene_info_t *pScene = NULL;
	
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)pstSceneClass->acScenelistPrivateParam;
	link_list_class_t *pstQueue = 
		pPrivateParam->pstExecQueue;
	
	while(1)
	{
		/*减少cpu的占用*/
		usleep(200 * 1000);
		if(0 == pstQueue->size(pstQueue))
		{
			goto scene_exec_loop_end;
		}
#if 1			
		/*执行场景*/

		link_queue_node_t *pstPtr = 
			(link_queue_node_t *)(pstQueue->next(
				pstQueue,
				NULL)
			);
		link_queue_node_t *pstHead = 
			(link_queue_node_t *)(pstQueue->head(pstQueue));
		if(NULL == pstPtr || NULL == pstHead)
		{
			continue;
		}
		while(pstPtr && pstHead != pstPtr)
		{
			if(0 == pstPtr->iExecDelayed)
			{
				/*立即执行*/
				pScene = &(pstPtr->stSceneInfo);
				HY_DEBUG("Exec the Scene(%s).\n", pScene->acSceneId);
				/*锁住场景*/
				pstSceneClass->scene_lock(pstSceneClass);
				/*执行动作*/
				pScene->pstAction->action_exec_all(pScene->pstAction);
				/*解锁场景*/
				pstSceneClass->scene_unlock(pstSceneClass);
				/*上报执行的场景ID*/
				scene_report_scene_exec(pstSceneClass, pScene->acSceneId);


				/*执行后删除*/
				link_queue_node_t *pstPtrTmp = pstPtr;
				pstPtr = 
					(link_queue_node_t *)(pstQueue->next(
						pstQueue, 
						(link_list_piece_t *)pstPtr)
					);
				pstQueue->del(pstQueue, (link_list_piece_t *)pstPtrTmp);
				continue;
			}
			else
			{
				/*判断延时条件是否满足*/
				if(time(NULL) - pstPtr->iTime >= pstPtr->iExecDelayed)
				{
					/*执行*/
					pScene = &(pstPtr->stSceneInfo);
					HY_DEBUG("Exec the Scene(%s).\n", pScene->acSceneId);
					/*锁住场景*/
					pstSceneClass->scene_lock(pstSceneClass);
					/*执行动作*/
					pScene->pstAction->action_exec_all(pScene->pstAction);
					/*解锁场景*/
					pstSceneClass->scene_unlock(pstSceneClass);
					/*上报执行的场景ID*/
					scene_report_scene_exec(pstSceneClass, pScene->acSceneId);

					/*执行后删除*/
					link_queue_node_t *pstPtrTmp = pstPtr;
					pstPtr = 
						(link_queue_node_t *)(pstQueue->next(
							pstQueue, 
							(link_list_piece_t *)pstPtr)
						);
					pstQueue->del(pstQueue, (link_list_piece_t *)pstPtrTmp);
					continue;
				}
			}
			pstPtr = 
				(link_queue_node_t *)(pstQueue->next(
					pstQueue, 
					(link_list_piece_t *)pstPtr)
				);
		}
#endif	
scene_exec_loop_end:

		/*线程取消点*/
		pthread_testcancel();
	}
	return NULL;
}


/*构造函数*/
scene_list_class_t* new_scene_list(DB *pDb)
{
	PARAM_CHECK_RETURN_NULL_1(pDb);
	
	int iDataSize = sizeof(scene_list_node_t);
	HY_DEBUG("iDataSize = %d\n", iDataSize);

	/*申请空间*/
	scene_list_class_t *pNew = 
		(scene_list_class_t *)calloc(1, sizeof(scene_list_class_t));
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
	
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)pNew->acScenelistPrivateParam;

	/*初始化互斥锁*/
	pPrivateParam->mutex = base_mutex_lock_create();
	if(NULL == pPrivateParam->mutex)
	{
		HY_ERROR("The mutex creation failed.\n");
		free(pNew);
        return NULL;
	}
	/*创建哈希表*/
	pPrivateParam->pHashTable = new_ptr_hash_table();
	if(NULL == pPrivateParam->pHashTable)
	{
		HY_ERROR("new_hash_table Error.\n");
		base_mutex_lock_destroy(pPrivateParam->mutex);
		free(pNew);
        return NULL;
	}

	/*创建联动执行队列*/
	pPrivateParam->pstExecQueue = 
		new_link_list(sizeof(link_queue_node_t));
	if(NULL == pPrivateParam->pstExecQueue)
	{
		HY_ERROR("new_circle_queue Error.\n");
		base_mutex_lock_destroy(pPrivateParam->mutex);
		destroy_ptr_hash_table(pPrivateParam->pHashTable);
		free(pNew);
        return NULL;
	}
	/*初始化定时器*/
	{
		scene_task_timer_init(TIMER_NUM);
	}
	
	/*创建场景线程*/
    {
    	if(0 != base_thread_create(
			&pPrivateParam->pid_exec, 
			scene_exec, 
			(void*)pNew)
		)
		{
			HY_ERROR("pthread_attr_init Error.\n");
			base_mutex_lock_destroy(pPrivateParam->mutex);
			destroy_ptr_hash_table(pPrivateParam->pHashTable);
			destroy_link_list(pPrivateParam->pstExecQueue);
			scene_task_timer_destroy();
			free(pNew);
            return NULL;
		}
		if(0 != base_thread_create(
			&pPrivateParam->pid_cond, 
			scene_cond, 
			(void*)pNew)
		)
		{
			HY_ERROR("pthread_attr_init Error.\n");
			base_mutex_lock_destroy(pPrivateParam->mutex);
			destroy_ptr_hash_table(pPrivateParam->pHashTable);
			destroy_link_list(pPrivateParam->pstExecQueue);
			scene_task_timer_destroy();
			free(pNew);
            return NULL;
		}
    }

	scene_list_private_methods_t *pPrivateMethods = 
		(scene_list_private_methods_t *)pNew->acScenelistPrivateMethods;
	
	pPrivateMethods->scene_table_db = scene_list_table_db;
	pPrivateMethods->scene_add_db = scene_list_add_db;
	pPrivateMethods->scene_del_db = scene_list_del_db;
	pPrivateMethods->scene_clear_db = scene_list_clear_db;
	pPrivateMethods->scene_set_db = scene_list_set_db;
	pPrivateMethods->scene_get_db = scene_list_get_db;
	pPrivateMethods->scene_get_list_db = scene_list_get_list_db;
	pPrivateMethods->scene_info_set = scene_list_info_set;
	pPrivateMethods->scene_info_get = scene_list_info_get;
 	pPrivateMethods->scene_bind_db = scene_bind_db;
	pPrivateMethods->scene_bind_set_db = scene_bind_set_db;
	pPrivateMethods->scene_unbind_db = scene_unbind_db;
	pPrivateMethods->scene_bind_get_db = scene_bind_get_db;
	pPrivateMethods->scene_bind_get_list_db = scene_bind_get_list_db;

	pNew->scene_lock = scene_list_lock;
	pNew->scene_unlock = scene_list_unlock;
	pNew->scene_init = scene_list_init;
	pNew->scene_sync = scene_list_sync;
	pNew->scene_clear = scene_list_clear;
	pNew->scene_add = scene_list_add;
	pNew->scene_del = scene_list_del;
	pNew->scene_set = scene_list_set;
	pNew->scene_get = scene_list_get;
	pNew->scene_get_list = scene_list_get_list;
	pNew->scene_exec = scene_list_exec;	
	pNew->scene_active = scene_list_active;	
	pNew->scene_report_reg = scene_report_reg;
	pNew->scene_event = scene_list_event;
	pNew->scene_time_true_all = scene_list_time_true_all;
	pNew->scene_print = scene_list_print;
	pNew->scene_dev_unregister = scene_list_dev_unregister;
	
	/*场景绑定*/
	pNew->scene_bind_init = scene_bind_init;
	pNew->scene_bind_sync = scene_bind_sync;
	pNew->scene_bind = scene_bind;
	pNew->scene_unbind = scene_unbind;
	pNew->scene_bind_get_list = scene_bind_get_list;
	pNew->scene_bind_clear = scene_bind_clear;
	
	/*时间或集合*/	
	pNew->time_or_cond_init = scene_time_or_cond_init;
	pNew->time_or_cond_sync = scene_time_or_cond_sync;
	pNew->time_or_cond_add = scene_time_or_cond_add;
	pNew->time_or_cond_del = scene_time_or_cond_del;
	pNew->time_or_cond_get_list = scene_time_or_cond_get_list;
	pNew->time_or_cond_clear = scene_time_or_cond_clear;
	
	/*事件或集合*/
	pNew->event_or_cond_init = scene_event_or_cond_init;
	pNew->event_or_cond_sync = scene_event_or_cond_sync;
	pNew->event_or_cond_get = scene_event_or_cond_get;
	pNew->event_or_cond_add = scene_event_or_cond_add;
	pNew->event_or_cond_del = scene_event_or_cond_del;
	pNew->event_or_cond_get_list = scene_event_or_cond_get_list;
	pNew->event_or_cond_clear = scene_event_or_cond_clear;
	
	/*时间与集合*/
	pNew->time_and_cond_init = scene_time_and_cond_init;
	pNew->time_and_cond_sync = scene_time_and_cond_sync;
	pNew->time_and_cond_add = scene_time_and_cond_add;
	pNew->time_and_cond_del = scene_time_and_cond_del;
	pNew->time_and_cond_get_list = scene_time_and_cond_get_list;
	pNew->time_and_cond_clear = scene_time_and_cond_clear;
	
	/*事件与集合*/
	pNew->event_and_cond_init = scene_event_and_cond_init;
	pNew->event_and_cond_sync = scene_event_and_cond_sync;
	pNew->event_and_cond_get = scene_event_and_cond_get;
	pNew->event_and_cond_add = scene_event_and_cond_add;
	pNew->event_and_cond_del = scene_event_and_cond_del;
	pNew->event_and_cond_get_list = scene_event_and_cond_get_list;
	pNew->event_and_cond_clear = scene_event_and_cond_clear;
	
	/*动作*/
	pNew->action_add = scene_action_add;
	pNew->action_del = scene_action_del;
	pNew->action_get_list = scene_action_get_list;
	pNew->action_clear = scene_action_clear;
	return pNew;
}

/*析构函数*/
int destroy_scene_list(scene_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	scene_list_private_param_t *pPrivateParam = 
		(scene_list_private_param_t *)_this->acScenelistPrivateParam;
	/*取消线程*/
	base_thread_cancel(pPrivateParam->pid_exec);
	base_thread_cancel(pPrivateParam->pid_cond);
	/*销毁哈希表*/
	destroy_ptr_hash_table(pPrivateParam->pHashTable);
	/*销毁队列*/
	destroy_link_list(pPrivateParam->pstExecQueue);
	/*销毁定时器*/
	scene_task_timer_destroy();
	/*销毁锁*/
	base_mutex_lock_destroy(pPrivateParam->mutex);
	if(_this->stList.size((link_list_class_t*)_this))
	{
		/*清空链表*/
		_this->scene_clear(_this, 0);
	}
	
	/*释放头结点*/
	free(_this->stList.pHead);
	_this->stList.pHead = _this->stList.pTail = NULL;
	free(_this);
	_this = NULL;
	return 0;
}

