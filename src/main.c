#include "commom.h"
#include "event_app.h"
#include "qrcode.h"

int main(int agrc, char *agrv[]) {
    printf("main start \n");
    //  qrcode_test_init();
    //  event_app_init();
uint32_t a=1;
uint32_t b=2;
if(a-b>0){
printf("a>b \n");
}else
{
    printf("a<b \n");
}

 printf("main end :%d \n", a-b);
    return 0;
}