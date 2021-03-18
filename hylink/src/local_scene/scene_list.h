/***********************************************************
*文件名    : scene_list.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.04
*说   明   : 本地化场景接口
*
*修改记录: 
************************************************************/
#ifndef SCENE_LIST_H
#define SCENE_LIST_H

#include "scene_link_list.h"
#include "condition_list.h"
#include "action_list.h"
#include "len.h"
#include "db_api.h" 

#define SCENE_LIST_PRIVATE_PARAM_MAX_LEN	128
#define SCENE_LIST_PRIVATE_METHODS_MAX_LEN	128
#define SCENE_MAX_NUM	256
#define SCENE_BIND_MAX_NUM	128
#define SCENE_EXEC_MAX_NUM	10
#define SCENE_DELAYED_MAX_NUM	20

/*场景执行上报注册接口*/
typedef int (*SceneReportFun)(void *);


#pragma pack(1)

/*场景面板*/
typedef struct scene_panel_s
{
	/*场景面板id*/
	char acDevId[DEV_ID_MAX_LEN];
	/*场景面板对象*/
	char acKey[KEY_MAX_LEN];
	/*场景面板值*/
	char acValue[VALUE_MAX_LEN];
	/*场景使能*/
	char acEnable[STATE_MAX_LEN];
}scene_panel_t;

/*场景面板信息列表结点*/
typedef struct scene_panel_node_s
{
	struct scene_panel_node_s *next;
	struct scene_panel_node_s *prev;
	scene_panel_t stPanel;
}scene_panel_node_t;

typedef struct cond_logic_s
{
	/*整体与条件集合的真假*/
	/*bit0 pstTimeCondList; bit1 pstEventCondList*/
	unsigned char ucCondListTrue;
	cond_list_class_t *pstTimeCondList;
	cond_list_class_t *pstEventCondList;
}cond_logic_t;



/*条件结构*/
/*										/条件0
*                             条件集合0 - ...
*                            /			\条件31
*			     时间条件列表- ...
*	   		   /			 \条件集合31
*         与条件
*       / 		\事件条件列表
	   /       
*     /
*    /						  			/条件0
*条件							 条件集合0 - ...
*    \					    /			\条件31
*     \        时间条件列表- ...
	   \      /			   \条件集合31
*		或条件
*		      \事件条件列表
*/
/*ucSceneId唯一确认一个场景*/

typedef struct scene_info_s
{
	/*场景ID*/
	char acSceneId[INDEX_MAX_LEN];
	/*场景名称*/
	char acSceneName[NAME_MAX_LEN];
	/*是否可被手动激活*/
	char cManualTriggerFlag;
	/*场景使能*/
	char acSceneEnable[STATE_MAX_LEN];
	/*延时执行时间*/
	char acExecDelayed[DELAYED_MAX_LEN];
	/*场景最后修改时间*/
	char acUpdateTime[TIME_MAX_LEN];
	/*场景备注*/
	char acNote[NOTE_MAX_LEN];
	/*整体场景条件真假*/
	unsigned char ucSceneCondTrue;

	/*场景绑定场景面板*/
	scene_panel_node_t *pstPanel;
	/*或条件集合*/
	cond_logic_t stOrCond;
	/*与条件集合*/
	cond_logic_t stAndCond;
	/*动作*/
	action_list_class_t *pstAction;
}scene_info_t;

typedef struct scene_list_node_s
{
	link_list_piece_t stLinkPiece;
	scene_info_t stSceneInfo;
}scene_list_node_t;

/*场景列表*/
typedef struct scene_list_class_s
{
	/*继承链表类*/
	link_list_class_t stList;

	/*参数*/
	/*private*/
	unsigned char acScenelistPrivateParam[SCENE_LIST_PRIVATE_PARAM_MAX_LEN];
	/*数据服务*/
	DB *pDb;
	/*场景激活上报函数地址*/
	SceneReportFun pSceneReportHandler;
	/*初始化阶段*/
	unsigned char ucInitFlag;
	
	/*方法*/
	/*private*/
	unsigned char acScenelistPrivateMethods[SCENE_LIST_PRIVATE_METHODS_MAX_LEN];
	/*方法*/

	/*锁住场景*/
	void (*scene_lock)(struct scene_list_class_s *);
	/*解锁场景*/
	void (*scene_unlock)(struct scene_list_class_s *);
	
	/*初始化场景链表，即将数据库中的信息加载到内存中*/
	int (*scene_init)(struct scene_list_class_s *);
	/*同步场景*/
	int (*scene_sync)(struct scene_list_class_s *, scene_info_t *, int);

	/*清空场景*/
	int (*scene_clear)(struct scene_list_class_s *, int);
	/*添加场景*/
	scene_info_t * (*scene_add)(struct scene_list_class_s *, scene_info_t *);
	/*删除场景*/
	int (*scene_del)(struct scene_list_class_s *, char *);
	/*修改动作*/
	int (*scene_set)(struct scene_list_class_s *, scene_info_t *);
	/*获取场景*/
	scene_info_t * (*scene_get)(struct scene_list_class_s *, char *);
	/*获取所有场景*/
	int (*scene_get_list)(struct scene_list_class_s *, scene_info_t *, int *);
	
	/*初始化场景绑定*/
	int (*scene_bind_init)(struct scene_list_class_s *, scene_info_t *);
	/*同步场景绑定*/
	int (*scene_bind_sync)(struct scene_list_class_s *, scene_info_t *, scene_panel_t *, int);
	/*场景绑定实体场景面板*/
	int (*scene_bind)(struct scene_list_class_s *, scene_info_t *, scene_panel_t *);
	/*场景解绑实体场景面板*/
	int (*scene_unbind)(struct scene_list_class_s *, scene_info_t *, scene_panel_t *);
	/*获取场景绑定的实体场景面板*/
	int (*scene_bind_get_list)(struct scene_list_class_s *, scene_info_t *, scene_panel_t *, int *);
	/*清空场景绑定*/
	int (*scene_bind_clear)(struct scene_list_class_s *, scene_info_t *, int);

	
	/*时间或集合*/
	/*初始化条件*/
	int (*time_or_cond_init)(struct scene_list_class_s *, scene_info_t *);
	/*同步条件*/
	int (*time_or_cond_sync)(struct scene_list_class_s *, scene_info_t *, time_cond_t *, int);
	/*添加条件*/
	int (*time_or_cond_add)(struct scene_list_class_s *, scene_info_t *, time_cond_t *);
	/*删除条件*/
	int (*time_or_cond_del)(struct scene_list_class_s *, scene_info_t *, time_cond_t *);
	/*获取所有条件*/
	int (*time_or_cond_get_list)(struct scene_list_class_s *, scene_info_t *, time_cond_t *, int *);
	/*清空条件*/
	int (*time_or_cond_clear)(struct scene_list_class_s *, scene_info_t *, int);
	
	/*事件或集合*/
	/*初始化条件*/
	int (*event_or_cond_init)(struct scene_list_class_s *, scene_info_t *);
	/*同步条件*/
	int (*event_or_cond_sync)(struct scene_list_class_s *, scene_info_t *, event_cond_t *, int);
	/*获取指定条件*/
	int (*event_or_cond_get)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*添加条件*/
	int (*event_or_cond_add)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*删除条件*/
	int (*event_or_cond_del)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*获取所有条件*/
	int (*event_or_cond_get_list)(struct scene_list_class_s *, scene_info_t *, event_cond_t *, int *);
	/*清空条件*/
	int (*event_or_cond_clear)(struct scene_list_class_s *, scene_info_t *, int);
	
	/*时间与集合*/
	/*初始化条件*/
	int (*time_and_cond_init)(struct scene_list_class_s *, scene_info_t *);
	/*同步条件*/
	int (*time_and_cond_sync)(struct scene_list_class_s *, scene_info_t *, time_cond_t *, int);
	/*添加条件*/
	int (*time_and_cond_add)(struct scene_list_class_s *, scene_info_t *, time_cond_t *);
	/*删除条件*/
	int (*time_and_cond_del)(struct scene_list_class_s *, scene_info_t *, time_cond_t *);
	/*获取所有条件*/
	int (*time_and_cond_get_list)(struct scene_list_class_s *, scene_info_t *, time_cond_t *, int *);
	/*清空条件*/
	int (*time_and_cond_clear)(struct scene_list_class_s *, scene_info_t *, int);

	
	/*事件与集合*/
	/*初始化条件*/
	int (*event_and_cond_init)(struct scene_list_class_s *, scene_info_t *);
	/*同步条件*/
	int (*event_and_cond_sync)(struct scene_list_class_s *, scene_info_t *, event_cond_t *, int);
	/*获取指定条件*/
	int (*event_and_cond_get)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*添加条件*/
	int (*event_and_cond_add)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*删除条件*/
	int (*event_and_cond_del)(struct scene_list_class_s *, scene_info_t *, event_cond_t *);
	/*获取所有条件*/
	int (*event_and_cond_get_list)(struct scene_list_class_s *, scene_info_t *, event_cond_t *, int *);
	/*清空条件*/
	int (*event_and_cond_clear)(struct scene_list_class_s *, scene_info_t *, int);

	
	/*动作*/
	/*添加动作*/
	int (*action_add)(struct scene_list_class_s *, scene_info_t *, action_info_t *);
	/*删除动作*/
	int (*action_del)(struct scene_list_class_s *, scene_info_t *, action_info_t *);
	/*获取所有动作*/
	int (*action_get_list)(struct scene_list_class_s *, scene_info_t *, action_info_t *, int*);
	/*清空动作*/
	int (*action_clear)(struct scene_list_class_s *, scene_info_t *, int);
	
	/*执行场景，不关心条件是否满足，强制执行动作*/
	int (*scene_exec)(struct scene_list_class_s *, scene_info_t *);
	/*激活场景，如果条件满足，或条件为空，则执行动作*/
	int (*scene_active)(struct scene_list_class_s *, scene_info_t *);
	/*场景上报注册接口*/
	int (*scene_report_reg)(struct scene_list_class_s *, SceneReportFun);
	
	/*事件发生*/
	int (*scene_event)(struct scene_list_class_s *, event_t *);
	/*时间条件判断*/
	int (*scene_time_true_all)(struct scene_list_class_s *);

	
	/*打印所有动作，仅用于调试打印*/
	int (*scene_print)(struct scene_list_class_s *);

	/*删除所有场景中，该设备的相关信息*/
	int (*scene_dev_unregister)(struct scene_list_class_s *, char *);
	
}scene_list_class_t;

#pragma pack()


/*构造函数*/
scene_list_class_t* new_scene_list(DB *pDb);

/*析构函数*/
int destroy_scene_list(scene_list_class_t *_this);




#endif /* SCENE_LIST_H */
