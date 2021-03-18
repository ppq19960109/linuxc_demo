/***********************************************************
*文件名     : tool.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.28
*说   明   : 公用工具接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <sys/ioctl.h>  
#include <net/if.h>  
#include <unistd.h>  
#include <netinet/in.h>  
#include <string.h>  
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>


#include "len.h"
#include "tool.h"
#include "log_api.h"


int HYgetMac(char* Mac)  
{  
    struct ifreq ifr;  
    struct ifconf ifc;  
    char buf[2048];  
   
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);  
    if (sock == -1) {  
        printf("socket error\n");  
        return -1;  
    }  
   
    ifc.ifc_len = sizeof(buf);  
    ifc.ifc_buf = buf;  
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {  
        printf("ioctl error\n");  
        return -1;  
    }  
   
    struct ifreq* it = ifc.ifc_req;  
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));  
    char szMac[64];  
    int count = 0;  
    for (; it != end; ++it) {  
        strcpy(ifr.ifr_name, it->ifr_name);  
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {  
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback  
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {  
                    count ++ ;  
                    unsigned char * ptr ;  
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];  
                    snprintf(szMac,64,"%02X%02X%02X%02X%02X%02X",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));  
                    // printf("%d,Interface name : %s , Mac address : %s \n",count,ifr.ifr_name,szMac);
                    memcpy(Mac, szMac, 64); 
                }  
            }  
        }else{  
            printf("get mac info error\n");  
            return -1;  
        }  
    }  
	return 0;
}  

/*************************************************************
*函数:	read_file
*参数:	pcPath :文件路径
*		pcBuff :输出缓存
*		iBuffSize:缓存大小
*返回值:成功返回0,失败返回-1
*描述:	读取文件
*************************************************************/
extern int
read_file(const char *pcPath, char *pcBuff, int iBuffSize)
{
	/*参数定义*/
	int i = 0;
	int iReadSize = 0;
	int iTotalReadSize = 0;
	FILE *fp =NULL;
	char *pcData = NULL;
	char *pcStr = NULL;
	
	/*参数校验*/
	if(NULL == pcPath ||
		NULL == pcBuff ||
		iBuffSize <= 0)
	{
		HY_ERROR("Parameter error.\n");
		return -1;
	}

	/*判断文件是否存在*/
	if(0 != access(pcPath, R_OK))
	{
		HY_ERROR("The dir(%s) not exist.", pcPath);
		return -1;
	}
	/*打开文件*/
	fp = fopen(pcPath, "r");
	if(NULL == fp)
	{
		HY_ERROR("Open the file(%s) failed.\n",pcPath);
		return -1;
	}

	/*获取命令的执行结果*/
	pcData = (char*)calloc(1, 4096);
	if(NULL == pcData)
	{
		/*关闭文件*/
		fclose(fp);
		HY_ERROR("Malloc failed.\n");
		return -1;
	}
	
	pcStr = pcData;
	do
	{
		i++;
		iReadSize = 0;
		iReadSize = fread (pcStr, 4096, 1, fp);
		if(1 != iReadSize)
		{
			break;
		}
		iTotalReadSize += iReadSize;
		
		pcData =
			(char*)realloc(pcData,(i + 1) * 4096);
		if(NULL == pcData)
		{
			/*关闭文件*/
			fclose(fp);
			HY_ERROR("Realloc failed.\n");
			return -1;
		}
		pcStr = pcData + (i * 4096);
		memset(pcStr, 0x0, 4096);	
	}while(1);

	/*返回读取结果*/
	strncpy(pcBuff, pcData, iBuffSize);
	pcBuff[iBuffSize - 1] = '\0';
	
	/*关闭管道*/
	fclose(fp);
	free(pcData);

	return 0;
}

/*************************************************************
*函数:	stristr
*参数:	str1:被查找目标
*		str2:要查找对象
*返回值:成功返回str2在str1的首次出现的地址,失败返回NULL
*描述:	忽略大小写的strstr
*************************************************************/
extern char * stristr(const char * str1, const char * str2)
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
	return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *) str2;

		while ( *s1 && *s2 && !(_tolower(*s1)-_tolower(*s2)) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);

}
/*************************************************************
*函数:	is_int
*参数:	str:字符串
*返回值:1表示是整数，0表示非整数
*描述:	判断字符串是否是整数类型
*************************************************************/
extern int is_int(char * str)
{
	char *p = str;
	while(*p)
	{
		if(*p < '0' || *p > '9')
		{
			return 0;
		}
		p++;
	}
	return 1;
}
/*************************************************************
*函数:	is_float
*参数:	str:字符串
*返回值:1表示是浮点数，0表示非浮点数
*描述:	判断字符串是否是浮点类型
*************************************************************/
extern int is_float(char * str)
{
	int iCount = 0;
	char *p = str;
	while(*p)
	{
		if((*p < '0' || *p > '9'))
		{
			if(*p == '.')
			{
				iCount ++;
			}
			else
			{
				return 0;
			}
		}
		p++;
	}
	if(1 != iCount)
	{
		return 0;
	}
	
	return 1;
}

/*************************************************************
*函数:	time_cmp
*参数:	iHour_1:iMinu_1 时间1的时分
*		iHour_2:iMinu_2 时间2的时分
*返回值:0表示时间相等，小于0，表示时间1先于时间2，大于0，表示时间1后于时间2
*描述:	判断时间先后(时间应为24小时制，且不跨天)
*************************************************************/
extern int time_cmp(int iHour_1, int iMinu_1, int iHour_2, int iMinu_2)
{
	if(iHour_1 > iHour_2)
	{
		return 1;
	}
	else if(iHour_1 < iHour_2)
	{
		return -1;
	}
	else
	{
		if(iMinu_1 > iMinu_2)
		{
			return 1;
		}
		else if(iMinu_1 < iMinu_2)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
}



