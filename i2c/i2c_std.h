#ifndef _I2C_STD_H_
#define _I2C_STD_H_

#include <stdint.h>

#define SA 0x50
#define _I2C_DEV "/dev/xm_i2c1"

int32_t _i2c_init(const char *i2c_dev);

void _i2c_close(void);

int32_t _i2c_read(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_byte, uint8_t *data, uint32_t data_len);

int32_t _i2c_write(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_byte, uint8_t *data, uint32_t data_len);

#endif