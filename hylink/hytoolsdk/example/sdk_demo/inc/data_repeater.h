/***********************************************************
*文件名     : data_repeater.h
*版   本   : v1.0.0.0
*日   期   : 2018.05.31
*说   明   : 数据重发器
*修改记录: 
************************************************************/

#ifndef DATA_REPEATER_H
#define DATA_REPEATER_H

#define DATA_REPEATER_PRIVATE_METHODS_MAX_LEN	128



typedef struct data_repeater_s
{	
	/*private*/
	unsigned char acPrivateParam[DATA_REPEATER_PRIVATE_METHODS_MAX_LEN];
	
	/**************************************************************
	*描述: 设置发送数据超时时间
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @int iTimeOut: 超时时间，单位ms
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*timeout)(struct data_repeater_s *_this, int iTimeOut);
	
	/**************************************************************
	*描述: 设置最大重发次数
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @int iMaxResend: 最大重发次数
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*maxResend)(struct data_repeater_s *_this, int iMaxResend);
	
	/**************************************************************
	*描述: 注册数据重发接口
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @void *pResendCb: 数据重发回调，当发送数据超时后，将调用该回调，重发数据
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	
	int (*resendCbReg)(struct data_repeater_s *_this, void *pResendCb, void *pUserData);
	/**************************************************************
	*描述: 注册发送数据与回应数据配对回调接口
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @void *pRespondMatchingCb: 发送与回应报文匹配回调，
	*		该回调中实现根据回应报文匹配其发送报文
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*respondMatchingCbReg)(struct data_repeater_s *_this, void *pRespondMatchingCb, void *pUserData);
		

	/**************************************************************
	*描述: 向重发器中放入发送数据
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @int iSendFlag: 0表示异步接口发出的数据, 
	*		1表示同步接口发出的数据, 2表示无需进行回复匹配
	*	   @void *pData: 发送数据
	*	   @int iDataLen: 数据长度
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*sendPush)(struct data_repeater_s *_this, int iSendFlag, void *pData, int iDataLen);

	/**************************************************************
	*描述: 向重发器中放入回应数据，用于匹配发送数据
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @void *pRecvData: 回应数据
	*	   @int iRecvDataLen: 回应数据长度
	*	   @int pSendData: 用于返回匹配到的发送报文
	*	   @int piSendDataLen: 传入发送报文缓存最大长度，传出发送报文的真实长度
	*返回: 成功返回与回应数据匹配的发送数据，失败返回-1；
	*注意: 如果pSendData域中有需要free的数据，需要自己释放
	**************************************************************/
	int (*respondPush)(struct data_repeater_s *_this, void *pRecvData, int iRecvDataLen, void *pSendData, int *piSendDataLen);

	/**************************************************************
	*描述: 注册数据域释放回调
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @void *pFreeDataCb: 数据源释放回调
	*返回: 成功返回0，失败返回-1；
	*说明: 如果数据域中有某一部分数据是动态分配的， 则需要注册释放回调接口
	**************************************************************/
	int (*freeDataCbReg) (struct data_repeater_s* _this, void* pFreeDataCb);

	/**************************************************************
	*描述: 打印数据
	*参数: struct data_repeater_s *_this: 重发器类指针
	*	   @void *pPrintDataCb: 数据源打印回调
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*print)(struct data_repeater_s* _this, void* pPrintDataCb);
}data_repeater_t;


/*构造函数*/
data_repeater_t *new_data_repeater(int iTimeOut, int iMaxResend);

/*析构函数*/
int destroy_data_repeater(data_repeater_t *_this);

/*相关函数指针类型定义*/
typedef int (*ResendFun)(data_repeater_t *_this, void *pData, int iDataLen, void *pUserData);
typedef int (*DataMatchingFun)(data_repeater_t *_this, void *pSendData, int iSendDataLen, void *pRecvData, int iRecvDataLen, void *pUserData);
typedef int (*PrintDataFun)(void *pData, int iDataLen);
typedef int (*FreeDataFun)(void *pData, int iDataLen);

#endif /* DATA_REPEATER_H */
