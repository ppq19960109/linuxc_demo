/***********************************************************
*文件名    : action_list.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.04
*说   明   : 动作接口
*修改记录: 
************************************************************/
#ifndef ACTION_LIST_H
#define ACTION_LIST_H

#include "scene_link_list.h"
#include "len.h"
#include "db_api.h" 

#define ACTION_LIST_PRIVATE_METHODS_MAX_LEN	128
#define ACTION_MAX_NUM	128
#define ACTION_CALLBACK_LIST_MAX_NUM	64

/*动作回调接口*/
typedef int (*ActionCbFun)(char *, void*);


#pragma pack(1)
/*数据库信息*/
typedef struct action_db_info_s
{
	/*数据库信息*/
	char acDbPath[PATH_MAX_LEN];
	/*数据库表名*/
	char acTableName[NAME_MAX_LEN];
	/*数据库索引名*/
	char acIndexName[NAME_MAX_LEN];
}action_db_info_t;

/*动作*/
typedef struct action_info_s
{
	/*动作识别id*/
	char acActionId[ACTION_ID_MAX_LEN];
	/*设备id*/
	char acDevId[DEV_ID_MAX_LEN];
	/*动作对象*/
	char acKey[KEY_MAX_LEN];
	/*动作值的编码方式*/
	char acValueCoding[VALUE_CODING_MAX_LEN];
	/*动作值*/
	char acValue[VALUE_MAX_LEN];
	/*动作结果*/
	char acResult[STATE_MAX_LEN];
}action_info_t;

/*场景动作回调*/
typedef struct action_cb_s
{
	int iCbId;
	void *pFun;
	void *pUserData;
}action_cb_t;

typedef struct action_list_node_s
{
	link_list_piece_t stLinkPiece;
	action_info_t stActionInfo;
}action_list_node_t;
/*执行动作动作*/
typedef struct action_list_class_s
{
	/*继承链表类*/
	link_list_class_t stList;

	/*参数*/
	/*数据服务*/
	DB *pDb;
	/*ID*/
	char acId[INDEX_MAX_LEN];
	/*初始化阶段*/
	unsigned char ucInitFlag;
	
	/*方法*/
	/*private*/
	unsigned char acActionlistPrivateMethods[ACTION_LIST_PRIVATE_METHODS_MAX_LEN];

	
	/*从数据库中删除动作*/
	int (*action_del_db)(struct action_list_class_s *, action_info_t *);
	
	/*初始化动作链表，即将数据库中的信息加载到内存中*/
	int (*action_init)(struct action_list_class_s *);
	/*同步动作*/
	int (*action_sync)(struct action_list_class_s *, action_info_t *, int);
	
	/*清空动作*/
	int (*action_clear)(struct action_list_class_s *);
	/*添加动作*/
	int (*action_add)(struct action_list_class_s *, action_info_t *);
	/*删除动作*/
	int (*action_del)(struct action_list_class_s *, action_info_t *);
	/*修改动作*/
	int (*action_set)(struct action_list_class_s *, action_info_t *);
	/*获取指定动作*/
	int (*action_get)(struct action_list_class_s *, action_info_t *);
	/*获取所有动作*/
	int (*action_get_list)(struct action_list_class_s *, action_info_t *, int*);

	
	/*执行指定动作*/
	int (*action_exec)(struct action_list_class_s *, action_info_t *);
	/*执行所有动作*/
	int (*action_exec_all)(struct action_list_class_s *);

	
	/*打印所有动作，仅用于调试打印*/
	int (*action_print)(struct action_list_class_s *);
}action_list_class_t;

#pragma pack()

/*构造函数*/
action_list_class_t* new_action_list(DB *pDb, char *pcId);

/*析构函数*/
int destroy_action_list(action_list_class_t *_this);


/*动作回调函数注册*/
int action_callback_reg(int iCbid, void *pFun, void *pUserData);

/*动作回调函数注册*/
action_cb_t* action_callback_get(int iCbid);

void hy_scene_command_register(int (*func)(char *, int));
#endif /* ACTION_LIST_H */