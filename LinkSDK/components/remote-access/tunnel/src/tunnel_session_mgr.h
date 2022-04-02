/**
 * @file tunnel_session_mgr.h
 * @brief 管理隧道的绘画
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_SESSION_MGR_H_
#define _TUNNEL_SESSION_MGR_H_

#include "core_list.h"

#define DEFAULT_SESSION_LEN 64
#define DEFAULT_SESSION_COUNT 32

typedef struct SESSION_INFO_NODE
{
    void               *network_handle;                               //本地服务的socket
//    RA_BUFFER_INFO_S  sendbuffer;                             //每个服务的发送缓存，用于保证发送数据完整性，当前此参数没有使用，可用于后续优化
    char              sessionID[DEFAULT_SESSION_LEN];         //本地服务建立的sessionID
    void              *data;
    struct core_list_head       node;
} SESSION_INFO_NODE_S;

typedef struct SESSION_List
{
    void *list_lock;
    struct core_list_head  session_list;   //远程服务信息链表，其node为_lOCAL_SERVICE_NODE_S
} SESSION_LIST_S;

typedef int (* ITERATOR_CALL_BACK)(SESSION_INFO_NODE_S *session_info, void *data);

int init_session_list(SESSION_LIST_S * rule_list);
int add_one_session_to_list(SESSION_LIST_S * rule_list,  char* sessionID, void *network_handle, void *data);
int release_one_session_from_list(SESSION_LIST_S * rule_list,char* sessionID);
int release_all_session_from_list(SESSION_LIST_S * rule_list);
void iterate_each_session(SESSION_LIST_S * rule_list,ITERATOR_CALL_BACK callback,void*data);
SESSION_INFO_NODE_S* get_one_session_from_list(SESSION_LIST_S * rule_list,char* sessionID);
int get_session_num_from_list(SESSION_LIST_S * rule_list);
int has_one_session(SESSION_LIST_S * rule_list,char* sessionID);


#endif /* ADVANCED_SERVICES_REMOTE_ACCESS_DAEMON_REMOTE_ACCESS_SESSION_MGR_H_ */
