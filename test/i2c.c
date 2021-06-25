#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h> //open close
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>
#include <stdint.h>

#define I2C_DEV "/dev/i2c-0"
#define CHIP_ADDR 0x2a

void log_printf_array(unsigned char *array, int len)
{
    printf("\n");
    for (int i = 0; i < len; ++i)
    {
        printf(" %d", array[i]);
    }
    printf("\n");
}
int i2c_read_reg(void)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;

    char buf[256] = {0x01, 0x01, 0x00};

    // 1.打开设备
    int fd = open(I2C_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("err.open i2c fail.%s", strerror(errno));
        return -1;
    }

    data.msgs = (struct i2c_msg *)malloc(2 * sizeof(struct i2c_msg));

    //2.写入reg,读取数据
    data.nmsgs = 2;                //消息的数目
    data.msgs[0].len = 3;          //写入1byte
    data.msgs[0].addr = CHIP_ADDR; //从设备地址
    data.msgs[0].flags = 0;        //flags-0:写 1:读.
    data.msgs[0].buf = buf;
    // data.msgs[0].buf[0] = 0x01;		//reg

    data.msgs[1].len = 16;         //读取32bytes
    data.msgs[1].addr = CHIP_ADDR; //从设备地址
    data.msgs[1].flags = I2C_M_RD; //flags-0:写 1:读.
    data.msgs[1].buf = (unsigned char *)malloc(16);
    memset(data.msgs[1].buf, 0, 16);

    //3.使用ioctl写入&读取数据
    ret = ioctl(fd, I2C_RDWR, (unsigned long)&data);
    if (ret < 0)
    {
        printf("ioctl: %s\n", strerror(errno));
        goto __err;
    }

    // printf("read:%s\n", data.msgs[1].buf);
    log_printf_array(data.msgs[1].buf, 16);
__err:
    free(data.msgs[1].buf);
    free(data.msgs);
    //4.关闭设备
    close(fd);
    return ret;
}
int i2c_only_write(void)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;
    char buf[256] = {0x01, 0x01, 0x00};
    // 1.打开设备
    int fd = open(I2C_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("err\n");
        return -1;
    }

    data.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg));

    // 2.读取数据
    data.nmsgs = 1;
    data.msgs[0].len = 1;
    data.msgs[0].addr = CHIP_ADDR; //从设备地址
    data.msgs[0].flags = 0;        //flags-0:写 1:读.
    data.msgs[0].buf = buf;

    // 3.使用ioctl读数据
    ret = ioctl(fd, I2C_RDWR, (unsigned long)&data);
    if (ret < 0)
    {
        printf("ioctl: %s\n", strerror(errno));
        return -1;
    }

    free(data.msgs);
    //4.关闭设备
    close(fd);
    return ret;
}
int i2c_only_read(void)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;

    // 1.打开设备
    int fd = open(I2C_DEV, O_RDWR);
    if (fd < 0)
    {
        printf("err\n");
        return -1;
    }

    data.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg));

    // 2.读取数据
    data.nmsgs = 1;
    data.msgs[0].len = 16;
    data.msgs[0].addr = CHIP_ADDR; //从设备地址
    data.msgs[0].flags = I2C_M_RD; //flags-0:写 1:读.
    data.msgs[0].buf = (unsigned char *)malloc(16);

    // 3.使用ioctl读数据
    ret = ioctl(fd, I2C_RDWR, (unsigned long)&data);
    if (ret < 0)
    {
        printf("ioctl: %s\n", strerror(errno));
        return -1;
    }

    // printf("read:%s\n", data.msgs[0].buf);
    log_printf_array(data.msgs[0].buf, 16);
    free(data.msgs[0].buf);
    free(data.msgs);
    //4.关闭设备
    close(fd);
    return ret;
}
