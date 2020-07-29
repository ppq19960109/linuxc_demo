/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: HiLink Interface头文件
 */
#ifndef HILINK_INTERFACE_H
#define HILINK_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *(*malloc)(unsigned int);
    void (*free)(void *);
} HILINK_MemHooks;

/*
 * 注册内存操作接口到HiLink SDK
 * 传入参数为用户注册的内存操作函数结构体指针
 * 返回0成功，返回非0失败
 * 注意：malloc和free接口必须成对注册
 */
int HILINK_RegisterMemHooks(const HILINK_MemHooks *memHooks);

#ifdef __cplusplus
}
#endif

#endif /* HILINK_INTERFACE_H */