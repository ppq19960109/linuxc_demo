/**
 * @file tunnel_buffer_mgr.h
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_BUFFER_MGR_H_
#define _TUNNEL_BUFFER_MGR_H_

typedef struct RA_BUFFER_INFO
{
    char* buffer;
    int   size;          //buffer的大小
    int   read_index;     //读位置下标
    int   write_index;    //写位置下标
} RA_BUFFER_INFO_S;

int create_tunnel_buffer(RA_BUFFER_INFO_S *RAbuffer, int size);
void release_tunnel_buffer(RA_BUFFER_INFO_S* buffer);
int reset_tunnel_buffer(RA_BUFFER_INFO_S* buffer);
int write_tunnel_buffer(RA_BUFFER_INFO_S* buffer, const char*data, int len);
char* get_tunnel_buffer_read_pointer(RA_BUFFER_INFO_S* buffer);
char* get_tunnel_buffer_write_pointer(RA_BUFFER_INFO_S* buffer);
int get_tunnel_buffer_read_len(RA_BUFFER_INFO_S* buffer);
int move_tunnel_buffer_read_pointer(RA_BUFFER_INFO_S* buffer, int offset_len);
int move_tunnel_buffer_write_pointer(RA_BUFFER_INFO_S* buffer, int offset_len);
int memmove_tunnel_buffer(RA_BUFFER_INFO_S* buffer, int offset_len);
void reset_tunnel_buffer_read_point(RA_BUFFER_INFO_S* buffer);
void reset_tunnel_buffer_write_point(RA_BUFFER_INFO_S* buffer);
int join_content_before_tunnel_buffer(char*data, int len, RA_BUFFER_INFO_S *channel_buffer);

#endif
