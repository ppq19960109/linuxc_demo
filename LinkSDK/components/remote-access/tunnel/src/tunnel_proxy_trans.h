/**
 * @file tunnel_proxy_trans.h
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_PROXY_TRANS_H_
#define _TUNNEL_PROXY_TRANS_H_

#include "tunnel_buffer_mgr.h"
#include "tunnel_proxy_protocol.h"
#include "tunnel_proxy_private.h"

int get_remote_access_protocol_header(RA_BUFFER_INFO_S *channel_buffer, PROXY_PROT_HEADER_S *hdr);
int send_cloud_channel_release_session_request(REMOTE_PROXY_INFO_S *remote_proxy_info,char *msg_id, char *session_id);
int cloud_channel_response_with_error(REMOTE_PROXY_INFO_S *remote_proxy_info,int code, char *msg, char *msg_id, char *session_id);
int cloud_channel_response_new_session(REMOTE_PROXY_INFO_S *remote_proxy_info, const char *msg_id, const char *session_id);
int cloud_channel_response_release_session(REMOTE_PROXY_INFO_S *remote_proxy_info, const char *msg_id, const char *session_id);

#endif
