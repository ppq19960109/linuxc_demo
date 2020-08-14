#include "client.h"
#include "protocol_cover.h"
#include <signal.h>
#include <time.h>

#define SERVER_PORT 9090

const char *heart =
    {"{ \
    \"Command\":\"TcpBeatHeart\", \
    \"Period\":\"60\" \
    }"};

void timer_thread(union sigval v)
{
    printf("timer_thread function! %s\n", heart);
}
void timer_signal_handler(int signal)
{
    printf("timer_signal_handler function! %s\n", heart);
    // writeToHaryan(heart,protocol_data.socketfd,protocol_data.sendData,256);
}
timer_t start_timer()
{
    struct sigaction act;
    act.sa_handler = timer_signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化

    // evp.sigev_value.sival_int = 1;                //也是标识定时器的，回调函数可以获得
    // evp.sigev_notify = SIGEV_THREAD;                //线程通知的方式，派驻新线程
    // evp.sigev_notify_function = timer_thread;       //线程函数地址
    evp.sigev_signo = SIGUSR1;
    evp.sigev_notify = SIGEV_SIGNAL;

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        return NULL;
    }

    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = 5; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = 2; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, NULL) == -1)
    {
        perror("fail to timer_settime");
        return NULL;
    }

    return timerid;
}

int net_client_srart()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    start_timer();
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    while (Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
    {
        sleep(2);
    }
    return sockfd;
}

void *thread_hander(void *arg)
{
    int *fd = ((int *)arg);
    while (1)
    {
        *fd = net_client_srart();
        int sockfd = *fd;
        char buf[256];
        int readLen, pos, step = 0;
        timer_t timerid = start_timer();
        while (1)
        {
            // memset(buf, 0, sizeof(buf));
            // while (step < 2)
            // {
            //     switch (step)
            //     {
            //     case 0:
            //         readLen = Recv(sockfd, buf, 1, 0);
            //         if (readLen == 1)
            //         {
            //             if (buf[0] == '{')
            //             {
            //                 step = 1;
            //                 pos = 1;
            //             }
            //         }
            //         break;
            //     case 1:
            //         readLen = Recv(sockfd, buf + pos, sizeof(buf) - pos, 0);
            //         if (readLen < 0)
            //         {
            //             return;
            //         }
            //         pos += readLen;
            //         if (pos >= 256)
            //         {
            //             step = 3;
            //             break;
            //         }
            //         if (buf[pos - 1] == '}')
            //         {
            //             step = 2;
            //         }
            //         break;
            //     }
            // }

            readLen = Recv(sockfd, buf, sizeof(buf), 0);
            // log_debug("Read:%s len:%d\n", buf, readLen);
            if (readLen == 0)
            {
                log_debug("client close");
                break;
            }
            else if (readLen < 0)
            {
                break;
            }
            else
            {
                printf("%s\n", &buf[1]);
                // Write(STDOUT_FILENO, buf, readLen);
                if (buf[0] == 0x02)
                    read_from_local(&buf[1]);
                // step = 0;
            }
        }
        close(sockfd);
        timer_delete(timerid);
    }
    pthread_exit(0);
}

void net_client(int *sockfd)
{

    pthread_t id;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&id, NULL, (void *)thread_hander, (void *)sockfd);
    //要将id分配出去。
    pthread_detach(id);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}