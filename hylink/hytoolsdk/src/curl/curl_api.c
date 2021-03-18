#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#include "curl.h"
#include "error_msg.h"
#include "log_api.h"
#include "misc_api.h"


#define CURL_METHOD_GET 0
#define CURL_METHOD_POST 1

#define BUFF_MAX_LEN 4096
/*动作*/
typedef struct curl_buff_s
{
	/*缓存*/
	unsigned char aucBuff[BUFF_MAX_LEN];
	/*长度*/
	unsigned int uiLen;
}curl_buff_t;

size_t curl_write_data_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t iLen = size * nmemb;
	if (userp)
	{
		curl_buff_t *pstCurlBuff = (curl_buff_t *)userp;
		//DEBUG("Len = %d, Date = %s\n", iLen, buffer);
		if(pstCurlBuff->uiLen + iLen < BUFF_MAX_LEN)
		{
			memcpy(pstCurlBuff->aucBuff + pstCurlBuff->uiLen, buffer, iLen);
			pstCurlBuff->uiLen += iLen;
		}
	}
	return iLen;
}

size_t curl_write_file_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t iLen = size * nmemb;
	if (userp)
	{
		FILE *fp = NULL;
		
		if (access((char*)userp, F_OK) == -1) 
		{
			fp = fopen((char*)userp, "wb");
		} 
		else 
		{
			fp = fopen((char*)userp, "ab");
		}
		if (NULL == fp) 
		{
			HY_DEBUG("open file(%s) failed\n", (char*)userp);
			return iLen;
		}
		fwrite(buffer, size, nmemb, fp);
		fclose(fp);
	}
	return iLen;
}

/*
	http get func 
*/
int curl_http_get(char *pcUrl, char *pcResult, int *piLen)
{	
	int iLenMax = *piLen;
	*piLen = 0;
	CURL *pstCurl; 
	CURLcode curlRet;
	curl_buff_t stCurlBuff;

	memset(&stCurlBuff, 0x0, sizeof(curl_buff_t));
    
	//curl初始化 
	pstCurl = curl_easy_init(); 
	if (!pstCurl)
	{
		HY_ERROR("curl easy init failed\n");
		return GeneralErr;
	}
 
	if (0 == strncmp(pcUrl, "https://", 8))
	{
		//设定为不验证证书和HOST
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
		
	curl_easy_setopt(pstCurl, CURLOPT_HEADER, 0);	//设置httpheader 解析, 不需要将HTTP头写传入回调函数
	curl_easy_setopt(pstCurl, CURLOPT_URL, pcUrl);	//设置远端地址 
	curl_easy_setopt(pstCurl, CURLOPT_VERBOSE, 1L);	// TODO: 打开调试信息
	curl_easy_setopt(pstCurl, CURLOPT_FOLLOWLOCATION, 1);	//设置允许302  跳转
	curl_easy_setopt(pstCurl, CURLOPT_WRITEFUNCTION, curl_write_data_cb); 	//执行写入文件流操作 的回调函数
	curl_easy_setopt(pstCurl, CURLOPT_WRITEDATA, &stCurlBuff);	// 设置回调函数的第4 个参数
	curl_easy_setopt(pstCurl, CURLOPT_CONNECTTIMEOUT, 5); 	//设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
	curl_easy_setopt(pstCurl, CURLOPT_NOSIGNAL, 1);		//linux多线程情况应注意的设置(防止curl被alarm信号干扰)
 
	curlRet = curl_easy_perform(pstCurl); 
	if (CURLE_OK != curlRet)
	{
		HY_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRet));
		curl_easy_cleanup(pstCurl);
		return GeneralErr;
	}
 	
	curl_easy_cleanup(pstCurl);
	
	if(NULL != pcResult && NULL != piLen)
	{
		*piLen = stCurlBuff.uiLen > iLenMax ? iLenMax : stCurlBuff.uiLen;
		memcpy(pcResult, stCurlBuff.aucBuff, *piLen);
	}
	return NoErr;
}
int curl_http_post(char *pcUrl, char*pcData, char *pcResult, int *piLen)
{
	int iLenMax = *piLen;
	*piLen = 0;
	CURL *pstCurl; 
	CURLcode curlRet;
	curl_buff_t stCurlBuff;
	memset(&stCurlBuff, 0x0, sizeof(curl_buff_t));
	
	//curl初始化 
	pstCurl = curl_easy_init(); 
	if (!pstCurl)
	{
		HY_ERROR("curl easy init failed\n");
		return GeneralErr;
	}
 
	if (0 == strncmp(pcUrl, "https://", 8))
	{
		//设定为不验证证书和HOST
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	
	curl_easy_setopt(pstCurl, CURLOPT_URL, pcUrl);
	curl_easy_setopt(pstCurl, CURLOPT_POST, 1);
	curl_easy_setopt(pstCurl, CURLOPT_WRITEFUNCTION, curl_write_data_cb);
	curl_easy_setopt(pstCurl, CURLOPT_WRITEDATA, &stCurlBuff);
	curl_easy_setopt(pstCurl, CURLOPT_POSTFIELDS, pcData);

    curlRet = curl_easy_perform(pstCurl);
	if (CURLE_OK != curlRet)
	{
		HY_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRet));
		curl_easy_cleanup(pstCurl);
		return GeneralErr;
	}


	curl_easy_cleanup(pstCurl);

	if(NULL != pcResult && NULL != piLen)
	{
		*piLen = stCurlBuff.uiLen > iLenMax ? iLenMax : stCurlBuff.uiLen;
		memcpy(pcResult, stCurlBuff.aucBuff, *piLen);
	}
	return NoErr;
}

int curl_http_post_json(char *pcUrl, char*pcJson, char *pcResult, int *piLen)
{
	int iLenMax = *piLen;
	*piLen = 0;
	CURL *pstCurl; 
	CURLcode curlRet;
	curl_buff_t stCurlBuff;
	
	memset(&stCurlBuff, 0x0, sizeof(curl_buff_t));
	
	//curl初始化 
	pstCurl = curl_easy_init(); 
	if (!pstCurl)
	{
		HY_ERROR("curl easy init failed\n");
		return GeneralErr;
	}
 
	if (0 == strncmp(pcUrl, "https://", 8))
	{
		//设定为不验证证书和HOST
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	
    curl_easy_setopt(pstCurl, CURLOPT_URL,pcUrl);
    curl_easy_setopt(pstCurl, CURLOPT_POST, 1);
    struct curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");  
    curl_easy_setopt(pstCurl, CURLOPT_HTTPHEADER, plist);  
    curl_easy_setopt(pstCurl, CURLOPT_WRITEFUNCTION, curl_write_data_cb);
	curl_easy_setopt(pstCurl, CURLOPT_WRITEDATA,&stCurlBuff);
    curl_easy_setopt(pstCurl, CURLOPT_POSTFIELDS, pcJson);
	
    curlRet = curl_easy_perform(pstCurl);
	if (CURLE_OK != curlRet)
	{
		HY_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRet));
		curl_easy_cleanup(pstCurl);
		return GeneralErr;
	}


	curl_easy_cleanup(pstCurl);
	
	if(NULL != pcResult && NULL != piLen)
	{
		*piLen = stCurlBuff.uiLen > iLenMax ? iLenMax : stCurlBuff.uiLen;
		memcpy(pcResult, stCurlBuff.aucBuff, *piLen);
	}
	return NoErr;
}

int curl_download_file(char *pcUrl, char *pcPath, char *pcMd5)
{
	int iCount = 0;
	CURL *pstCurl; 
	CURLcode curlRet;

	if (0 == access(pcPath, F_OK)) 
	{
		remove(pcPath);
	} 
		
	//curl初始化 
	pstCurl = curl_easy_init(); 
	if (!pstCurl)
	{
		HY_ERROR("curl easy init failed\n");
		return GeneralErr;
	}
 
	if (0 == strncmp(pcUrl, "https://", 8))
	{
		//设定为不验证证书和HOST
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
		
	curl_easy_setopt(pstCurl, CURLOPT_HEADER, 0);	//设置httpheader 解析, 不需要将HTTP头写传入回调函数
	curl_easy_setopt(pstCurl, CURLOPT_URL, pcUrl);	//设置远端地址 
	curl_easy_setopt(pstCurl, CURLOPT_VERBOSE, 1L);	// TODO: 打开调试信息
	curl_easy_setopt(pstCurl, CURLOPT_FOLLOWLOCATION, 1);	//设置允许302  跳转
	curl_easy_setopt(pstCurl, CURLOPT_WRITEFUNCTION, curl_write_file_cb); 	//执行写入文件流操作 的回调函数
	curl_easy_setopt(pstCurl, CURLOPT_WRITEDATA, pcPath);	// 设置回调函数的第4 个参数
	curl_easy_setopt(pstCurl, CURLOPT_CONNECTTIMEOUT, 5); 	//设置连接超时，单位s, CURLOPT_CONNECTTIMEOUT_MS 毫秒
	curl_easy_setopt(pstCurl, CURLOPT_NOSIGNAL, 1);		//linux多线程情况应注意的设置(防止curl被alarm信号干扰)

	iCount = 3;
	while(iCount)
	{
		curlRet = curl_easy_perform(pstCurl); 
		if (CURLE_OK != curlRet)
		{
			HY_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRet));
			iCount--;
			continue;
		}
		if(NULL != pcMd5)
		{
			/*MD5校验*/
			char acMd5[MD5_MAX_LEN] = {0};
			MD5_File(pcPath, acMd5);
			if(!strcasecmp(pcMd5, acMd5))
			{
				break;
			}
			else
			{
				HY_ERROR("File MD5 check exception.\n");
				iCount--;
				continue;
			}
		}
		else
		{
			break;
		}
	}
	
	curl_easy_cleanup(pstCurl);
	
	return NoErr;
}

int curl_init(void)
{
	curl_global_init(CURL_GLOBAL_ALL);
	return NoErr;
}

int curl_cleanup(void)
{
	curl_global_cleanup();
	return NoErr;
}
