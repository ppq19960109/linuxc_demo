#include <stdio.h>
#include <stdlib.h>

static int CalCrc(int crc, const char *buf, int len)
{
    unsigned int byte;
    unsigned char k;
    unsigned short ACC, TOPBIT;
    unsigned short remainder = 0x0000;
    // unsigned short remainder = crc;
    TOPBIT = 0x8000;
    for (byte = 0; byte < len; ++byte)
    {
        ACC = buf[byte];
        remainder ^= (ACC << 8);
        for (k = 8; k > 0; --k)
        {
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ 0x8005;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    remainder = remainder ^ 0x0000;
    return remainder;
}
const char data[] = {};
int main(int agrc, char *agrv[])
{
    printf("main start \n");
    // int* a=1;

    // int crc=CalCrc(0,data,sizeof(data));
    // printf("crc value:%x len:%x\n",crc,sizeof(data));
    // printf("main end %d\n",&(*a));
    return 0;
}
