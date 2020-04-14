#include "receiveData.h"

struct temperature_info {
    int max;
    int min;
    int max_x;
    int max_y;
    int min_x;
    int min_y;
};

struct temperature_info temp_data;

int rev_state = 0;
char rev_buf[1544];

extern int serial_read(char*, int);

int receiveInit() { return 0; }

void receiveData() {
    char buf[4];
    int cnt = 0;
    //选择接收状态
    switch (rev_state) {
        //状态=0时，为空闲状态，检测起始标志位
        case 0:
            cnt = 0;

            serial_read(buf, 1);
            if (buf[0] == 0x5a) {
                //如果读到数据是起始位0x5a，更改状态为1
                rev_state = 1;
                break;
            }
            cnt++;

            break;
        //状态=1时，继续检测剩余起始标志位
        case 1:
            //读取接下来的三个字节，判断是否符合
            serial_read(buf, 3);
            if (buf[0] == 0x5a && buf[1] == 0x02 && buf[2] == 0x06) {
                //标志位校验成功，进入读取数据状态
                rev_state = 2;
                break;
            } else {
                //校验位校验失败，进入空闲状态继续检测
                rev_state = 0;
            }
            break;
            //状态=2时，读取剩余1540个数据
        case 2:
            serial_read(rev_buf, 1540);
            rev_state = 0;
            //数据读取完成，进入处理程序
            rev_process();
            break;
        default:
            break;
    }
}

void calculating_temperature(char* ptr, char* out, struct temperature_info* temp_data) {
    //初始化最大最小值
    temp_data->max = 0;
    temp_data->min = 500;
    int inmax = 0;
    int inmin = 500;
    for (int i = 0; i < 769; i++) {
        //读取高八位
        unsigned short dh = ptr[i * 2 + 1];
        dh <<= 8;
        //读取低八位
        dh |= ptr[i * 2];
        short rul = (short)dh;

        //计算温度值
        out[i] = rul / 100.0f;

        if (i < 768) {
            if (out[i] > 60) {
                out[i] = out[i - 1];
            }
            //计算温度最大值
            if (out[i] > inmax) {
                inmax = out[i];

                //最大值坐标
                dis1->max = inmax;
                dis1->max_x = (i % 32) * 10;
                dis1->max_y = (i / 32) * 10;
                dis2->max = inmax;
                dis2->max_x = (i % 32) * 10;
                dis2->max_y = (i / 32) * 10;
            }
            //计算最小值
            if (Tep[i] < inmin) {
                inmin = Tep[i];
                //最小值坐标
                dis1->min = inmin;
                dis1->min_x = (i % 32) * 10;
                dis1->min_y = (i / 32) * 10;
                dis2->min = inmin;
                dis2->min_x = (i % 32) * 10;
                dis2->min_y = (i / 32) * 10;
            }
        }
    }
}

void rev_process() {
    calculating_temperature(rev_buf);

    //插值运算，将32*24的温度图像放大成320*240的温度图像
    for (uint i = 0; i < 24; i++) {
        for (uint j = 0; j < 32; j++) {
            for (uint k = 0; k < 10; k++) {
                for (uint m = 0; m < 10; m++) {
                    //防止溢出
                    float tep = Tep[i * 32 + j];
                    if (tep > max)
                        tep = max;
                    else if (tep < min)
                        tep = min;
                    //温度到颜色区间转换
                    MColor mcl = GetColor(tep, max, min);
                    //原始图像填充
                    img1->setPixel(j * 10 + m, i * 10 + k, qRgba(mcl.r, mcl.g, mcl.b, 255));
                }
            }
        }
    }
    //高斯滤波生成新的图像
    getBlurImage(img1);
}