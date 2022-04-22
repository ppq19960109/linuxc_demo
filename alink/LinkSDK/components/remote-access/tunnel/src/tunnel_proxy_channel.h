/**
 * @file tunnel_proxy_channel.h
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_PROXY_CHANNEL_H_
#define _TUNNEL_PROXY_CHANNEL_H_

#include "tunnel_buffer_mgr.h"

/*
*  接口名称: call_back_func
*  功能描述: cloud_channel_read 接口回调参数的定义
*  输入参数:
*   void *args              用户自定义数据
*   const char *data        recv的数据
*   int data_len             data 数据的长度
*   is_fIn                   报文数据是否结束(websocket 报文)
*/
typedef void (*call_back_func)(void *args, const char *data, int data_len, int is_fin);


void *open_cloud_proxy_channel(char *host, char *port, char *path, char *token, void *userdata, int *code);
void  close_cloud_proxy_channel(void *channel_handle);
int   write_cloud_proxy_channel(void *channel_handle, const char *data, unsigned int len, unsigned int timeout);
int   read_cloud_proxy_channel(void *channel_handle, RA_BUFFER_INFO_S *channel_buffer,int *is_fin);
void update_log_switch(void* conn, int is_debug);

#endif
