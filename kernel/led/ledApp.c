#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
 文件名 : ledApp.c
 作者 :
 版本 : V1.0
 描述 : platform 驱动驱测试 APP。
 其他 : 无
 使用方法 ： ./ledApp /dev/platled 0 关闭 LED
 ./ledApp /dev/platled 1 打开 LED
 论坛 : www.openedv.com
 日志 : 初版 V1.0 2019/8/16 
 ***************************************************************/
#define LEDOFF 0
#define LEDON 1

/*
 * @description : main 主程序
 * @param - argc : argv 数组元素个数
 * @param - argv : 具体参数
 * @return : 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    unsigned char databuf[2];

    if (argc != 3)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    /* 打开 led 驱动 */
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }

    databuf[0] = atoi(argv[2]); /* 要执行的操作：打开或关闭 */
    retvalue = write(fd, databuf, sizeof(databuf));
    if (retvalue < 0)
    {
        printf("LED Control Failed!\r\n");
        close(fd);
        return -1;
    }

    retvalue = close(fd); /* 关闭文件 */
    if (retvalue < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}