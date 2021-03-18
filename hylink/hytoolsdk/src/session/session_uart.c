/***********************************************************
*文件名     : session_ser.c
*版   本   : v1.0.0.0
*日   期   : 2019.04.17
*说   明   : 会话服务
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>

#include "error_msg.h"
#include "base_api.h"
#include "log_api.h"

/*ioctl 指令类型*/
typedef enum
{
	/*设置串口波特率*/
	SESSION_UART_IOCTL_CMD_BAUDT_RATE_SET,
	/*设置串口数据位*/
	SESSION_UART_IOCTL_CMD_DATA_BITS_SET,
	/*设置串口校验位*/
	SESSION_UART_IOCTL_CMD_PARITY_SET,
	/*设置串口停止位*/
	SESSION_UART_IOCTL_CMD_STOP_BITS_SET,
	
	SESSION_UART_IOCTL_CMD_NB = 0xFF
}session_uart_ioctl_cmd_t;

/**************************************************************
*描述: 会话打开
*参数: 
*返回: 成功返回会话ID，失败返回-1；
*说明: 默认 9600,8,N,1
**************************************************************/
int session_uart_open(const char * pSessionDev, int iMode)
{
	int iRet = 0;
	int iFd = -1;
	struct termios stTio = {0};
	
	/*打开串口文件*/
	iFd = base_open(pSessionDev, O_RDWR | O_NOCTTY | O_NDELAY);
	if(iFd < 0)
	{
		HY_ERROR("Can't Open Serial Port!\n");
		return GeneralErr;
	}
	
	/*将本地模式(CLOCAL)和串行数据接收(CREAD)设置为有效*/
	stTio.c_cflag |= (CLOCAL | CREAD);
	
	/*设置波特率*/	
	cfsetispeed(&stTio, B9600);
	cfsetospeed(&stTio, B9600);
	
	/*设置字符大小*/
	stTio.c_cflag &= ~CSIZE;
	stTio.c_cflag |= CS8;
	
	/*选择奇偶校验*/
	stTio.c_cflag &= ~PARENB;
	stTio.c_iflag &= ~INPCK; 
	/*选择停止位*/
	stTio.c_cflag &= ~CSTOPB;
	
	/*串口工作在原始模式下*/
	stTio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	/*选择原始输出*/
	stTio.c_oflag &= ~OPOST;
	/*不使用流量控制*/
	stTio.c_cflag &= ~CRTSCTS; 
	
	//设置等待时间和最小接收字符  
	stTio.c_cc[VTIME] = 1;/* 读取一个字符等待0*(1/10)s */ 
	stTio.c_cc[VMIN] = 0;/* 读取字符的最少个数为0 */
	
	tcflush(iFd, TCIOFLUSH);
	iRet = tcsetattr(iFd, TCSANOW, &stTio);
	if(iRet != 0)
	{
		HY_ERROR("Setup Serial arguments error");
		base_close(iFd);
		return GeneralErr;
	}
	
	return iFd;
}

/**************************************************************
*描述: 会话关闭
*参数: 
*返回: 成功返回会话ID，失败返回-1；
**************************************************************/
int session_uart_close(int iSessionId)
{
	return base_close(iSessionId);
}

/**************************************************************
*描述: 会话控制接口
*参数: @int iSessionId: 会话ID
*	   @int iCmd: 控制指令
*	   @unsigned long ulArg: 指令参数
*返回: 成功返回0，失败返回-1；
**************************************************************/
int session_uart_ioctl(int iSessionId, int iCmd, unsigned long ulArg)
{
	int iRet = 0;
	int i = 0;
	struct termios stTio = {0};
	
	int aiBaudRateBin[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1800, B1200, B600, B300};
	int aiBaudRate[] =    { 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1800,  1200,  600,  300};
	
	/*获取属性*/
	iRet = tcgetattr(iSessionId, &stTio);
	if(iRet < 0)
	{
		HY_ERROR("Failed to get serial attribute.\n");
		error_num = SystemErr;
		return GeneralErr;
	}

	switch(iCmd)
	{
		/*设置串口波特率*/
		case SESSION_UART_IOCTL_CMD_BAUDT_RATE_SET:
			{
				for(i = 0; i < sizeof(aiBaudRateBin)/sizeof(int); i++)
				{
					if(ulArg == aiBaudRate[i])
					{
						break;
					}
				}
				if(i >= sizeof(aiBaudRateBin)/sizeof(int))
				{
					error_num = ParaErr;
					return GeneralErr;
				}
				
				cfsetispeed(&stTio, aiBaudRateBin[i]);
				cfsetospeed(&stTio, aiBaudRateBin[i]);

				
				tcflush(iSessionId, TCIOFLUSH);
				return tcsetattr(iSessionId, TCSANOW, &stTio);
			}
			break;
		/*设置串口数据位*/
		case SESSION_UART_IOCTL_CMD_DATA_BITS_SET:
			{
				stTio.c_cflag &= ~CSIZE;//数据位屏蔽
				switch(ulArg)
				{
					case 5:
						stTio.c_cflag |= CS5;
						break;
					case 6:
						stTio.c_cflag |= CS6;
						break;
					case 7:
						stTio.c_cflag |= CS7;
						break;
					case 8:
						stTio.c_cflag |= CS8;
						break;
					default:
						error_num = ParaErr;
						return GeneralErr;
				}
				
				tcflush(iSessionId, TCIOFLUSH);
				return tcsetattr(iSessionId, TCSANOW, &stTio);
			}
			break;
		/*设置串口校验位*/
		case SESSION_UART_IOCTL_CMD_PARITY_SET:
			{
				switch(ulArg)
				{
					case 'n'://无校验
					case 'N':
						stTio.c_cflag &= ~PARENB;
						stTio.c_iflag &= ~INPCK; //打开输入奇偶校验 
						break;
					case 'o'://奇校验
					case 'O':
						stTio.c_cflag |= PARENB;//进行奇偶校验
						stTio.c_cflag |= PARODD;//奇校验,否则偶校验
						stTio.c_iflag |= (INPCK | ISTRIP/*剥除字符第8位*/); 
						break;
					case 'e'://偶校验
					case 'E':
						stTio.c_cflag |= PARENB;
						stTio.c_cflag &= ~PARODD;
						stTio.c_iflag |= (INPCK | ISTRIP);
						break;
					case 's'://空格
					case 'S':
						stTio.c_cflag &= ~PARENB;
						stTio.c_cflag &= ~CSTOPB;//2位停止位，否则为1位 
						break;
					default:
						error_num = ParaErr;
						return GeneralErr;
				}
				
				tcflush(iSessionId, TCIOFLUSH);
				return tcsetattr(iSessionId, TCSANOW, &stTio);
			}
			break;
		/*设置串口停止位*/
		case SESSION_UART_IOCTL_CMD_STOP_BITS_SET:
			{
				switch(ulArg)
				{
					case 1:
						stTio.c_cflag &= ~CSTOPB;
						break;
					case 2:
						stTio.c_cflag |= CSTOPB;
						break;
					default:
						error_num = ParaErr;
						return GeneralErr;
				}

				tcflush(iSessionId, TCIOFLUSH);
				return tcsetattr(iSessionId, TCSANOW, &stTio);
			}
			break;
		default:
			error_num = ParaErr;
			return GeneralErr;
	}

	
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 读取缓存
*	   @int iDataLen: 缓存长度
*返回: 成功返回读取长度，失败返回-1；
**************************************************************/
int session_uart_read(int iSessionId, void * pData, int iDataLen)
{
	return base_read (iSessionId, pData, iDataLen);
}

/**************************************************************
*描述: 会话读取接口
*参数: @int iSessionId: 会话ID
*	   @void * pData: 发送缓存
*	   @int iDataLen: 发送数据长度
*返回: 成功返回发送长度，失败返回-1；
**************************************************************/
int session_uart_write(int iSessionId, void * pData, int iDataLen)
{
	return base_write(iSessionId, pData, iDataLen);
}