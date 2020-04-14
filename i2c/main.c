#include "hal_iic.h"
#include "i2c_std.h"
#include "pn.h"
#include "pn512.h"
int main(int agrc, char *agrv[]) {
    uint32_t re[5] = {0};
    printf("main start \n");

    if (i2c_init()) {
        return -1;
    }
    // PN512_Init(0);
        I2C_DATA_S i2c_data = {0};
    i2c_data.dev_addr = 0xa0>>1;
    i2c_data.reg_addr = 0;
    i2c_data.addr_byte_num = 1;
    i2c_data.data = 0;
    i2c_data.data_byte_num = 1;

    while(1)
    {
    //    PN512_READ_IC();



    sleep(1);

    i2c_read(&i2c_data, re, 1);
printf("array:%x \n",re[0]);
    }
    i2c_deinit();
    return 0;
}