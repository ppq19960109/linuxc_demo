/***********************************************************
*文件名    : condition_list.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.04
*说   明   : 条件接口，条件分两种，时间条件以及时间条件
*修改记录: 
************************************************************/
#ifndef CONDITION_LIST_H
#define CONDITION_LIST_H

#include "scene_link_list.h"
#include "len.h"
#include "ptr_hash.h"
#include "db_api.h" 

#define COND_LIST_PRIVATE_METHODS_MAX_LEN	128
#define COND_MAX_NUM	128

/*条件类型*/
#define COND_TIME		"Time"
#define COND_EVENT		"Event"
/*逻辑关系*/
#define LOGIC_AND		"And"
#define LOGIC_OR		"Or"
#if 0
/*天定义*/
#define DAY_1			"1"
#define DAY_2			"2"
#define DAY_3			"3"
#define DAY_4			"4"
#define DAY_5			"5"
#define DAY_6			"6"
#define DAY_7			"7"
#define DAY_8			"8"
#define DAY_9			"9"
#define DAY_10			"10"
#define DAY_11			"11"
#define DAY_12			"12"
#define DAY_13			"13"
#define DAY_14			"14"
#define DAY_15			"15"
#define DAY_16			"16"
#define DAY_17			"17"
#define DAY_18			"18"
#define DAY_19			"19"
#define DAY_20			"20"
#define DAY_21			"21"
#define DAY_22			"22"
#define DAY_23			"23"
#define DAY_24			"24"
#define DAY_25			"25"
#define DAY_26			"26"
#define DAY_27			"27"
#define DAY_28			"28"
#define DAY_29			"29"
#define DAY_30			"30"
#define DAY_31			"31"
static char* apcDay[] = {DAY_1,  DAY_2,  DAY_3,  DAY_4,  DAY_5,  DAY_6,  DAY_7,  DAY_8,  DAY_9, DAY_10,
						DAY_11, DAY_12, DAY_13, DAY_14, DAY_15, DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
						DAY_21, DAY_22, DAY_23, DAY_24, DAY_25, DAY_26, DAY_27, DAY_28, DAY_29, DAY_30, 
						DAY_31};
#endif
/*星期定义*/
#define WEEK_MON		"Mon"
#define WEEK_TUE		"Tue"
#define WEEK_WED		"Wed"
#define WEEK_THU		"Thu"
#define WEEK_FIR		"Fri"
#define WEEK_SAT		"Sat"
#define WEEK_SUN		"Sun"

#if 0
/*月定义*/
#define MONTH_JAN			"Jan"
#define MONTH_FEB			"Feb"
#define MONTH_MAR			"Mar"
#define MONTH_APR			"Apr"
#define MONTH_MAY			"May"
#define MONTH_JUN			"Jun"
#define MONTH_JUL			"Jul"
#define MONTH_AUG			"Aug"
#define MONTH_SEP			"Sep"
#define MONTH_OCT			"Oct"
#define MONTH_NOV			"Nov"
#define MONTH_DEC			"Dec"
static char* apcMonth[] = {MONTH_JAN, MONTH_FEB, MONTH_MAR, MONTH_APR, MONTH_MAY, MONTH_JUN, MONTH_JUL, MONTH_AUG, MONTH_SEP, MONTH_OCT, MONTH_NOV, MONTH_DEC};
#endif
/*激活条件*/
#define ACTIVE_EQU		"Equ"
#define ACTIVE_GTR		"Gtr"
#define ACTIVE_LSS		"Lss"
#define ACTIVE_NOT		"Not"
#define ACTIVE_NULL		"NULL"
//static char* apcActive[] = {ACTIVE_EQU, ACTIVE_GTR, ACTIVE_LSS};

#pragma pack(1)
/*条件列表信息*/
typedef struct cond_info_s
{
	/*该条件列表属于哪个场景ID*/
	char acId[INDEX_MAX_LEN];
	/*该条件列表属于哪个条件逻辑*/
	char acLogic[LOGIC_MAX_LEN];
	/*该条件列表的条件类型是什么*/
	char acCondType[COND_TYPE_MAX_LEN];
}cond_info_t;

/*事件*/
typedef struct event_s
{
	/*设备id*/
	char acDevId[DEV_ID_MAX_LEN];
	/*事件对象*/
	char acKey[KEY_MAX_LEN];
	/*事件值*/
	char acValue[VALUE_MAX_LEN];
}event_t;

/*时间条件*/
/*acTimeKey确定一个条件*/
/*开始时间与截止时间相同则为时间点*/
/*开始时间与截止时间不相同则为时间段*/
/*时间段不允许跨天*/
/*与条件集合里时间条件只允许一条*/
typedef struct time_cond_s
{
	char acTimeKey[KEY_MAX_LEN];
	/*触发类型*/
	char acTriggerType[TRIGGER_TYPE_MAX_LEN];
	/*触发间隔*/
	char acTriggerInterval[TRIGGER_INTERVAL_MAX_LEN];
	int iTriggerInterval;
	
	/*开始时间*/
	char acStartHour[HOUR_MAX_LEN];
	char acStartMinu[MINU_MAX_LEN];
	/*截止时间*/
	char acEndHour[HOUR_MAX_LEN];
	char acEndMinu[MINU_MAX_LEN];
#if 0
	/*天*/
	char acDay[DAY_MAX_LEN];
#endif
	/*星期*/
	char acWeek[WEEK_MAX_LEN];
#if 0
	/*月*/
	char acMonth[MONTH_MAX_LEN];
	/*年*/
	char acYear[YEAR_MAX_LEN];
#endif
	/*是否重复*/
	char acRepeat[STATE_MAX_LEN];
	/*是否失效，当时间条件不重复时，满足条件后则失效*/
	char acLose[STATE_MAX_LEN];
}time_cond_t;

/*事件条件*/
/*acDevId acKey共同确定一个条件*/
typedef struct event_cond_s
{
	/*设备id*/
	char acDevId[DEV_ID_MAX_LEN];
	/*事件对象*/
	char acKey[KEY_MAX_LEN];
	/*触发类型*/
	char acTriggerType[TRIGGER_TYPE_MAX_LEN];
	/*持续时间*/
	char acContinueTime[CONTINUE_TIME_MAX_LEN];
	/*事件激活值*/
	char acValue[VALUE_MAX_LEN];
	/*事件激活条件，等于、大于或小于*/
	char acActive[ACTIVE_MAX_LEN];

	/*事件状态记录*/
	char acLastValue[VALUE_MAX_LEN];
}event_cond_t;


/*时间条件结点*/
typedef struct time_cond_node_s
{
	struct time_cond_node_s *next;
	struct time_cond_node_s *prev;
	int iCondIndex;
	time_cond_t stCond;
}time_cond_node_t;
/*事件条件结点*/
typedef struct event_cond_node_s
{
	struct event_cond_node_s *next;
	struct event_cond_node_s *prev;
	int iCondIndex;
	event_cond_t stCond;
}event_cond_node_t;


/*条件集合*/
/*该集合最多可容纳32个条件，集合内的每个条件由一个ID，从0到31*/
/*uiTrue的bit0用于表示ID为0的条件的真假，bit1-bit31类似*/
typedef struct cond_set_s
{
	/*每一个bit，代表一个条件的真假*/
	unsigned int uiCondTrue;
	/*该集合中条件的个数*/
	unsigned char ucCondCount; 
	void *pstCond;
}cond_set_t;

/*条件集合结点*/
typedef struct cond_list_node_s
{
	link_list_piece_t stLinkPiece;
	int iCondSetIndex;
	cond_set_t stCondSet;
}cond_list_node_t;

/*条件列表*/
/*该列表最多支持32个条件集合，而每个条件集合又支持32个条件，即每个条件列表支持最多1024个条件*/
/*每个条件集合*/
/*uiTrue*/
typedef struct cond_list_class_s
{
	/*继承链表类*/
	link_list_class_t stList;

	/*参数*/
	/*该条件链表的整体真假*/
	unsigned int uiCondSetTrue;
	/*条件总数*/
	int iCount;
	/*数据服务*/
	DB *pDb;
	/*条件列表信息*/
	cond_info_t stCondInfo;
	/*初始化阶段*/
	unsigned char ucInitFlag;
	
	/*方法*/
	/*private*/
	unsigned char acCondlistPrivateMethods[COND_LIST_PRIVATE_METHODS_MAX_LEN];

	/*获取数据库中的条件列表*/
	int (*cond_add_db)(struct cond_list_class_s *, void *);
	int (*cond_get_list_db)(struct cond_list_class_s *, void *, int*);
	int (*cond_del_db)(struct cond_list_class_s *, void *);
	int (*cond_set_db)(struct cond_list_class_s *, void *);
	int (*time_cond_time_lose_set_db)(struct cond_list_class_s *,	time_cond_t *);
	
	/*获取条件个数*/
	int (*cond_size)(struct cond_list_class_s *);

	
	/*清空条件*/
	int (*cond_clear)(struct cond_list_class_s *);
	/*添加条件*/
	int (*cond_add)(struct cond_list_class_s *, void *, ptr_info_t *);
	/*删除条件*/
	int (*cond_del)(struct cond_list_class_s *, ptr_info_t *);
	/*修改条件*/
	int (*cond_set)(struct cond_list_class_s *, void *, ptr_info_t *);
	/*获取指定条件*/
	int (*cond_get)(struct cond_list_class_s *, void *, ptr_info_t *);
	/*获取所有条件*/
	int (*cond_get_list)(struct cond_list_class_s *, void *, int*);

	
	/*判断条件的真假(该接口仅对时间条件有效)*/
	int (*cond_true_all)(struct cond_list_class_s *);

	
	/*打印所有动作，仅用于调试打印*/
	int (*cond_print)(struct cond_list_class_s *);
}cond_list_class_t;
#pragma pack()

/*构造函数*/
cond_list_class_t* new_cond_list(DB *pDb, cond_info_t *pstCondInfo);

/*析构函数*/
int destroy_cond_list(cond_list_class_t *_this);


#endif /* CONDITION_LIST_H */