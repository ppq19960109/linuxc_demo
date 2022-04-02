
#include "tunnel_proxy_private.h"
#include "tunnel_session_mgr.h"
#include "core_stdinc.h"
#include "core_log.h"

static SESSION_INFO_NODE_S *creat_one_session_node(void *network_handle, char *sessionID, void *data)
{
    SESSION_INFO_NODE_S *session_node = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }
    //创建session空间
    session_node = (SESSION_INFO_NODE_S *)sysdep->core_sysdep_malloc(sizeof(SESSION_INFO_NODE_S), "SESSION");
    if(NULL == session_node)
        return NULL;

    session_node->network_handle = network_handle;
    session_node->data = data;
    strncpy(session_node->sessionID, sessionID, sizeof(session_node->sessionID) - 1);
    return session_node;
}

static int release_one_session_node(SESSION_INFO_NODE_S *session_node)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }
    //关闭socket
    if(session_node->network_handle != NULL)
    {
        sysdep->core_sysdep_network_deinit(&session_node->network_handle);
        session_node->network_handle = NULL;
    }

    //释放buffer

    //释放session空间
    sysdep->core_sysdep_free(session_node);
    return 0;
}

/*************************************************************************
 * 接口名称：init_session_list
 * 描       述：初始化session哈希表
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
int init_session_list(SESSION_LIST_S * rule_list)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }

    rule_list->list_lock = sysdep->core_sysdep_mutex_init();
    CORE_INIT_LIST_HEAD(&rule_list->session_list);

    return STATE_SUCCESS;
}

/*************************************************************************
 * 接口名称：release_all_session_from_list
 * 描       述：释放所有的session
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
int release_all_session_from_list(SESSION_LIST_S * rule_list)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }
    SESSION_INFO_NODE_S *item = NULL, *next = NULL;
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);

    core_list_for_each_entry_safe(item, next, &rule_list->session_list, node, SESSION_INFO_NODE_S)
    {
        core_list_del(&item->node);
        release_one_session_node(item);
    }

    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);
    return 0;
}

/*************************************************************************
 * 接口名称：add_one_session_to_list
 * 描       述：向哈希表中添加一个session资源节点
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
int add_one_session_to_list(SESSION_LIST_S * rule_list, char* sessionID, void *network_handle, void *data)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);

    SESSION_INFO_NODE_S *session_info = creat_one_session_node(network_handle,sessionID, data);
    if(NULL == session_info)
    {
        sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);
        core_log(sysdep, 0, "session node create failed\r\n");
        return STATE_TUNNEL_FAILED;
    }

    core_list_add(&session_info->node, &rule_list->session_list);
    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);

    return STATE_SUCCESS;
}

/*************************************************************************
 * 接口名称：release_one_session_from_list
 * 描       述：从哈希表中释放一个session资源节点
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
int release_one_session_from_list(SESSION_LIST_S * rule_list,char* sessionID)
{
    //找到此session
    SESSION_INFO_NODE_S *session_node = get_one_session_from_list(rule_list, sessionID);
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }

    if(NULL == session_node)
    {
        return STATE_SUCCESS;
    }

    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);
    //从List中remove出session
    core_list_del(&session_node->node);
    //释放session
    release_one_session_node(session_node);
    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);

    return STATE_SUCCESS;
}

/*************************************************************************
 * 接口名称：get_one_session_from_list
 * 描       述：从哈希表中获取一个session
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
SESSION_INFO_NODE_S* get_one_session_from_list(SESSION_LIST_S * rule_list,char* sessionID)
{
    // SESSION_INFO_NODE_S* session_list_lookup_key(const session_list *session_list, const void *key);
    SESSION_INFO_NODE_S *iterm = NULL, *next = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return NULL;
    }
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);

    core_list_for_each_entry_safe(iterm, next, &rule_list->session_list, node, SESSION_INFO_NODE_S)
    {
        if(0 == strncmp(iterm->sessionID, sessionID, strlen(sessionID)))
        {
            sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);
            return iterm;
        }
    }
    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);

    return NULL;

}

/*************************************************************************
 * 接口名称：has_one_session
 * 描       述：判断session是否存在
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
int has_one_session(SESSION_LIST_S * rule_list,char* sessionID)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);

    SESSION_INFO_NODE_S *node = get_one_session_from_list(rule_list, sessionID);
    if(node == NULL)
    {
        sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);
        return STATE_TUNNEL_FAILED;
    }

    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);

    return STATE_SUCCESS;
}

/*********************************************************
 * 接口名称：get_session_num_from_list
 * 描       述：获取本地服务session的数量
 * 输入参数：SESSION_LIST_S * rule_list
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int get_session_num_from_list(SESSION_LIST_S * rule_list)
{
    int num = 0;
    SESSION_INFO_NODE_S *iterm = NULL, *next = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return STATE_TUNNEL_FAILED;
    }
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);

    core_list_for_each_entry_safe(iterm, next, &rule_list->session_list, node, SESSION_INFO_NODE_S)
    {
        num++;
    }
    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);

    return num;
}

/*************************************************************************
 * 接口名称：iterate_each_session
 * 描       述：遍历session_list中的每一个session资源
 * 输入参数：
 * 输出参数：
 * 返  回 值：成功：0
 *          失败：非0
 * 说       明：
 *************************************************************************/
void iterate_each_session(SESSION_LIST_S * rule_list,ITERATOR_CALL_BACK callback,void*data)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep || NULL == rule_list) {
        return;
    }
    sysdep->core_sysdep_mutex_lock(rule_list->list_lock);
    SESSION_INFO_NODE_S *iterm = NULL, *next = NULL;
    core_list_for_each_entry_safe(iterm, next, &rule_list->session_list, node, SESSION_INFO_NODE_S)
    {
        if(callback != NULL)
        {
            callback(iterm,data);
        }
    }
    sysdep->core_sysdep_mutex_unlock(rule_list->list_lock);
}



