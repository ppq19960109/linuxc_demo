#include "curl.h"
#include "easy.h"
#include <iostream>
using namespace std;

void print_cookies(CURL *easy_handle)
{
    printf("curl_easy_getinfo CURLINFO_COOKIELIST\n");
    struct curl_slist *cookies, *nc;
    int num = 1;
    curl_easy_getinfo(easy_handle, CURLINFO_COOKIELIST, &cookies);
    nc = cookies;
    while (nc)
    {
        printf("[%d]: %s\n", num++, nc->data);
        nc = nc->next;
    }
    curl_slist_free_all(cookies);
}
/**
 *	@brief libcurl接收到数据时的回调函数
 *
 *	将接收到的数据保存到本地文件中，同时显示在控制台上。
 *
 *	@param [in] buffer 接收到的数据所在缓冲区
 *	@param [in] size 数据长度
 *	@param [in] nmemb 数据片数量
 *	@param [in/out] 用户自定义指针
 *	@return 获取的数据长度
 */

size_t
process_data(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    FILE *fp = (FILE *)user_p;
    size_t return_size = fwrite(buffer, size, nmemb, fp);
    cout << (char *)buffer << endl;
    return return_size;
}

int main(int argc, char **argv)
{
    // 初始化libcurl
    CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_SSL); //CURL_GLOBAL_SSL
    if (CURLE_OK != return_code)
    {
        cerr << "init libcurl failed." << endl;
        return -1;
    }

    // 获取easy handle
    CURL *easy_handle = curl_easy_init();
    if (NULL == easy_handle)
    {
        cerr << "get a easy handle failed." << endl;
        curl_global_cleanup();
        return -1;
    }

    FILE *fp = fopen("data.html", "wb+"); //
    // 设置easy handle属性
    curl_easy_setopt(easy_handle, CURLOPT_URL, "https://blog.csdn.net/JGood");
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &process_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

    // 执行数据请求
    CURLcode res = curl_easy_perform(easy_handle);
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
    }

    // 释放资源

    fclose(fp);
    curl_easy_cleanup(easy_handle);
    curl_global_cleanup();

    return 0;
}
