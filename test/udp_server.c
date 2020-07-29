// 接收端
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define BROADCAST_PORT 9999
int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    fflush(stdout);

    // 绑定地址
    struct sockaddr_in addrto;
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_ANY);
    addrto.sin_port = htons(BROADCAST_PORT);

    int sock = -1, ret;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket error\n");
        return -1;
    }
    ret = bind(sock, (struct sockaddr *)&(addrto), sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        perror("bind error\n");
        return 0;
    }

    socklen_t rlen;
    char rmsg[128] = {0};
    struct sockaddr_in from;
    while (1)
    {
        //从广播地址接受消息
        int ret = recvfrom(sock, rmsg, sizeof(rmsg), 0, (struct sockaddr *)&from, &rlen);
        if (ret <= 0)
        {
            perror("recvfrom error\n");
            continue;
        }
        printf("%s ip:%s,port:%d\n", rmsg, inet_ntoa(from.sin_addr),
               htons(from.sin_port));
        sleep(1);
    }

    return 0;
}