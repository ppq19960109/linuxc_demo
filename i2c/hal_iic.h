#ifndef _HAL_IIC_H_
#define _HAL_IIC_H_

#include <stdint.h>

#define I2C_DEV_NAME "/dev/xm_i2c1"

typedef struct I2C_DATA {
    unsigned char dev_addr;
    unsigned int reg_addr;
    unsigned int addr_byte_num;
    unsigned int data;
    unsigned int data_byte_num;
} I2C_DATA_S;

int32_t i2c_write_one(uint8_t slave_addr, uint32_t reg_addr, uint8_t reg_addr_byte, uint32_t data, uint8_t data_byte);

int32_t i2c_read_one(uint8_t slave_addr, uint32_t reg_addr, uint8_t reg_addr_byte, uint32_t* data, uint8_t data_byte);

int32_t i2c_read(I2C_DATA_S* i2c_data, uint32_t* data, uint32_t data_len);

int32_t i2c_write(I2C_DATA_S* i2c_data, uint32_t* data, uint32_t data_len, uint32_t delay_time);

int32_t i2c_init(void);

int32_t i2c_deinit(void);

void i2c_test(void);

#endif
