/***********************************************************
*文件名     : dev_list.h
*版   本   : v1.0.0.0
*日   期   : 2019.10.28
*说   明   : 设备类
*修改记录: 
************************************************************/


#ifndef DEV_LIST_H
#define DEV_LIST_H

#include "link_list.h"

#define DEV_LIST_DEV_MAX_NUM					256
#define DEV_LIST_DEV_ATTR_MAX_NUM				32


#define DEV_LIST_PRIVATE_PARAM_MAX_LEN			128
#define DEV_LIST_PRIVATE_METHODS_MAX_LEN		128

#define DEV_DB_PATH_MAX_LEN						128
#define DEV_LIST_DEV_ID_MAX_LEN					32
#define DEV_LIST_DEV_TYPE_MAX_LEN				16
#define DEV_LIST_DEV_MODEL_MAX_LEN				16
#define DEV_LIST_DEV_NAME_MAX_LEN				64
#define DEV_LIST_DEV_VER_MAX_LEN				16
#define DEV_LIST_DEV_SECRET_MAX_LEN				64

#define DEV_LIST_ATTR_KEY_MAX_LEN				32
#define DEV_LIST_ATTR_VALUE_MAX_LEN				128

#define DEV_LIST_TIME_MAX_LEN					32

/*设备心跳周期*/
#define DEV_OFTEN_HEART_BEAT_CYCLE				(5 * 60)
#define DEV_BATTERY_HEART_BEAT_CYCLE			(30 * 60)
#define DEV_KINETIC_HEART_BEAT_CYCLE			(-1)

/*设备心跳超时*/
#define DEV_HEART_BEAT_TIMEOUT_TIME				3

/*设备类型*/
#define DEV_TYPR_ZIGBEE							"zigbee"
#define DEV_TYPR_WIFI							"wifi"
#define DEV_TYPR_RF_433							"433"
#define DEV_TYPR_ZIGBEE							"zigbee"
#define DEV_TYPR_BLUETOOTH						"bht"
#define DEV_TYPR_LORA							"lora"

/*供电类型*/
#define DEV_POWER_TYPR_KINETIC					"kinetic"/*动能设备*/
#define DEV_POWER_TYPR_BATTERY					"battery"/*电池类设备*/
#define DEV_POWER_TYPR_OFTEN					"often"/*常供电设备*/

typedef struct dev_info_s
{
	/*设备类型. zigbee\wifi\433\bht\lora*/
	char acDevType[DEV_LIST_DEV_TYPE_MAX_LEN];
	/*供电类型，kinetic\battery\often*/
	char acPowerType[DEV_LIST_DEV_TYPE_MAX_LEN];
	/*设备型号*/
	char acDevModel[DEV_LIST_DEV_MODEL_MAX_LEN];
	/*设备名称*/
	char acDevName[DEV_LIST_DEV_NAME_MAX_LEN];
	/*设备硬件版本信息*/
	char acDevHWVer[DEV_LIST_DEV_VER_MAX_LEN];
	/*设备软件版本信息*/
	char acDevSWVer[DEV_LIST_DEV_VER_MAX_LEN];
	/*硬件型号*/
	char acHwModel[DEV_LIST_DEV_MODEL_MAX_LEN];
	/*云端型号*/
	char acProductKey[DEV_LIST_DEV_MODEL_MAX_LEN];
	/*秘钥信息*/
	char acSecret[DEV_LIST_DEV_SECRET_MAX_LEN];
	/*注册状态*/
	unsigned char ucRegister;
	/*在线状态*/
	unsigned char ucOnline;
	/*设备上报最小时间间隔，单位S*/
	short sReportInterval;
}dev_info_t;

typedef struct dev_attr_info_s
{
	/*Dev属性key,为，具有唯一性*/
	char acKey[DEV_LIST_ATTR_KEY_MAX_LEN];
	char acValue[DEV_LIST_ATTR_VALUE_MAX_LEN];

	/*上报时间戳*/
	base_timeval_t stReportTimestamp;
}dev_attr_info_t;

typedef struct device_s
{
	/*设备ID，具有唯一性*/
	char acDevId[DEV_LIST_DEV_ID_MAX_LEN];
	
	/*活跃时间戳*/
	base_timeval_t stTimestamp;
	
	/*设备信息*/
	dev_info_t stDevInfo;

	/*设备属性列表*/
	double_link_list_class_t *pstDevAttrList;
}device_t;

typedef struct dev_attr_ptr_s
{
	device_t *pstDev;
	dev_attr_info_t *pstDevAttr;
}dev_attr_ptr_t;

/*设备上下线回调*/
typedef int (*DevOnlineReportFun)(device_t *pDev, void *pUserData);


/*双向链表类*/
typedef struct dev_list_class_s
{
	/*父类*/
	double_link_list_class_t stLinkList;

	char acDbPath[DEV_DB_PATH_MAX_LEN];
	
	/*private*/
	unsigned char acPrivateParam[DEV_LIST_PRIVATE_PARAM_MAX_LEN];
	/*私有方法*/
	unsigned char acPrivateMethods[DEV_LIST_PRIVATE_METHODS_MAX_LEN];
	
	/*设备列表加锁(阻塞)*/
	int (*lock) (struct dev_list_class_s *_this);
	/*设备列表加锁(非阻塞)*/
	int (*trylock) (struct dev_list_class_s *_this);
	/*设备列表解锁*/
	int (*unlock) (struct dev_list_class_s *_this);

	/*设备列表初始化*/
	int (*init) (struct dev_list_class_s *_this);
	/*设备列表同步*/
	int (*sync) (struct dev_list_class_s *_this, device_t *pstDevList, int iCount);
	
	/*获取设备数*/
	int (*len) (struct dev_list_class_s *_this);
	/*添加设备*/
	int (*add) (struct dev_list_class_s *_this, device_t *pstDev);
	/*查找设备*/
	device_t* (*find) (struct dev_list_class_s *_this, char *pcDevId);
	/*获取设备列表*/
	int (*list) (struct dev_list_class_s *_this, device_t astDev[], int *piDevCount);
	/*删除设备, pstDev为find接口的返回值*/
	int (*del) (struct dev_list_class_s *_this, char *pcDevId);
	/*清空设备*/
	int (*clear) (struct dev_list_class_s *_this, int iDbClear);

	/*活跃时间戳更新*/
	int (*active_timestamp) (struct dev_list_class_s *_this, char *pcDevId);
	
	int	(*attr_sync) (
		struct dev_list_class_s *_this,
		char *pcDevId,
		dev_attr_info_t *pstDevAttrList,
		int iCount
	);

	
	/*获取设备属性数*/
	int (*attr_len) (struct dev_list_class_s *_this, char *pcDevId);
	/*添加设置设备属性*/
	int (*attr_add) (
		struct dev_list_class_s *_this, 
		char *pcDevId,
		dev_attr_info_t *pstDevAttr
	);
	/*查找设备属性*/
	dev_attr_ptr_t* (*attr_find) (
		struct dev_list_class_s *_this,
		char *pcDevId,
		char *pcAttrKey
	);
	/*获取设备属性列表*/
	int (*attr_list) (struct dev_list_class_s *_this, 
		char *pcDevId,
		dev_attr_info_t astDevAttr[], 
		int *piDevAttrCount
	);
	/*删除设备属性*/
	int (*attr_del) (
		struct dev_list_class_s *_this,
		char *pcDevId,
		char *pcAttrKey
	);
	/*清空设备属性*/
	int (*attr_clear) (struct dev_list_class_s *_this, char *pcDevId, int iDbClear);

	/*设备列表打印*/
	int (*print) (struct dev_list_class_s *_this);

	/*注册设备上下线上报*/
	int (*online_cb_reg) (struct dev_list_class_s *_this, void *pFun, void *pUserData);
	
}dev_list_class_t;

/*构造函数*/
dev_list_class_t *new_dev_list(char *acDbPath);

/*析构函数*/
int destroy_dev_list(dev_list_class_t *_this);


#endif /* DEV_LIST_H */
