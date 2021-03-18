/***********************************************************
*文件名     : dev_list.c
*版   本   : v1.0.0.0
*日   期   : 2019.10.28
*说   明   : 设备类
*修改记录: 
************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_msg.h"
#include "log_api.h"
#include "base_api.h"
#include "hashmap.h"
#include "db_api.h"
#include "msg_queue.h"
#include "dev_list.h"

/*数据库条件表名*/
#define DB_TABLE_NAME_DEV_LIST			"dev_list"
#define DB_TABLE_NAME_DEV_ATTR_LIST		"dev_attr_list"

/*数据库存储任务类型*/
typedef enum
{
	DEV_LIST_DB_SAVE_MSQ_TYPE_DEV_ADD = 1,
	DEV_LIST_DB_SAVE_MSQ_TYPE_ATTR_ADD = 2,
	
	
	DEV_LIST_DB_SAVE_MSQ_TYPE_NB = 0xFF
} dev_list_db_save_msg_type_en_t;
	

/*私有类型定义*/
typedef struct dev_list_private_s  
{  
	/*hash*/
	hash_map_t *pstHash;
	/*互斥锁*/
	void *mutex;
	/*设备类表处理线程*/
	thread_t Pid;
	
	/*设备上下线回调*/
	void *pDevOnlineCb;
	void *pDevOnlineUserData;
	
	/*数据库存储线程*/
	thread_t DBPid;
	/*数据库保存任务队列*/
	msg_queue_class_t *pstSaveQueue;

	/*初始化窗口*/
	unsigned char ucInitFlag;
}dev_list_private_t;

/*私有方法定义*/
typedef struct dev_list_private_methods_s  
{  
	/*向数据库中添加设备*/
	int (*dev_add_db)(
		struct dev_list_class_s *_this,
		device_t *pstDev
	);
	/*从数据库中删除设备*/
	int (*dev_del_db)(
		struct dev_list_class_s *_this,
		char *pcDevId
	);
	/*从数据库中获取指定设备*/
	int (*dev_get_db)(
		struct dev_list_class_s *_this,
		device_t *pstDev
	);
	/*获取数据库中的设备列表*/
	int (*dev_get_list_db)(
		struct dev_list_class_s *_this,
		device_t *pstDevList,
		int *piCount
	);
	/*从数据库中清空设备*/
	int (*dev_clear_db)(
		struct dev_list_class_s *_this
	);

	/*向数据库中添加设备属性*/
	int (*dev_attr_add_db)(
		struct dev_list_class_s *_this, 
		char *pcDevId,
		dev_attr_info_t *pstDevAttr
	);
	/*从数据库中删除设备属性*/
	int (*dev_attr_del_db)(
		struct dev_list_class_s *_this,
		char *pcDevId, 
		char *pcAttrKey
	);
	/*从数据库中获取指定设备属性*/
	int (*dev_attr_get_db)(
		struct dev_list_class_s *_this,
		char *pcDevId, 
		dev_attr_info_t *pstDevAttr
	);
	/*获取数据库中的设备属性列表*/
	int (*dev_attr_get_list_db)(
		struct dev_list_class_s *_this, 
		char *pcDevId,
		dev_attr_info_t *pstDevAttrList,
		int *piCount
	);
	/*从数据库中清空设备属性*/
	int (*dev_attr_clear_db)(
		struct dev_list_class_s *_this,
		char *pcDevId
	);
	/*从数据库中清空设备属性*/
	int (*dev_attr_clear_all_db)(
		struct dev_list_class_s *_this
	);
}dev_list_private_methods_t; 


/*向数据库中添加设备*/
static int 
dev_list_add_db(
	dev_list_class_t *_this,
	device_t *pstDev
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstDev);
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", _this->acDbPath);
		return GeneralErr;
	}
	
	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 1;
	db_data_entry_t astData[iRow * iRank];
	base_memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT dev_id FROM %s\
		WHERE dev_id='%s';",
		DB_TABLE_NAME_DEV_LIST,
		pstDev->acDevId);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				dev_id varchar(%d) NOT NULL,\
				dev_type varchar(%d),\
				power_type vachar(%d),\
				dev_name varchar(%d),\
				model_id varchar(%d),\
				hw_ver varchar(%d),\
				sw_ver varchar(%d),\
				hw_model varchar(%d),\
				product_key varchar(%d),\
				secret varchar(%d),\
				reg_status varchar(%d),\
				online varchar(%d),\
				report_interval varchar(%d),\
				last_time varchar(%d) \
			);", 
			DB_TABLE_NAME_DEV_LIST, 
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
		iRet = db_add(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_DEV_LIST);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
		
		iRow = 0;
	}

	char acTimeStr[DEV_LIST_TIME_MAX_LEN] = {0};
	base_time_str_get(acTimeStr);
	
	if(0 == iRow)
	{
		/*未查找到条目，使用insert语句*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN, "\
			INSERT INTO %s \
				(dev_id, \
				dev_type, \
				power_type, \
				dev_name, \
				model_id, \
				hw_ver, \
				sw_ver, \
				hw_model, \
				product_key, \
				secret, \
				reg_status, \
				online, \
				report_interval, \
				last_time) \
			VALUES \
				('%s', '%s', '%s', '%s', '%s', \
				'%s', '%s', '%s', '%s', '%s', \
				'%d', '%d', '%d', '%s');\
			", 
			DB_TABLE_NAME_DEV_LIST,
			pstDev->acDevId,
			pstDev->stDevInfo.acDevType,
			pstDev->stDevInfo.acPowerType,
			pstDev->stDevInfo.acDevName,
			pstDev->stDevInfo.acDevModel,
			pstDev->stDevInfo.acDevHWVer,
			pstDev->stDevInfo.acDevSWVer,
			pstDev->stDevInfo.acHwModel,
			pstDev->stDevInfo.acProductKey,
			pstDev->stDevInfo.acSecret,
			pstDev->stDevInfo.ucRegister,
			pstDev->stDevInfo.ucOnline,
			pstDev->stDevInfo.sReportInterval,
			acTimeStr
		);
		iRet = db_add(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("INSERT DB Table(%s) failed.\n", 
				DB_TABLE_NAME_DEV_LIST);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
	}
	else
	{
		/*查找到条目，使用update语句*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN, "\
			UPDATE %s \
			SET dev_type='%s', \
				power_type='%s', \
				dev_name='%s', \
				model_id='%s', \
				hw_ver='%s', \
				sw_ver='%s', \
				hw_model='%s', \
				product_key='%s', \
				secret='%s', \
				reg_status='%d', \
				online='%d', \
				report_interval='%d', \
				last_time='%s' \
			WHERE dev_id='%s';\
			", 
			DB_TABLE_NAME_DEV_LIST,
			pstDev->stDevInfo.acDevType,
			pstDev->stDevInfo.acPowerType,
			pstDev->stDevInfo.acDevName,
			pstDev->stDevInfo.acDevModel,
			pstDev->stDevInfo.acDevHWVer,
			pstDev->stDevInfo.acDevSWVer,
			pstDev->stDevInfo.acHwModel,
			pstDev->stDevInfo.acProductKey,
			pstDev->stDevInfo.acSecret,
			pstDev->stDevInfo.ucRegister,
			pstDev->stDevInfo.ucOnline,
			pstDev->stDevInfo.sReportInterval,
			acTimeStr,
			pstDev->acDevId);
		iRet = db_set(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("UPDATE DB Table(%s) failed.\n", 
				NoErr);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
	}
	
	/*关闭数据库*/
	db_close(&pDB);
	
	return NoErr;
}
/*从数据库中删除设备*/
static int 
dev_list_del_db(
	dev_list_class_t *_this,
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);
	
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n",
			_this->acDbPath);
		return GeneralErr;
	}

	/*删除条目*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE dev_id='%s';\
		", 
		DB_TABLE_NAME_DEV_LIST,
		pcDevId);
	iRet = db_del(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_DEV_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}

/*从数据库中获取指定设备*/
static int 
dev_list_get_db(
	dev_list_class_t *_this,
	device_t *pstDev
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstDev);

	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", 
			_this->acDbPath);
		return GeneralErr;
	}

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 12;
	db_data_entry_t astData[iRow * iRank];
	base_memset(astData, 0x0,
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT dev_type, \
				power_type, \
				dev_name, \
				model_id, \
				hw_ver, \
				sw_ver, \
				hw_model, \
				product_key, \
				secret, \
				reg_status, \
				online, \
				report_interval \
		FROM %s \
		WHERE dev_id='%s';",
		DB_TABLE_NAME_DEV_LIST,
		pstDev->acDevId);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_DEV_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	if(0 == iRow)
	{
		HY_ERROR("The ActionInfo(%s) not found.\n",
			pstDev->acDevId);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	base_strncpy(pstDev->stDevInfo.acDevType, 
		astData[0].acData, DEV_LIST_DEV_TYPE_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acPowerType, 
		astData[1].acData, DEV_LIST_DEV_TYPE_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acDevName, 
		astData[2].acData, DEV_LIST_DEV_NAME_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acDevModel,
		astData[3].acData, DEV_LIST_DEV_MODEL_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acDevHWVer,
		astData[4].acData, DEV_LIST_DEV_VER_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acDevSWVer, 
		astData[5].acData, DEV_LIST_DEV_VER_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acHwModel, 
		astData[6].acData, DEV_LIST_DEV_MODEL_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acProductKey, 
		astData[7].acData, DEV_LIST_DEV_MODEL_MAX_LEN);
	base_strncpy(pstDev->stDevInfo.acSecret, 
		astData[8].acData, DEV_LIST_DEV_SECRET_MAX_LEN);
	pstDev->stDevInfo.ucRegister = 
		(unsigned char)base_atoi(astData[9].acData);
	pstDev->stDevInfo.ucOnline =
		(unsigned char)base_atoi(astData[10].acData);
	pstDev->stDevInfo.sReportInterval =
		(unsigned short)base_atoi(astData[11].acData);
	
	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}

/*获取数据库中的设备列表*/
static int 
dev_list_get_list_db(
	dev_list_class_t *_this,
	device_t *pstDevList,
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstDevList, piCount);
	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	device_t *pstEntry = pstDevList;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", 
			_this->acDbPath);
		return GeneralErr;
	}

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = DEV_LIST_DEV_MAX_NUM;
	iRank = 13;
	db_data_entry_t *astData =
		(db_data_entry_t *)base_calloc(
			iRow * iRank, sizeof(db_data_entry_t)
		);
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return GeneralErr;
	}
	base_memset(astData, 0x0,
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT dev_id, \
				dev_type, \
				power_type, \
				dev_name, \
				model_id, \
				hw_ver, \
				sw_ver, \
				hw_model, \
				product_key, \
				secret, \
				reg_status, \
				online, \
				report_interval \
		FROM %s;",
		DB_TABLE_NAME_DEV_LIST);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_DEV_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		*piCount = 0;
		if(astData)
		{
			base_free(astData);
			astData = NULL;
		}
		return GeneralErr;
	}
	
	for(i = 0; i < iRow && iInfoCount < iInfoMax; ++i)
	{
		if(base_strcmp(astData[(iRank * i) + 0].acData, ""))
		{
			base_strncpy(pstEntry->acDevId, 
				astData[(iRank * i) + 0].acData,
				DEV_LIST_DEV_ID_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acDevType, 
				astData[(iRank * i) + 1].acData, 
				DEV_LIST_DEV_TYPE_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acPowerType, 
				astData[(iRank * i) + 2].acData, 
				DEV_LIST_DEV_TYPE_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acDevName, 
				astData[(iRank * i) + 3].acData, 
				DEV_LIST_DEV_NAME_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acDevModel, 
				astData[(iRank * i) + 4].acData, 
				DEV_LIST_DEV_MODEL_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acDevHWVer, 
				astData[(iRank * i) + 5].acData, 
				DEV_LIST_DEV_VER_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acDevSWVer, 
				astData[(iRank * i) + 6].acData,
				DEV_LIST_DEV_VER_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acHwModel, 
				astData[(iRank * i) + 7].acData, 
				DEV_LIST_DEV_MODEL_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acProductKey, 
				astData[(iRank * i) + 8].acData, 
				DEV_LIST_DEV_MODEL_MAX_LEN);
			base_strncpy(pstEntry->stDevInfo.acSecret, 
				astData[(iRank * i) + 9].acData, 
				DEV_LIST_DEV_SECRET_MAX_LEN);
			pstEntry->stDevInfo.ucRegister =
				(unsigned char)base_atoi(astData[(iRank * i) + 10].acData);
			pstEntry->stDevInfo.ucOnline = 
				(unsigned char)base_atoi(astData[(iRank * i) + 11].acData);
			pstEntry->stDevInfo.sReportInterval =  
				(unsigned short)base_atoi(astData[(iRank * i) + 12].acData);

			iInfoCount ++;
			pstEntry ++;
		}
	}
	*piCount = iInfoCount;
	/*关闭数据库*/
	db_close(&pDB);
	if(astData)
	{
		base_free(astData);
		astData = NULL;
	}
	return NoErr;
}

/*从数据库中删除设备*/
static int 
dev_list_clear_db(
	dev_list_class_t *_this
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n",
			_this->acDbPath);
		return GeneralErr;
	}

	/*删除表*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		DROP %s;\
		", 
		DB_TABLE_NAME_DEV_LIST);
	iRet = db_del(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_DEV_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}


/*向数据库中添加设备属性*/
static int 
dev_attr_list_add_db(
	dev_list_class_t *_this,
	char *pcDevId, 
	dev_attr_info_t *pstDevAttr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pstDevAttr);
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", _this->acDbPath);
		return GeneralErr;
	}
	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 1;
	db_data_entry_t astData[iRow * iRank];
	base_memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT value FROM %s \
		WHERE dev_id='%s' and key = '%s';",
		DB_TABLE_NAME_DEV_ATTR_LIST,
		pcDevId,
		pstDevAttr->acKey);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		/*表不存在，创建表*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN,
			"CREATE TABLE %s \
			(\
				dev_id varchar(%d) NOT NULL,\
				key varchar(%d),\
				value varchar(%d)\
			);", 
			DB_TABLE_NAME_DEV_ATTR_LIST, 
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN,
			DB_DATA_MAX_LEN);
		iRet = db_add(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("Create DB Table(%s) failed.\n",
				DB_TABLE_NAME_DEV_ATTR_LIST);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
		
		iRow = 0;
	}

	char acTimeStr[DEV_LIST_TIME_MAX_LEN] = {0};
	base_time_str_get(acTimeStr);

	/*更新设备的last time*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		UPDATE %s \
		SET last_time='%s' \
		WHERE dev_id='%s';\
		", 
		DB_TABLE_NAME_DEV_LIST,
		acTimeStr,
		pcDevId);
	iRet = db_set(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("UPDATE DB Table(%s) failed.\n", 
			NoErr);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}
	
	if(0 == iRow)
	{
		/*未查找到条目，使用insert语句*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN, "\
			INSERT INTO %s \
				(dev_id, \
				key, \
				value) \
			VALUES \
				('%s', '%s', '%s');\
			", 
			DB_TABLE_NAME_DEV_ATTR_LIST,
			pcDevId,
			pstDevAttr->acKey,
			pstDevAttr->acValue
		);
		iRet = db_add(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("INSERT DB Table(%s) failed.\n", 
				DB_TABLE_NAME_DEV_ATTR_LIST);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
	}
	else
	{
		/*查找到条目，使用update语句*/
		base_memset(acSql, 0x0, sizeof(acSql));
		base_snprintf(acSql, DB_SQL_MAX_LEN, "\
			UPDATE %s \
			SET value='%s' \
			WHERE dev_id='%s' and key='%s';\
			", 
			DB_TABLE_NAME_DEV_ATTR_LIST,
			pstDevAttr->acValue,
			pcDevId,
			pstDevAttr->acKey);
		iRet = db_set(pDB, acSql);
		if(NoErr != iRet)
		{
			HY_ERROR("UPDATE DB Table(%s) failed.\n", 
				NoErr);
			/*关闭数据库*/
			db_close(&pDB);
			return GeneralErr;
		}
	}
	/*关闭数据库*/
	db_close(&pDB);
	
	return NoErr;
}

/*从数据库中删除设备属性*/
static int 
dev_attr_list_del_db(
	dev_list_class_t *_this,
	char *pcDevId, 
	char *pcAttrKey
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pcAttrKey);
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n",
			_this->acDbPath);
		return GeneralErr;
	}

	/*删除条目*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE dev_id='%s' and key='%s';\
		", 
		DB_TABLE_NAME_DEV_ATTR_LIST,
		pcDevId,
		pcAttrKey);
	iRet = db_del(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_DEV_ATTR_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}

/*从数据库中获取指定设备属性*/
static int 
dev_attr_list_get_db(
	dev_list_class_t *_this, 
	char *pcDevId,
	dev_attr_info_t *pstDevAttr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pstDevAttr);
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", 
			_this->acDbPath);
		return GeneralErr;
	}

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = 1;
	iRank = 1;
	db_data_entry_t astData[iRow * iRank];
	base_memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT value\
		FROM %s \
		WHERE dev_id='%s' and key='%s';",
		DB_TABLE_NAME_DEV_ATTR_LIST,
		pcDevId,
		pstDevAttr->acKey);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_DEV_ATTR_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	if(0 == iRow)
	{
		HY_ERROR("The Dev Attr(%s, %s) not found.\n",
			pcDevId, 
			pstDevAttr->acKey);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	base_strncpy(pstDevAttr->acValue, 
		astData[0].acData, DEV_LIST_ATTR_VALUE_MAX_LEN);
	
	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}

/*获取数据库中的设备属性列表*/
static int 
dev_attr_list_get_list_db(
	dev_list_class_t *_this, 
	char *pcDevId, 
	dev_attr_info_t *pstDevAttrList, 
	int *piCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(\
		_this, pcDevId, pstDevAttrList, piCount);
	int i = 0;
	int iRet = 0;
	int iInfoCount = 0;
	int iInfoMax = *piCount;
	dev_attr_info_t *pstEntry = pstDevAttrList;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n", 
			_this->acDbPath);
		return GeneralErr;
	}

	/*获取数据库信息*/
	int iRow = 0;
	int iRank = 0;
	iRow = DEV_LIST_DEV_ATTR_MAX_NUM;
	iRank = 2;
	db_data_entry_t *astData =
		(db_data_entry_t *)base_calloc(
			iRow * iRank,
			sizeof(db_data_entry_t)
		);
	if(NULL == astData)
	{
		HY_ERROR("calloc error.\n");
		return GeneralErr;
	}
	base_memset(astData, 0x0, 
		iRow * iRank * sizeof(db_data_entry_t));
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, 
		"SELECT key, \
				value \
		FROM %s \
		WHERE dev_id='%s';",
		DB_TABLE_NAME_DEV_ATTR_LIST,
		pcDevId);
	iRet = db_get(pDB, acSql, astData, &iRow, &iRank);
	if(NoErr != iRet)
	{
		HY_ERROR("The Table(%s) not exist.\n", 
			DB_TABLE_NAME_DEV_ATTR_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		*piCount = 0;
		if(astData)
		{
			base_free(astData);
			astData = NULL;
		}
		return GeneralErr;
	}
	
	for(i = 0; i < iRow && iInfoCount < iInfoMax; ++i)
	{
		if(base_strcmp(astData[(iRank * i) + 0].acData, ""))
		{
			base_strncpy(pstEntry->acKey, 
				astData[(iRank * i) + 0].acData,
				DEV_LIST_ATTR_KEY_MAX_LEN);
			base_strncpy(pstEntry->acValue, 
				astData[(iRank * i) + 1].acData,
				DEV_LIST_ATTR_VALUE_MAX_LEN);
			

			iInfoCount ++;
			pstEntry ++;
		}
	}
	*piCount = iInfoCount;
	/*关闭数据库*/
	db_close(&pDB);
	if(astData)
	{
		base_free(astData);
		astData = NULL;
	}
	return NoErr;
}
/*从数据库中删除设备属性*/
static int 
dev_attr_list_clear_db(
	dev_list_class_t *_this,
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n",
			_this->acDbPath);
		return GeneralErr;
	}

	/*删除条目*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		DELETE FROM %s \
		WHERE dev_id='%s';\
		", 
		DB_TABLE_NAME_DEV_ATTR_LIST,
		pcDevId);
	iRet = db_del(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_DEV_ATTR_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}


/*从数据库中删除设备*/
static int 
dev_attr_list_clear_all_db(
	dev_list_class_t *_this
)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	int iRet = 0;
	char acSql[DB_SQL_MAX_LEN] = {0};
	DB *pDB = NULL;
	/*打开数据库*/
	iRet = db_open(_this->acDbPath, &pDB);
	if(NoErr != iRet)
	{
		HY_ERROR("Open DB(%s) failed.\n",
			_this->acDbPath);
		return GeneralErr;
	}

	/*删除表*/
	base_memset(acSql, 0x0, sizeof(acSql));
	base_snprintf(acSql, DB_SQL_MAX_LEN, "\
		DROP %s;\
		", 
		DB_TABLE_NAME_DEV_ATTR_LIST);
	iRet = db_del(pDB, acSql);
	if(NoErr != iRet)
	{
		HY_ERROR("DELETE DB Table(%s) failed.\n", 
			DB_TABLE_NAME_DEV_ATTR_LIST);
		/*关闭数据库*/
		db_close(&pDB);
		return GeneralErr;
	}

	/*关闭数据库*/
	db_close(&pDB);
	return NoErr;
}


/*设备查找回调*/
static int _dev_list_cmp_cb(
	void *pDevInfo1, int iDevInfoLen1, 
	void *pDevInfo2, int iDevInfoLen2
)
{
	device_t *pDev1 = (device_t *)pDevInfo1;
	device_t *pDev2 = (device_t *)pDevInfo2;

	return base_strncmp(pDev1->acDevId, 
		pDev2->acDevId, DEV_LIST_DEV_ID_MAX_LEN);
}
/*设备属性查找回调*/
static int _dev_attr_list_cmp_cb(
	void *pDevAttrInfo1, int iDevAttrInfoLen1, 
	void *pDevAttrInfo2, int iDevAttrInfoLen2
)
{
	dev_attr_info_t *pDevAttr1 =
		(dev_attr_info_t *)pDevAttrInfo1;
	dev_attr_info_t *pDevAttr2 = 
		(dev_attr_info_t *)pDevAttrInfo2;

	return base_strncmp(pDevAttr1->acKey, 
		pDevAttr2->acKey, DEV_LIST_ATTR_KEY_MAX_LEN);
}

/*设备列表加锁(阻塞)*/
static int 
dev_list_lock (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;

	return base_mutex_lock(pPrivate->mutex);
}

/*设备列表加锁(非阻塞)*/
static int
dev_list_trylock (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	return base_mutex_trylock(pPrivate->mutex);
}

/*设备列表解锁*/
static int
dev_list_unlock (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;

	return base_mutex_unlock(pPrivate->mutex);
}


/*获取设备数*/
static int 
dev_list_len (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->stLinkList.len((double_link_list_class_t*)_this);
}

/*获取设备数*/
static int 
dev_list_init (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)_this->acPrivateMethods;
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	
	int i = 0;
	int iDevCount = DEV_LIST_DEV_MAX_NUM;
	device_t *pstDevList = 
		(device_t *)base_calloc(
			iDevCount,
			sizeof(device_t)
		);
	if(NULL == pstDevList)
	{
		HY_ERROR("calloc error.\n");
		return GeneralErr;
	}

	/*开启初始化窗口*/
	pPrivate->ucInitFlag = 1;
	
	/*从数据库中获取设备列表*/
	pPrivateMethods->dev_get_list_db(
		_this,
		pstDevList,
		&iDevCount
	);
	
	for(i = 0; i < iDevCount; ++i)
	{
		HY_INFO("DevId = %s\n",
			pstDevList[i].acDevId);
		HY_INFO("DevType = %s\n", 
			pstDevList[i].stDevInfo.acDevType);
		HY_INFO("PowerType = %s\n", 
			pstDevList[i].stDevInfo.acPowerType);
		HY_INFO("DevName = %s\n", 
			pstDevList[i].stDevInfo.acDevName);
		HY_INFO("DevModel = %s\n", 
			pstDevList[i].stDevInfo.acDevModel);
		HY_INFO("DevHWVer = %s\n", 
			pstDevList[i].stDevInfo.acDevHWVer);
		HY_INFO("DevSWVer = %s\n", 
			pstDevList[i].stDevInfo.acDevSWVer);
		HY_INFO("ModuleType = %s\n", 
			pstDevList[i].stDevInfo.acHwModel);
		HY_INFO("ModuleType = %s\n", 
			pstDevList[i].stDevInfo.acProductKey);
		HY_INFO("Secret = %s\n", 
			pstDevList[i].stDevInfo.acSecret);
		HY_INFO("Register = %d\n", 
			pstDevList[i].stDevInfo.ucRegister);
		HY_INFO("Online = %d\n", 
			pstDevList[i].stDevInfo.ucOnline);
		HY_INFO("ReportInterval = %d\n", 
			pstDevList[i].stDevInfo.sReportInterval);
	}

	
	/*同步设备列表*/
	_this->sync(_this, pstDevList, iDevCount);

	/*初始化设备属性列表*/
	device_t *pstDev = NULL;
	while(NULL != (pstDev = (device_t *)(
				_this->stLinkList.next(
					(double_link_list_class_t*)_this,
					(void *)pstDev
				)
			)
		)
	)
	{
		/*从数据库中获取设备属性列表*/
		int iDevAttrCount = DEV_LIST_DEV_ATTR_MAX_NUM;
		dev_attr_info_t astDevAttrList[DEV_LIST_DEV_ATTR_MAX_NUM];
		base_memset(astDevAttrList, 
			0x0, 
			DEV_LIST_DEV_ATTR_MAX_NUM * sizeof(dev_attr_info_t)
		);
		
		pPrivateMethods->dev_attr_get_list_db(
			_this, 
			pstDev->acDevId,
			astDevAttrList, 
			&iDevAttrCount
		);
		
		for(i = 0; i < iDevAttrCount; ++i)
		{
			HY_INFO("DevId = %s\n", pstDev->acDevId);
			HY_INFO("DevAttrKey = %s\n", 
				astDevAttrList[i].acKey);
			HY_INFO("DevAttrValue = %s\n", 
				astDevAttrList[i].acValue);
		}

		/*同步设备属性列表*/
		_this->attr_sync(
			_this,
			pstDev->acDevId, 
			astDevAttrList,
			iDevAttrCount
		);
	}
	
	/*关闭初始化窗口*/
	pPrivate->ucInitFlag = 0;

	
	if(NULL != pstDevList)
	{
		base_free(pstDevList);
		pstDevList = NULL;
	}

	return NoErr;
}

/*获取设备数*/
static int 
dev_list_sync (
	dev_list_class_t *_this,
	device_t *pstDevList, 
	int iCount
	)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstDevList);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iCount);

	int i = 0;
	
	if(0 == _this->stLinkList.len(
			(double_link_list_class_t *)_this
		)
	)
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->add(_this, &pstDevList[i]);
			HY_INFO("Add Dev(%s)\n", 
				pstDevList[i].acDevId);
		}
		return NoErr;
	}
	if(0 == iCount)
	{
		_this->clear(_this, 1);
		HY_INFO("Clear Dev\n");
		return NoErr;
	}
	char *pacFoundFlag = 
		(char *)base_calloc(iCount ,sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return GeneralErr;
	}

	device_t *pstDev = NULL;
	device_t *pstDevDel = NULL;
	int iFlag = 0;
	while(NULL != (pstDev = 
			(device_t *)(
				_this->stLinkList.next(
					(double_link_list_class_t*)_this,
					(void *)pstDev
				)
			)
		)
	)
	{
		if(2 == iFlag)
		{
			/*未找到，删除*/
			_this->del(_this, pstDevDel->acDevId);
			HY_INFO("Del Del(%s)\n", 
				pstDevDel->acDevId);
			iFlag = 0;
		}
		for(i = 0; i < iCount; ++i)
		{
			if(!base_strncmp(
					pstDev->acDevId,
					pstDev[i].acDevId,
					DEV_LIST_DEV_ID_MAX_LEN
				)
			)
			{
				/*找到,判断数据是否发生改变*/
				if(base_memcmp(
						&(pstDev->stDevInfo),
						&pstDevList[i].stDevInfo,
						sizeof(dev_info_t)
					)
				)
				{
					/*信息发生改变*/
					base_memcpy(
						&(pstDev->stDevInfo), 
						&pstDevList[i].stDevInfo,
						sizeof(dev_info_t)
					);
					HY_INFO("Set Dev(%s)\n", 
						pstDevList[i].acDevId);
				}
				pacFoundFlag[i] = 1;
				iFlag = 1;
				break;
			}
		}
		if(0 == iFlag)
		{
			pstDevDel = pstDev;
			iFlag = 2;
		}
	}
	if(2 == iFlag)
	{
		/*未找到，删除*/
		_this->del(_this, pstDevDel->acDevId);
		HY_INFO("Del Del(%s)\n", 
			pstDevDel->acDevId);
		iFlag = 0;
	}

		
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->add(_this, &pstDevList[i]);
			HY_INFO("Add Dev(%s)\n", 
				pstDevList[i].acDevId);
		}
	}
	base_free(pacFoundFlag);
	
	return NoErr;
}

/*添加设备*/
static int
dev_list_add (
	dev_list_class_t *_this,
	device_t *pstDev
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstDev);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	int iRet = 0;
	device_t * pstFoundDev = 
		_this->find(_this, pstDev->acDevId);
	if(NULL == pstFoundDev)
	{
		pstDev->pstDevAttrList = new_double_link_list();
		if(NULL == pstDev->pstDevAttrList)
		{
			HY_ERROR("New DevAttrList Error.\n");
			return GeneralErr;
		}
		
		/*添加设备*/
		iRet = _this->stLinkList.tailInsert(
			(double_link_list_class_t*)_this, 
			(void *)pstDev, 
			sizeof(device_t)
		);
		if(NoErr == iRet)
		{
			/*添加哈希表*/
			pstFoundDev = 
				_this->stLinkList.find(
					(double_link_list_class_t*)_this,
					(void *)_dev_list_cmp_cb, 
					(void *)pstDev,
					sizeof(device_t));
			if(NULL != pstFoundDev)
			{
				dev_list_private_t *pPrivate = 
					(dev_list_private_t *)_this->acPrivateParam;
				char acHashKey[HASH_KEY_MAX_LEN] = {0};
				base_snprintf(
					acHashKey,
					HASH_KEY_MAX_LEN, 
					"%s",
					pstFoundDev->acDevId
				);
				
				iRet = pPrivate->pstHash->inst(
					pPrivate->pstHash,
					acHashKey, 
					(void *)&pstFoundDev,
					sizeof(device_t)
				);
				
			}
			else
			{
				destroy_double_link_list(pstDev->pstDevAttrList);
				iRet = GeneralErr;
			}
		}
	}
	else
	{
		/*修改设备*/
		base_memcpy(
			&(pstFoundDev->stDevInfo),
			&(pstDev->stDevInfo),
			sizeof(dev_info_t)
		);

		if(0 == pPrivate->ucInitFlag)
		{
			/*更新时间戳*/
			HY_DEBUG("UpData Time\n");
			base_time_get(&(pstFoundDev->stTimestamp));
		}
	}

	/*上下线处理*/
	if(0 == pPrivate->ucInitFlag && 
		0 == pstFoundDev->stDevInfo.ucOnline)
	{
		/*上线*/
		pstFoundDev->stDevInfo.ucOnline = 1;
		
		if(pPrivate->pDevOnlineCb)
		{
			((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
				pstDev, 
				pPrivate->pDevOnlineUserData
			);
		}
	}
	/*更新数据库*/
	if(NoErr == iRet)
	{
		//pPrivateMethods->dev_add_db(_this, pstFoundDev);

		dev_attr_ptr_t stDBSavePara = {0};
		stDBSavePara.pstDev = pstFoundDev;
		pPrivate->pstSaveQueue->push(
			pPrivate->pstSaveQueue,
			DEV_LIST_DB_SAVE_MSQ_TYPE_DEV_ADD,
			&stDBSavePara,
			sizeof(dev_attr_ptr_t));
	}
	
	return iRet;
}

/*查找设备*/
static device_t* 
dev_list_find (
	dev_list_class_t *_this, 
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pcDevId);
	
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;

	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(acHashKey,
		HASH_KEY_MAX_LEN, "%s", pcDevId);

	device_t * pstDev = NULL;
	void* ptr = (void*)(
			pPrivate->pstHash->find(
				pPrivate->pstHash,
				acHashKey
			)
		);
	if(NULL != ptr)
	{
		base_memcpy(&pstDev, ptr, sizeof(device_t *));
		return pstDev;
	}
	else
	{
		return NULL;
	}
	
}
/*获取设备列表*/
static int dev_get_list(
	dev_list_class_t *_this,
	device_t astDev[], 
	int *piDevCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, astDev, piDevCount);
	int iCount = 0;
	device_t *pstDev = NULL;
	while(NULL != (pstDev = 
			(device_t *)(
				_this->stLinkList.next(
					(double_link_list_class_t*)_this,
					(void *)pstDev
				)
			)
		)
	)
	{
		if(iCount <= *piDevCount)
		{
			base_memcpy(&astDev[iCount++], pstDev, sizeof(device_t));
		}
	}
	*piDevCount = iCount;
	return NoErr;
}

/*删除设备, pstDev为find接口的返回值*/
static int 
dev_list_del (
	dev_list_class_t *_this, 
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)_this->acPrivateMethods;
	
	int iRet = 0;

	device_t * pstFoundDev = 
		_this->find(_this, pcDevId);
	if(NULL == pstFoundDev)
	{
		return NoErr;
	}
	else
	{
		/*清空属性列表*/
		_this->attr_clear(_this, pcDevId, 1);

		/*销毁属性列表*/
		destroy_double_link_list(pstFoundDev->pstDevAttrList);
		
		/*删除设备*/
		_this->stLinkList.del(
			(double_link_list_class_t*)_this, 
			pstFoundDev
		);

		/*删除hash*/
		char acHashKey[HASH_KEY_MAX_LEN] = {0};
		base_snprintf(acHashKey, 
			HASH_KEY_MAX_LEN, 
			"%s", 
			pcDevId);
		iRet = pPrivate->pstHash->del(
			pPrivate->pstHash,
			acHashKey
		);

		/*更新数据库*/
		if(NoErr == iRet)
		{
			pPrivateMethods->dev_del_db(_this, pcDevId);
		}
		return iRet;
	}
	
		
	
}

/*删除设备, pstDev为find接口的返回值*/
static int 
dev_list_active_timestamp_update (
	dev_list_class_t *_this, 
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);

	device_t * pstFoundDev = 
		_this->find(_this, pcDevId);
	if(NULL == pstFoundDev)
	{
		HY_ERROR("Not found dev(%s).\n", pcDevId);
		return NotFoundErr;
	}
	else
	{
		/*清空属性列表*/
		HY_DEBUG("UpData Time\n");
		base_time_get(&(pstFoundDev->stTimestamp));
	}
	
	return NoErr;
}


/*清空设备*/
static int 
dev_list_clear (dev_list_class_t *_this, int iDbClear)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)_this->acPrivateMethods;
	
	device_t *pstDev = NULL;
	while(NULL != (pstDev = 
			(device_t *)(
				_this->stLinkList.next(
					(double_link_list_class_t*)_this,
					(void *)pstDev
				)
			)
		)
	)
	{
		/*清空属性列表*/
		_this->attr_clear(_this, pstDev->acDevId, iDbClear);
		/*销毁属性列表*/
		destroy_double_link_list(pstDev->pstDevAttrList);
		/*删除hash*/
		char acHashKey[HASH_KEY_MAX_LEN] = {0};
		base_snprintf(acHashKey, 
			HASH_KEY_MAX_LEN, 
			"%s", 
			pstDev->acDevId);
		pPrivate->pstHash->del(pPrivate->pstHash, acHashKey);
		
		/*更新数据库*/
		if(1 == iDbClear)
		{
			pPrivateMethods->dev_del_db(_this, pstDev->acDevId);
		}
		
	}

	return _this->stLinkList.clear((double_link_list_class_t*)_this);
}


/*获取设备属性数, pstDev为find接口的返回值*/
static int 
dev_list_attr_len (
	dev_list_class_t *_this, 
	char *pcDevId
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);
	device_t * pstDev = 
		_this->find(_this, pcDevId);
	return pstDev->pstDevAttrList->len(pstDev->pstDevAttrList);
}
/*添加设备, pstDev为find接口的返回值*/
static int 
dev_list_attr_sync (
	dev_list_class_t *_this,
	char *pcDevId,
	dev_attr_info_t *pstDevAttrList,
	int iCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pstDevAttrList);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iCount);
	int i = 0;
	/*查找设备*/
	device_t * pstDev = 
		_this->find(_this, pcDevId);
	if(NULL == pstDev)
	{
		HY_ERROR("No device found.\n");
		return GeneralErr;
	}
	
	if(0 == pstDev->pstDevAttrList->len(pstDev->pstDevAttrList))
	{
		/*添加*/
		for(i = 0; i < iCount; ++i)
		{
			_this->attr_add(_this, pcDevId, &pstDevAttrList[i]);
			HY_INFO("Add Dev Attr(%s, %s)\n", 
				pstDevAttrList[i].acKey,
				pstDevAttrList[i].acValue);
		}
		return NoErr;
	}
	
	if(0 == iCount)
	{
		_this->attr_clear(_this, pcDevId, 1);
		HY_INFO("Clear Dev\n");
		return NoErr;
	}
	
	char *pacFoundFlag = (char *)base_calloc(iCount ,sizeof(char));
	if(NULL == pacFoundFlag)
	{
		return GeneralErr;
	}

	dev_attr_info_t *pstDevAttr = NULL;
	dev_attr_info_t *pstDevAttrDel = NULL;
	int iFlag = 0;
	while(NULL != (pstDevAttr = 
			(dev_attr_info_t *)(
				pstDev->pstDevAttrList->next(
					pstDev->pstDevAttrList, 
					(void *)pstDevAttr
				)
			)
		)
	)
	{
		if(2 == iFlag)
		{
			/*未找到，删除*/
			_this->attr_del(_this, pcDevId, pstDevAttrDel->acKey);
			HY_INFO("Del Dev Attr(%s)\n", 
				pstDevAttrDel->acKey);
			iFlag = 0;
		}
		for(i = 0; i < iCount; ++i)
		{
			if(!base_strncmp(
					pstDevAttr->acKey, 
					pstDevAttrList[i].acKey,
					DEV_LIST_ATTR_KEY_MAX_LEN
				)
			)
			{
				/*找到,判断数据是否发生改变*/
				if(base_strncmp(
						pstDevAttr->acValue, 
						pstDevAttrList[i].acValue,
						DEV_LIST_ATTR_VALUE_MAX_LEN
					)
				)
				{
					/*信息发生改变*/
					base_strncpy(
						pstDevAttr->acValue, 
						pstDevAttrList[i].acValue,
						DEV_LIST_ATTR_VALUE_MAX_LEN
					);
					HY_INFO("Set Dev Attr(%s)\n", 
						pstDevAttr->acValue);
				}
				pacFoundFlag[i] = 1;
				iFlag = 1;
				break;
			}
		}
		if(0 == iFlag)
		{
			pstDevAttrDel = pstDevAttr;
			iFlag = 2;
		}
	}
	if(2 == iFlag)
	{
		/*未找到，删除*/
		_this->attr_del(_this, pcDevId, pstDevAttrDel->acKey);
		HY_INFO("Del Dev Attr(%s)\n", 
			pstDevAttrDel->acKey);
		iFlag = 0;
	}

		
	for(i = 0; i < iCount; ++i)
	{
		if(0 == pacFoundFlag[i])
		{
			/*未被查找过，添加*/
			_this->attr_add(_this, pcDevId, &pstDevAttrList[i]);
			HY_INFO("Add Dev Attr(%s)\n", 
				pstDevAttrList[i].acKey);
		}
	}
	base_free(pacFoundFlag);
	
	return NoErr;
}

/*添加设备, pstDev为find接口的返回值*/
static int 
dev_list_attr_add (
	dev_list_class_t *_this,
	char *pcDevId,
	dev_attr_info_t *pstDevAttr
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pstDevAttr);
	
	dev_list_private_t *pPrivate = 
			(dev_list_private_t *)_this->acPrivateParam;
	int iRet = 0;

	device_t * pstDev = NULL;
	dev_attr_info_t * pstDevAttrMem = NULL;
	dev_attr_ptr_t * pstFoundDevAttr = 
		_this->attr_find(_this, pcDevId, pstDevAttr->acKey);
	if(NULL == pstFoundDevAttr)
	{
		/*查找设备*/
		pstDev = 
			_this->find(_this, pcDevId);
		if(NULL == pstDev)
		{
			HY_ERROR("No device found.\n");
			return GeneralErr;
		}

		/*添加属性*/
		iRet = pstDev->pstDevAttrList->tailInsert(
			pstDev->pstDevAttrList, 
			(void *)pstDevAttr, 
			sizeof(dev_attr_info_t)
		);
		if(NoErr != iRet)
		{
			HY_ERROR("Device attr add failed.\n");
			return GeneralErr;
		}
		
		/*添加Hash*/
		pstDevAttrMem = 
			pstDev->pstDevAttrList->find(
				pstDev->pstDevAttrList,
				(void *)_dev_attr_list_cmp_cb, 
				(void *)pstDevAttr,
				sizeof(dev_attr_info_t));
		if(NULL != pstDevAttrMem)
		{
			dev_list_private_t *pPrivate = 
				(dev_list_private_t *)_this->acPrivateParam;
			
			char acHashKey[HASH_KEY_MAX_LEN] = {0};
			base_snprintf(acHashKey,
				HASH_KEY_MAX_LEN,
				"%s%s",
				pcDevId, 
				pstDevAttrMem->acKey
			);
			
			dev_attr_ptr_t stDevAttrPtr = {0};
			stDevAttrPtr.pstDev = pstDev;
			stDevAttrPtr.pstDevAttr = pstDevAttrMem;
			
			iRet = pPrivate->pstHash->inst(
				pPrivate->pstHash,
				acHashKey, 
				(void *)&stDevAttrPtr,
				sizeof(dev_attr_ptr_t)
			);
			
		}

	}
	else
	{
		pstDev = pstFoundDevAttr->pstDev;
		pstDevAttrMem = pstFoundDevAttr->pstDevAttr;
		/*修改属性*/
		base_memcpy(
			pstFoundDevAttr->pstDevAttr,
			pstDevAttr,
			sizeof(dev_attr_info_t)
		);
		if(0 == pPrivate->ucInitFlag)
		{
			/*更新时间戳*/
			base_time_get(&(pstDev->stTimestamp));
		}
	}

	
	/*上下线处理*/
	if(0 == pPrivate->ucInitFlag &&
		0 == pstDev->stDevInfo.ucOnline)
	{
		/*上线*/
		pstDev->stDevInfo.ucOnline = 1;
		
		if(pPrivate->pDevOnlineCb)
		{
			((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
				pstDev, 
				pPrivate->pDevOnlineUserData
			);
		}
	}
	/*更新数据库*/
	if(NoErr == iRet)
	{
		dev_attr_ptr_t stDBSavePara = {0};
		stDBSavePara.pstDev = pstDev;
		pPrivate->pstSaveQueue->push(
			pPrivate->pstSaveQueue,
			DEV_LIST_DB_SAVE_MSQ_TYPE_DEV_ADD,
			&stDBSavePara,
			sizeof(dev_attr_ptr_t));
		
		stDBSavePara.pstDev = pstDev;
		stDBSavePara.pstDevAttr = pstDevAttrMem;
		pPrivate->pstSaveQueue->push(
			pPrivate->pstSaveQueue,
			DEV_LIST_DB_SAVE_MSQ_TYPE_ATTR_ADD,
			&stDBSavePara,
			sizeof(dev_attr_ptr_t));
	}
	
	return iRet;
}

/*添加设备*/
static dev_attr_ptr_t* 
dev_list_attr_find (
	dev_list_class_t *_this, 
	char * pcDevId,
	char *pcAttrKey
)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pcDevId, pcAttrKey);
	
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;

	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(
		acHashKey,
		HASH_KEY_MAX_LEN, 
		"%s%s", 
		pcDevId, 
		pcAttrKey
	);
	dev_attr_ptr_t *p = (dev_attr_ptr_t*)(
			pPrivate->pstHash->find(
				pPrivate->pstHash,
				acHashKey
			)
		);

	return p;
}
/*获取设备属性列表*/
static int dev_list_attr_get_list(
	dev_list_class_t *_this, 
	char *pcDevId,
	dev_attr_info_t astDevAttr[], 
	int *piDevAttrCount
)
{
	PARAM_CHECK_RETURN_ERRORNO_4(_this, pcDevId, astDevAttr, piDevAttrCount);

	int iCount = 0;
	
	/*查找设备*/
	device_t * pstDev = 
		_this->find(_this, pcDevId);
	if(NULL == pstDev)
	{
		HY_ERROR("No device found.\n");
		return GeneralErr;
	}
	
	double_link_list_class_t *pAttrList = pstDev->pstDevAttrList;
	dev_attr_info_t *pstAttr = NULL;
	while(NULL != (pstAttr = 
			(dev_attr_info_t *)(
				pAttrList->next(
					pAttrList, 
					(void *)pstAttr
				)
			)
		)
	)
	{
		if(iCount <= *piDevAttrCount)
		{
			base_memcpy(&astDevAttr[iCount++], pstAttr, sizeof(dev_attr_info_t));
		}
		
	}

	*piDevAttrCount = iCount;

	return NoErr;
}

/*删除设备, pstDevAttrPtr为attr_find接口的返回值*/
static int 
dev_list_attr_del (
	dev_list_class_t *_this,
	char *pcDevId,
	char *pcAttrKey
)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pcDevId, pcAttrKey);

	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)_this->acPrivateMethods;
	
	int iRet = 0;
	dev_attr_ptr_t * pstFoundDevAttr = 
		_this->attr_find(_this, pcDevId, pcAttrKey);
	if(NULL == pstFoundDevAttr)
	{
		HY_ERROR("Not found attr.\n");
		return GeneralErr;
	}
	/*删除属性*/
	iRet = pstFoundDevAttr->pstDev->pstDevAttrList->del(
		pstFoundDevAttr->pstDev->pstDevAttrList,
		pstFoundDevAttr->pstDevAttr
	);

	if(NoErr != iRet)
	{
		HY_ERROR("Device attribute deletion failed.\n");
		return iRet;
	}

	/*删除hash*/
	char acHashKey[HASH_KEY_MAX_LEN] = {0};
	base_snprintf(acHashKey, 
		HASH_KEY_MAX_LEN, 
		"%s%s", 
		pstFoundDevAttr->pstDev->acDevId,
		pstFoundDevAttr->pstDevAttr->acKey);
	iRet = pPrivate->pstHash->del(pPrivate->pstHash, acHashKey);

	/*更新数据库*/
	if(NoErr == iRet)
	{
		pPrivateMethods->dev_attr_del_db(_this, 
			pstFoundDevAttr->pstDev->acDevId, 
			pstFoundDevAttr->pstDevAttr->acKey);
	}

	return iRet;
}

/*清空设备, pstDev为find接口的返回值*/
static int 
dev_list_attr_clear (
	dev_list_class_t *_this, 
	char * pcDevId, 
	int iDbClear
)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pcDevId);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)_this->acPrivateMethods;
	
	/*查找设备*/
	device_t * pstDev = 
		_this->find(_this, pcDevId);
	if(NULL == pstDev)
	{
		HY_ERROR("No device found.\n");
		return GeneralErr;
	}
	
	double_link_list_class_t *pAttrList = pstDev->pstDevAttrList;
	dev_attr_info_t *pstAttr = NULL;
	while(NULL != (pstAttr = 
			(dev_attr_info_t *)(
				pAttrList->next(
					pAttrList, 
					(void *)pstAttr
				)
			)
		)
	)
	{
		/*删除hash*/
		char acHashKey[HASH_KEY_MAX_LEN] = {0};
		base_snprintf(acHashKey, 
			HASH_KEY_MAX_LEN, 
			"%s%s", 
			pcDevId,
			pstAttr->acKey);
		pPrivate->pstHash->del(pPrivate->pstHash, acHashKey);
		
		if(1 == iDbClear)
		{
			pPrivateMethods->dev_attr_del_db(_this, 
				pcDevId, 
				pstAttr->acKey);
		}
		
	}

	return pAttrList->clear(pAttrList);
}



/*设备列表打印*/
static int 
dev_list_print (dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	return NoErr;
}

/*设备列表打印*/
static int 
dev_list_online_cb_reg (dev_list_class_t *_this, void *pFun, void *pUserData)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pFun);
	dev_list_private_t *pPrivate = 
			(dev_list_private_t *)_this->acPrivateParam;

	pPrivate->pDevOnlineCb = pFun;
	pPrivate->pDevOnlineUserData = pUserData;
	
	return NoErr;
}

/*数据保存*/
static void*
dev_list_db_save_exec(void *arg)
{
	dev_list_class_t *pstDevList =
		(dev_list_class_t *)arg;
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)pstDevList->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)pstDevList->acPrivateMethods;
	/*开启线程退出功能*/
	base_thread_set_cancel();


	int iMsgType = -1;
	dev_attr_ptr_t stDbSaveParav = {0};
	int iMsgLen = sizeof(dev_attr_ptr_t);
	while(1)
	{
		/*线程取消点*/
		base_thread_cancel_point();
		
		/*获取保存任务*/
		iMsgType = -1;
		while(NoErr == pPrivate->pstSaveQueue->pop(pPrivate->pstSaveQueue, &iMsgType, -1, &stDbSaveParav, &iMsgLen))
		{
			switch(iMsgType)
			{
				case DEV_LIST_DB_SAVE_MSQ_TYPE_DEV_ADD:
					{
						HY_DEBUG("dev_add_db\n");
						pPrivateMethods->dev_add_db(
							pstDevList, 
							stDbSaveParav.pstDev);
					}
					break;
				case DEV_LIST_DB_SAVE_MSQ_TYPE_ATTR_ADD:
					{
						HY_DEBUG("dev_attr_add_db\n");
						pPrivateMethods->dev_attr_add_db(
							pstDevList,
							stDbSaveParav.pstDev->acDevId,
							stDbSaveParav.pstDevAttr);
					}
					break;
				default:
				{
					HY_ERROR("The Msg Type is not found.\n");
				}
				break;
			}

			base_memset(&stDbSaveParav, 0x0, sizeof(dev_attr_ptr_t));
			iMsgLen = sizeof(dev_attr_ptr_t);
			iMsgType = -1;
		}

	}

	return NULL;
}

static void*
dev_list_exec(void *arg)
{
	base_timeval_t stNew;
	
	dev_list_class_t *pstDevList =
		(dev_list_class_t *)arg;
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)pstDevList->acPrivateParam;
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)pstDevList->acPrivateMethods;
	/*开启线程退出功能*/
	base_thread_set_cancel();

#if 0
	/*等待20S后，开始任务*/
	base_delay_s(20);

	/*首次任务，进行上下线处理*/
	pstDevList->lock(pstDevList);
	device_t *pstDev = NULL;
	while(NULL != (pstDev = 
			(device_t *)(
				pstDevList->stLinkList.next(
					(double_link_list_class_t*)pstDevList,
					(void *)pstDev
				)
			)
		)
	)
	{
		if(pPrivate->pDevOnlineCb)
		{
			((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
				pstDev, 
				pPrivate->pDevOnlineUserData
			);
		}
	}
	pstDevList->unlock(pstDevList);
#endif
	
	while(1)
	{
		/*线程取消点*/
		base_thread_cancel_point();
		/*获取当前时间*/
		base_time_get(&stNew);

		/*设备在线状态监测*/
		pstDevList->lock(pstDevList);
		device_t *pstDev = NULL;
		while(NULL != (pstDev = 
				(device_t *)(
					pstDevList->stLinkList.next(
						(double_link_list_class_t*)pstDevList,
						(void *)pstDev
					)
				)
			)
		)
		{
			if(!base_strcmp(pstDev->stDevInfo.acPowerType,
				DEV_POWER_TYPR_KINETIC)
			)
			{
				/*动能设备*/
				
			}
			else if(!base_strcmp(pstDev->stDevInfo.acPowerType, 
				DEV_POWER_TYPR_BATTERY)
			)
			{
				/*电池类设备*/
				if(stNew.uiSec - pstDev->stTimestamp.uiSec > 
					DEV_BATTERY_HEART_BEAT_CYCLE * DEV_HEART_BEAT_TIMEOUT_TIME
				)
				{
					pstDev->stDevInfo.ucOnline = 0;
					pPrivateMethods->dev_add_db(pstDevList, pstDev);
					/*离线*/
					if(pPrivate->pDevOnlineCb)
					{
						((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
							pstDev, 
							pPrivate->pDevOnlineUserData
						);
					}
				}
				else
				{
					if(0 == pstDev->stDevInfo.ucOnline)
					{
						pstDev->stDevInfo.ucOnline = 1;
						pPrivateMethods->dev_add_db(pstDevList, pstDev);
						/*上线*/
						if(pPrivate->pDevOnlineCb)
						{
							((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
								pstDev, 
								pPrivate->pDevOnlineUserData
							);
						}
					}
				}
			}
			else if(!base_strcmp(pstDev->stDevInfo.acPowerType, 
				DEV_POWER_TYPR_OFTEN)
			)
			{
				/*常供电设备*/
				if(stNew.uiSec - pstDev->stTimestamp.uiSec > 
					DEV_OFTEN_HEART_BEAT_CYCLE * DEV_HEART_BEAT_TIMEOUT_TIME
				)
				{
					pstDev->stDevInfo.ucOnline = 0;
					pPrivateMethods->dev_add_db(pstDevList, pstDev);
					/*离线*/
					if(pPrivate->pDevOnlineCb)
					{
						((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
							pstDev, 
							pPrivate->pDevOnlineUserData
						);
					}
				}
				else
				{
					if(0 == pstDev->stDevInfo.ucOnline)
					{
						pstDev->stDevInfo.ucOnline = 1;
						pPrivateMethods->dev_add_db(pstDevList, pstDev);
						/*上线*/
						if(pPrivate->pDevOnlineCb)
						{
							((DevOnlineReportFun)pPrivate->pDevOnlineCb)(
								pstDev, 
								pPrivate->pDevOnlineUserData
							);
						}
					}
				}
			}
		}
		pstDevList->unlock(pstDevList);
		base_delay_s(5 * 60);
	}	
	return NULL;
}


/*构造函数*/
dev_list_class_t *new_dev_list(char *acDbPath)
{
	
	dev_list_class_t* pNew = 
		(dev_list_class_t *)base_calloc(
			1,
			sizeof(dev_list_class_t)
		);
	if(NULL == pNew)
	{
		HY_ERROR("Malloc error.\n");
		
		return NULL;
	}

	/*调用父类的构造函数*/
	double_link_list_class_t *pstLink = 
		new_double_link_list();
	if(NULL == pstLink)
	{
		HY_ERROR("New DevList Error.\n");
		base_free(pNew);
		return NULL;
	}

	/*将父类的数据拷贝到子类中*/
	base_memcpy(pNew, 
		pstLink, 
		sizeof(double_link_list_class_t)
	);
	/*pstLink里的某些成员也是动态分配的，
	*而这些内存是要使用的，所以
	*此处指释放pstLink，而不释放成员的分配
	*/
	base_free(pstLink);

	/*参数初始化*/
	base_strncpy(pNew->acDbPath, 
		acDbPath, DEV_DB_PATH_MAX_LEN);
	
	/*私有变量初始化*/
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)pNew->acPrivateParam;
	base_memset(pPrivate, 
		0x0, sizeof(dev_list_private_t));

	/*初始化hash*/
	pPrivate->pstHash = new_hash_map(0, 1);
	if(NULL == pPrivate->pstHash)
	{
		HY_ERROR("Failed to create a hash table.\n");
		base_free(pNew);
		return NULL;
	}

	/*初始化互斥锁*/
	pPrivate->mutex = base_mutex_lock_create();
	if(NULL == pPrivate->mutex)
	{
		HY_ERROR("Failed to create a hash table.\n");
		destroy_hash_map(pPrivate->pstHash);
		base_free(pNew);
		return NULL;
	}

	/*初始化数据库存储队列*/
	pPrivate->pstSaveQueue = new_msg_queue();
	if(NULL == pPrivate->pstSaveQueue)
	{
		HY_ERROR("Failed to create a queue.\n");
		destroy_hash_map(pPrivate->pstHash);
		base_mutex_lock_destroy(pPrivate->mutex);
		base_free(pNew);
		return NULL;
	}
	/*创建处理线程*/
	if(0 != base_thread_create(
		&pPrivate->Pid, 
		dev_list_exec, 
		(void *)pNew)
	)
	{
		HY_ERROR("thread_create Error.\n");
		destroy_hash_map(pPrivate->pstHash);
		base_mutex_lock_destroy(pPrivate->mutex);
		destroy_msg_queue(pPrivate->pstSaveQueue);
		base_free(pNew);
        return NULL;
	}

	if(0 != base_thread_create(
		&pPrivate->DBPid, 
		dev_list_db_save_exec, 
		(void *)pNew)
	)
	{
		HY_ERROR("thread_create Error.\n");
		base_thread_cancel(pPrivate->Pid);
		base_delay_s(1);
		
		destroy_hash_map(pPrivate->pstHash);
		base_mutex_lock_destroy(pPrivate->mutex);
		destroy_msg_queue(pPrivate->pstSaveQueue);
		base_free(pNew);
        return NULL;
	}
	
	pNew->lock = dev_list_lock;
	pNew->trylock = dev_list_trylock;
	pNew->unlock = dev_list_unlock;
	pNew->init = dev_list_init;
	pNew->sync = dev_list_sync;
	pNew->len = dev_list_len;
	pNew->add = dev_list_add;
	pNew->find = dev_list_find;
	pNew->list = dev_get_list;
	pNew->del = dev_list_del;
	pNew->clear = dev_list_clear;
	pNew->attr_sync = dev_list_attr_sync;
	pNew->attr_len = dev_list_attr_len;
	pNew->attr_add = dev_list_attr_add;
	pNew->attr_find = dev_list_attr_find;
	pNew->attr_list = dev_list_attr_get_list;
	pNew->attr_del = dev_list_attr_del;
	pNew->attr_clear = dev_list_attr_clear;
	pNew->print = dev_list_print;
	pNew->online_cb_reg = dev_list_online_cb_reg;
	pNew->active_timestamp = dev_list_active_timestamp_update;
	
	dev_list_private_methods_t *pPrivateMethods = 
		(dev_list_private_methods_t *)pNew->acPrivateMethods;
	pPrivateMethods->dev_add_db = 
		dev_list_add_db;
	pPrivateMethods->dev_del_db = 
		dev_list_del_db;
	pPrivateMethods->dev_get_db = 
		dev_list_get_db;
	pPrivateMethods->dev_get_list_db = 
		dev_list_get_list_db;
	pPrivateMethods->dev_clear_db = 
		dev_list_clear_db;
	pPrivateMethods->dev_attr_add_db = 
		dev_attr_list_add_db;
	pPrivateMethods->dev_attr_del_db = 
		dev_attr_list_del_db;
	pPrivateMethods->dev_attr_get_db = 
		dev_attr_list_get_db;
	pPrivateMethods->dev_attr_get_list_db =
		dev_attr_list_get_list_db;
	pPrivateMethods->dev_attr_clear_db = 
		dev_attr_list_clear_db;
	pPrivateMethods->dev_attr_clear_all_db = 
		dev_attr_list_clear_all_db;

	/*将数据库的数据加载到内存中*/
	pNew->init(pNew);
		
	return pNew;
}

/*析构函数*/
int destroy_dev_list(dev_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	dev_list_private_t *pPrivate = 
		(dev_list_private_t *)_this->acPrivateParam;
	/*停止线程*/
	base_thread_cancel(pPrivate->Pid);
	base_delay_s(1);

	base_thread_cancel(pPrivate->DBPid);
	base_delay_s(1);
	
	if(_this->len(_this))
	{
		/*清空链表*/
		_this->clear(_this, 0);
	}

	/*销毁hash*/
	destroy_hash_map(pPrivate->pstHash);

	/*销毁互斥锁*/
	base_mutex_lock_destroy(pPrivate->mutex);
	
	base_free(_this);
	_this = NULL;
	return NoErr;
}

