/***********************************************************
*文件名     : session_ser.h
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 会话服务
*修改记录: 
************************************************************/


#ifndef SESSION_SER_H
#define SESSION_SER_H

#include "link_list.h"
#include "data_repeater.h"

#define SESSION_TYPE_MAX_NUM				16
#define SESSION_MAX_NUM						32
#define SESSION_SER_PRIVATE_PARAM_MAX_LEN	128
#define SESSION_DATA_MAX_LEN				2048
#define SESSION_ADDR_MAX_LEN				128
#define SESSION_PUSH_MAX_NUM				3
#define SESSION_WRITE_TIEMOUT_MS			2

/*Gpio方向*/
typedef enum
{
	SESSION_GPIO_DIRECTION_IN = 0,
	SESSION_GPIO_DIRECTION_OUT = 1,
	
	SESSION_GPIO_DIRECTION_NB = 0xFF
} session_direction_t;

/*Gpio触发类型*/
typedef enum
{
	SESSION_GPIO_EDGE_NONE = 0,
	SESSION_GPIO_EDGE_RISING = 1,
	SESSION_GPIO_EDGE_FALLING = 2,
	SESSION_GPIO_EDGE_BOTH = 3,
	
	SESSION_GPIO_EDGE_NB = 0xFF
} session_edge_t;


/*会话类型*/
typedef enum
{
	/*硬件类*/
	/*uart*/
	SESSION_TYPE_UART = 0x0,
	/*i2c*/
	SESSION_TYPE_I2C,
	/*gpio*/
	SESSION_TYPE_GPIO,
	/*声音*/
	SESSION_TYPE_SOUND,

	
	/*socket类*/
	/*pipe*/
	SESSION_TYPE_PIPE,
	/*tcp*/
	SESSION_TYPE_TCP,
	/*udp*/
	SESSION_TYPE_UDP,
	
	/*应用协议类*/
	/*mqtt*/
	SESSION_TYPE_MQTT,
	/*COAP*/
	SESSION_TYPE_COAP,
	
	/*接口模拟类*/
	SESSION_TYPE_SOFTWARE_INTERFACE,
	
	SESSION_TYPE_NB = 0xFF
}session_type_t;


/*ioctl 指令类型*/
typedef enum
{
	/*设置串口波特率*/
	SESSION_UART_IOCTL_CMD_BAUDT_RATE = 0,
	/*设置串口数据位*/
	SESSION_UART_IOCTL_CMD_DATA_BITS,
	/*设置串口校验位*/
	SESSION_UART_IOCTL_CMD_PARITY,
	/*设置串口停止位*/
	SESSION_UART_IOCTL_CMD_STOP_BITS,
	
	/*设置GPIO方向, 0-in 1-out*/
	SESSION_GPIO_IOCTL_CMD_DIRECTION_SET = 0,
	/*获取GPIO方向*/
	SESSION_GPIO_IOCTL_CMD_DIRECTION_GET,
	/*设置GPIO触发方式, 0-none 1-rising(上升沿触发) 2-falling(下降沿触发) 3-both(边沿触发)*/
	SESSION_GPIO_IOCTL_CMD_EDGE_SET,
	/*获取GPIO触发方式*/
	SESSION_GPIO_IOCTL_CMD_EDGE_GET,


	/*获取语音忙碌状态*/
	SESSION_SOUND_IOCTL_CMD_BUSY_GET = 0,


	
	SESSION_IOCTL_CMD_NB = 0xFF
}session_ioctl_cmd_t;

/*会话回调函数类型*/
typedef enum
{
	/*接收数据包黏连组合回调*/
	/*
	*该回调的作用将接收到断裂的数据组合在一起
	*接收到黏连的数据分割开
	*/
	//int (*dataSplitCombineFun)(session_ser_class_t *_this, unsigned char*, int, unsigned char *, int *, void *pUserData);
	SESSION_CB_TYPE_DATA_SPLITCOMBINE = 0x0,
	/*接收数据解析回调*/
	/*
	*该回调的作用是解析经过“接收数据包黏连组合回调”处理后的完整数据报文
	*/
	//int (*dataParserFun)(session_ser_class_t *_this, void *, int, void *pUserData);
	SESSION_CB_TYPE_DATA_PARSER,
	/*接收数据与发送数据 匹配回调*/
	/*
	*该回调的作用是，判断回应报文与之匹配的发送报文
	*/
	//int (*DataMatchingFun)(data_repeater_t *_this, void *pSendData, int iSendDataLen, void *pRecvData, int iRecvDataLen, void *pUserData);
	SESSION_CB_TYPE_RESP_MATCHING,
	/*数据接收回调*/
	/*
	*该回调的作用是用于接收最终的会话报文。
	*/
	//int (*dataRecvFun)(session_ser_class_t *_this, void *, int, void *pUserData);
	SESSION_CB_TYPE_DATA_RECV,

	SESSION_CB_TYPE_NB = 0xFF
}session_cb_type_t;


/*会话信息*/
typedef struct session_info_s
{
	/*会话类型*/
	int iSessionType;
	/*会话ID*/
	int iSessionId;
	/*会话地址*/
	char acSessionAddr[SESSION_ADDR_MAX_LEN];
	
	/*数据超时时间*/
	int iDataTimeOut;
	/*数据最小间隔*/
	int iDataSpacing;
	/*数据最大重发次数*/
	int iMaxResendTime;
	
}session_info_t;

/*会话数据*/
typedef struct session_data_s
{
	/*会话类型*/
	int iSessionType;
	/*会话ID*/
	int iSessionId;
	/*时间戳*/
	base_timeval_t stTimestamp;
	
	/*同异步标志*/
	int iSync;
	/*超时重发校验*/
	int iResend;
	
	/*数据解析结果指针*/
	void *pProcessedData;
	/*原始数据长度*/
	int iDataLen;
	/*原始数据*/
	unsigned char aucData[SESSION_DATA_MAX_LEN];
}session_data_t;



typedef struct session_ser_class_s
{
	/*父类*/
	double_link_list_class_t stLinkList;
	
	/*private*/
	unsigned char acPrivateParam[SESSION_SER_PRIVATE_PARAM_MAX_LEN];

	/**************************************************************
	*描述: 创建会话
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @session_info_t* pSessionInfo: 会话属性
	*返回: 成功返回SessionId，失败返回-1；
	**************************************************************/
	int (*open) (struct session_ser_class_s* _this, 
		session_info_t* pSessionInfo);
	
	/**************************************************************
	*描述: 关闭会话
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iSessionId: 会话ID
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*close) (struct session_ser_class_s* _this, 
		int iSessionId);
	
	/**************************************************************
	*描述: 会话控制
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iSessionId: 会话ID
	*	   @int iCmd: 控制指令，详见session_ioctl_cmd_t
	*	   @unsigned long ulArg: 参数
	*返回: 成功返回0，失败返回-1, 具体错误码查看error_num；
	**************************************************************/
	int (*ioctl) (struct session_ser_class_s* _this, 
		int iSessionId,
		int iCmd, 
		unsigned long ulArg);
	
	/**************************************************************
	*描述: 定位数据读取位置
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iOffset: 位移量
	*	   @int iWhence: 起始位置
	*返回: 成功返回SessionId，失败返回-1；
	**************************************************************/
	int (*lseek) (struct session_ser_class_s* _this, 
		int iSessionId,
		int iOffset, 
		int iWhence);
		
	/**************************************************************
	*描述: 向指定会话发送数据
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iSessionId: 会话ID
	*	   @void* pSendData: 将要发送的数据
	*	   @void* pRecvData: 将要同步接收的数据, 如果异步接收则设为NULL
	*	   @int iTimeOut: 同步接收超时
	*
	*返回: 成功返回已发送的数据长度，失败返回-1；
	*说明：如果是同步数据，只能从该接口同步接收，
	*		接收回调以及recv接口都无法收到回应
	**************************************************************/
	int (*send) (struct session_ser_class_s* _this, 
		session_data_t* pSendData, 
		session_data_t* pRecvData, 
		int iTimeOut);
	
	/**************************************************************
	*描述: 接收指定会话的数据
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iSessionId: 会话ID
	*	   @void* pData: 用于存放接收的数据
	*	   @int* piDataLen: 传入接收缓存的最大长度，传出真实接收数据的长度
	*	   @int iTimeOut: 0表示,如果没有数据则立即返回; -1表示,
	*		如果没有数据则阻塞，直到收到数据;
	*		大于0, 如果没有数据，则最多等待iTimeOut，单位ms。
	*返回: 成功返回接收数据长度，失败返回-1；
	*说明: 如果在开启会话时注册了接收回调函数，
	*		则所有的异步数据从回调返回，而不是从该接口接收
	**************************************************************/
	int (*recv) (struct session_ser_class_s* _this, 
		session_data_t* pData,
		int iTimeOut);

	/**************************************************************
	*描述: 回调函数注册
	*参数: @struct session_ser_class_s* _this: 会话服务类指针
	*	   @int iSessionId: 会话ID
	*	   @int iCbType: 回调类型
	*	   @void* pFun: 回调函数指针
	*	   @void* pUserData: 用户数据
	*返回: 成功返回0，失败返回-1；
	**************************************************************/
	int (*callback_reg) (struct session_ser_class_s* _this, 
		int iSessionId,
		int iCbType,
		void* pFun,
		void* pUserData);
}
session_ser_class_t;

/*构造函数*/
session_ser_class_t *new_session_ser(void);

/*析构函数*/
int destroy_session_ser(session_ser_class_t *_this);

#endif /*SESSION_SER_H*/
