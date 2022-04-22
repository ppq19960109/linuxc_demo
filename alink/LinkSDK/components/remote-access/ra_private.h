/**
 * @file tunnel_private.h
 * @brief 安全隧道内部结构体定义
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */
#ifndef _RA_PRIVATE_H_
#define _RA_PRIVATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "aiot_sysdep_api.h"
#include "aiot_ra_api.h"

typedef struct {
    void *tunnel_switch;
    void *tunnel;
    aiot_ra_event_handler_t   event_handle;
    void*                     userdata;                          /* 组件调用入参之一 */
    int32_t result;
} ra_handle_t;


#endif /* __AIOT_tunnel_API_H_ */
