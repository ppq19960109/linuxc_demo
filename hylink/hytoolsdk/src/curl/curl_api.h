/***********************************************************
*文件名     : curl_api.h
*版   本   : v1.0.0.0
*日   期   : 2018.05.03
*说   明   : 变量最大长度定义
*修改记录: 
************************************************************/

#ifndef CURL_API_H
#define CURL_API_H

int curl_http_get(char *pcUrl, char *pcResult, int *piLen);
int curl_http_post(char *pcUrl, char*pcData, char *pcResult, int *piLen);
int curl_http_post_json(char *pcUrl, char*pcJson, char *pcResult, int *piLen);

int curl_download_file(char *pcUrl, char *pcPath, char *pcMd5);

int curl_init(void);
int curl_cleanup(void);

#endif /* CURL_API_H */

