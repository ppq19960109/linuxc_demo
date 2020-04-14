#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "hal_iic.h"

#define CMD_I2C_WRITE 0x01
#define CMD_I2C_READ 0x03

static int32_t i2c_fd = -1;

int32_t i2c_read_one(uint8_t slave_addr, uint32_t reg_addr, uint8_t reg_addr_byte, uint32_t* data, uint8_t data_byte) {
    I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = slave_addr;
    i2c_data.reg_addr = reg_addr;
    i2c_data.addr_byte_num = reg_addr_byte;
    // data.data = *value;
    i2c_data.data_byte_num = data_byte;

    if (ioctl(i2c_fd, CMD_I2C_READ, &i2c_data)) {
        printf("i2c read failed.\r\n");
        return -1;
    }
    *data = i2c_data.data;
    return 0;
}

int32_t i2c_read(I2C_DATA_S* i2c_data, uint32_t* data, uint32_t data_len) {
    while (data_len--) {
        if (ioctl(i2c_fd, CMD_I2C_READ, i2c_data)) {
            printf("i2c read pos:%d failed.\n", data_len);
            return -1;
        }
        *data = i2c_data->data;
        data++;
        i2c_data->reg_addr++;
    }
    return 0;
}

int32_t i2c_write_one(uint8_t slave_addr, uint32_t reg_addr, uint8_t reg_addr_byte, uint32_t data, uint8_t data_byte) {
    I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = slave_addr;
    i2c_data.reg_addr = reg_addr;
    i2c_data.addr_byte_num = reg_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = data_byte;

    if (ioctl(i2c_fd, CMD_I2C_WRITE, &i2c_data)) {
        printf("i2c write failed.\r\n");
        return -1;
    }

    return 0;
}

int32_t i2c_write(I2C_DATA_S* i2c_data, uint32_t* data, uint32_t data_len, uint32_t delay_time) {
    while (data_len--) {
        i2c_data->data = *data;
        if (ioctl(i2c_fd, CMD_I2C_WRITE, i2c_data)) {
            printf("i2c write pos:%d failed.\n", data_len);
            return -1;
        }
        data++;
        i2c_data->reg_addr++;
        usleep(delay_time);
    }
    return 0;
}

int32_t i2c_init(void) {
    i2c_fd = open(I2C_DEV_NAME, O_RDWR);
    if (i2c_fd < 0) {
        printf("%s open failed\r\n", I2C_DEV_NAME);
        return -1;
    }
    return 0;
}

int32_t i2c_deinit(void) {
    if (i2c_fd < 0) {
        return -1;
    }

    close(i2c_fd);
    i2c_fd = -1;
    return 0;
}

static void printf_array(uint32_t* array, uint32_t array_len) {
    int i;
    printf("array: \n");
    for (i = 0; i < array_len; i++) {
        printf(" %x", array[i]);
    }
    printf("\n");
}

void i2c_test(void) {
    uint32_t wr[5] = {2, 3, 4, 6, 9};
    uint32_t re[5] = {0};
    if (i2c_init()) {
        return;
    }

    I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = 0x50;
    i2c_data.reg_addr = 0;
    i2c_data.addr_byte_num = 1;
    i2c_data.data = 0;
    i2c_data.data_byte_num = 1;

    i2c_write(&i2c_data, wr, 5, 5000);
    sleep(1);

    i2c_read(&i2c_data, re, 5);

    printf_array(re, 5);
    i2c_deinit();
}
