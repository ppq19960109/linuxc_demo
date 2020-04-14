#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2c_std.h"

static int _i2c_fd = -1;

int32_t _i2c_read(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_byte, uint8_t *data, uint32_t data_len) {
    int ret;
    uint8_t buf[4] = {0};
    struct i2c_rdwr_ioctl_data i2c_data;

    i2c_data.nmsgs = 2;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        printf("i2c_data.msgs malloc error");
        return -1;
    }

    // write reg
    if (reg_byte == 4) {
        buf[0] = reg_addr >> 24;
        buf[1] = reg_addr >> 16;
        buf[2] = reg_addr >> 8;
        buf[3] = reg_addr;
    } else if (reg_byte == 2) {
        buf[0] = reg_addr >> 8;
        buf[1] = reg_addr;
    } else {
        buf[0] = reg_addr;
    }

    i2c_data.msgs[0].len = reg_byte;
    i2c_data.msgs[0].addr = dev_addr;
    i2c_data.msgs[0].flags = 0;  // 0: write 1:read
    i2c_data.msgs[0].buf = buf;
    // read data
    i2c_data.msgs[1].len = data_len;
    i2c_data.msgs[1].addr = dev_addr;
    i2c_data.msgs[1].flags = I2C_M_RD;  // 0: write 1:read
    i2c_data.msgs[1].buf = data;

    ret = ioctl(_i2c_fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        printf("read data %x %x error\r\n", dev_addr, reg_addr);
        free(i2c_data.msgs);
        return 1;
    }
    free(i2c_data.msgs);

    return 0;
}

int32_t _i2c_write(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_byte, uint8_t *data, uint32_t data_len) {
    int ret;
    uint8_t *buf;
    struct i2c_rdwr_ioctl_data i2c_data;

    i2c_data.nmsgs = 1;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL) {
        printf("i2c_data.msgs malloc error");
        return -1;
    }

    buf = malloc(data_len + reg_byte);
    if (buf == NULL) {
        printf("buf malloc error");
        return -1;
    }
    memset(buf, 0, data_len + reg_byte);
    // write reg
    if (reg_byte == 4) {
        buf[0] = reg_addr >> 24;
        buf[1] = reg_addr >> 16;
        buf[2] = reg_addr >> 8;
        buf[3] = reg_addr;
    } else if (reg_byte == 2) {
        buf[0] = reg_addr >> 8;
        buf[1] = reg_addr;
    } else {
        buf[0] = reg_addr;
    }

    memcpy(buf + reg_byte, data, data_len);

    i2c_data.msgs[0].len = data_len + reg_byte;
    i2c_data.msgs[0].addr = dev_addr;
    i2c_data.msgs[0].flags = 0;  // 0: write 1:read
    i2c_data.msgs[0].buf = buf;

    ret = ioctl(_i2c_fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0) {
        printf("write data %x %x error\r\n", dev_addr, reg_addr);
        free(i2c_data.msgs);
        return 1;
    }
    free(i2c_data.msgs);

    return 0;
}

int32_t _i2c_init(const char *i2c_dev) {
    _i2c_fd = open(i2c_dev, O_RDWR);
    if (_i2c_fd < 0) {
        printf("i2c open %s fail.\n", i2c_dev);
        return -1;
    }
    ioctl(_i2c_fd, I2C_TIMEOUT, 1);
    ioctl(_i2c_fd, I2C_RETRIES, 2);
    return 0;
}

void _i2c_close(void) {
    if (_i2c_fd >= 0) {
        close(_i2c_fd);
        _i2c_fd = -1;
    }
}

static void printf_array(uint8_t *array, uint32_t array_len) {
    int i;
    printf("array: \n");
    for (i = 0; i < array_len; i++) {
        printf(" %x", array[i]);
    }
    printf("\n");
}

int32_t _i2c_test(void) {
    uint8_t wr[5] = {2, 3, 4, 6, 9};
    uint8_t re[5] = {0};

    if (_i2c_init(_I2C_DEV)) {
        return -1;
    }

    _i2c_write(SA, 0x00, 1, wr, 5);
    sleep(1);
    _i2c_read(SA, 0x00, 1, re, 5);
    printf_array(re, 5);
    return 0;
}