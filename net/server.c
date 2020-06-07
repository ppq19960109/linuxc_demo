#include "server.h"

int net_server_srart()
{
    struct sockaddr_in server;
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;                //簇
    server.sin_port = htons(SERVER_PORT);       //端口
    server.sin_addr.s_addr = htonl(INADDR_ANY); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        return -1;
    }
    Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    Listen(sockfd, 128);
    return sockfd;
}

void net_server()
{
    char buf[255];
    int readLen;
    struct sockaddr_in server_client;
    socklen_t addrlen;

    int sockfd = net_server_srart();
    while (1)
    {
        int clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
        if (clientfd != -1)
        {
            log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));
            while (1)
            {
                memset(buf, 0, sizeof(buf));
                readLen = Read(clientfd, buf, sizeof(buf));
                log_debug("Read:%s len:%d\n", buf, readLen);
                if (readLen == 0)
                {
                    log_debug("client close");
                    break;
                }
                else if (readLen < 0)
                {
                }
                else
                {
                    Write(clientfd, buf, readLen);
                }
            }
        }
    }
    Close(sockfd);
}

void catch_child(int sig)
{
    while (waitpid(0, NULL, WNOHANG) > 0)
        ;
    //pid = -1  等待任何子进程,相当于 wait()
    //WNOHANG   若pid指定的子进程没有结束，则waitpid()函数返回0，不予以等待。若结束，则返回该子进程的ID
}

void net_server_fork()
{
    char buf[255];
    int readLen;
    struct sockaddr_in server_client;
    socklen_t addrlen;

    int sockfd = net_server_srart();

    // signal(SIGCHLD, catch_child);
    struct sigaction act;
    act.sa_handler = catch_child; //信号响应函数
    sigemptyset(&act.sa_mask);    //清空该操作
    act.sa_flags = 0;             //选择第一种函数指针 sa_handler

    if (sigaction(SIGCHLD, &act, NULL) != 0) //注册信号，需要捕捉的信号：SIGCHLD。act是捕捉到该信号后的反应
    {
        perr_exit("sigaction error");
    }
    while (1)
    {
        int clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
        if (clientfd != -1)
        {
            log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));
            pid_t pid = fork();
            if (pid < 0)
            {
                perr_exit("fork error");
            }
            else if (pid == 0)
            {
                Close(sockfd);
                while (1)
                {
                    memset(buf, 0, sizeof(buf));
                    readLen = Read(clientfd, buf, sizeof(buf));
                    log_debug("Read:%s len:%d\n", buf, readLen);
                    if (readLen == 0)
                    {
                        log_debug("client close");
                        break;
                    }
                    else if (readLen < 0)
                    {
                    }
                    else
                    {
                        Write(clientfd, buf, readLen);
                    }
                }
                Close(clientfd);
                return;
            }
            else
            {

                Close(clientfd);
            }
        }
    }
    Close(sockfd);
}

void *thread_hander(void *arg)
{
    int sockfd = *((int *)arg);
    char buf[255];
    int readLen;
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        readLen = Read(sockfd, buf, sizeof(buf));
        // log_debug("Read:%s len:%d\n", buf, readLen);
        if (readLen == 0)
        {
            log_debug("client close");
            break;
        }
        else if (readLen < 0)
        {
        }
        else
        {
            Write(STDOUT_FILENO, buf, readLen);
            Write(sockfd, buf, readLen);
        }
    }
    pthread_exit(0);
    return (void *)0;
}
void net_server_pthread()
{

    struct sockaddr_in server_client;
    socklen_t addrlen;

    int sockfd = net_server_srart();
    while (1)
    {
        int clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
        if (clientfd != -1)
        {
            log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));
            pthread_t id;
            //clientMethod为此线程客户端，要执行的程序。
            pthread_create(&id, NULL, (void *)thread_hander, (void *)&clientfd);
            //要将id分配出去。
            pthread_detach(id);
        }
    }
    Close(sockfd);
}
void *thread_hander_select(void *arg)
{
    int sockfd = *((int *)arg);
    char buf[255];
    int readLen;
    int maxfd, ret;
    fd_set rfdset, allfdset;
    maxfd = sockfd;
    FD_ZERO(&allfdset);
    FD_SET(sockfd, &allfdset);

    while (1)
    {
        rfdset = allfdset;
        ret = select(maxfd + 1, &rfdset, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
        if (ret < 0)
        {
            perror("select error!");
            break;
        }
        else if (ret == 0)
        {
            printf("timeout\n");
            continue;
        }
        else
        {
            memset(buf, 0, sizeof(buf));
            readLen = Read(sockfd, buf, sizeof(buf));
            // log_debug("Read:%s len:%d\n", buf, readLen);
            if (readLen == 0)
            {
                log_debug("client close");
                break;
            }
            else if (readLen < 0)
            {
            }
            else
            {
                Write(STDOUT_FILENO, buf, readLen);
                Write(sockfd, buf, readLen);
            }
        }
    }
    pthread_exit(0);
    return (void *)0;
}

void net_server_pthread_select()
{
    int maxfd, ret;
    struct sockaddr_in server_client;
    socklen_t addrlen;
    fd_set rfdset, allfdset;

    int sockfd = net_server_srart();
    maxfd = sockfd;
    FD_ZERO(&allfdset);
    FD_SET(sockfd, &allfdset);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    while (1)
    {
        rfdset = allfdset;
        ret = select(maxfd + 1, &rfdset, (fd_set *)0, (fd_set *)0, &timeout);
        if (ret < 0)
        {
            perror("select error!");
            break;
        }
        else if (ret == 0)
        {
            // printf("timeout\n");
            continue;
        }
        else
        {
            if (FD_ISSET(sockfd, &rfdset))
            {
                int clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
                if (clientfd != -1)
                {
                    log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));
                    pthread_t id;
                    //clientMethod为此线程客户端，要执行的程序。
                    pthread_create(&id, NULL, (void *)thread_hander_select, (void *)&clientfd);
                    //要将id分配出去。
                    pthread_detach(id);
                }
            }
        }
    }
    Close(sockfd);
}

void net_server_select()
{
    char buf[255];
    int readLen;
    int maxfd, ret;
    struct sockaddr_in server_client;
    socklen_t addrlen;
    fd_set rfdset, allfdset;

    int sockfd = net_server_srart();
    maxfd = sockfd;
    FD_ZERO(&allfdset);
    FD_SET(sockfd, &allfdset);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    while (1)
    {
        rfdset = allfdset;
        ret = select(maxfd + 1, &rfdset, (fd_set *)0, (fd_set *)0, &timeout);
        if (ret < 0)
        {
            perror("select error!");
            break;
        }
        else if (ret == 0)
        {
            // printf("timeout\n");
            continue;
        }
        else
        {
            if (FD_ISSET(sockfd, &rfdset))
            {
                int clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
                if (clientfd != -1)
                {
                    log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));

                    FD_SET(clientfd, &allfdset);
                    if (maxfd < clientfd)
                        maxfd = clientfd;

                    if (ret == 1)
                        continue;
                }
            }

            for (int i = sockfd + 1; i < maxfd + 1; i++)
            {
                if (FD_ISSET(i, &rfdset))
                {
                    memset(buf, 0, sizeof(buf));
                    readLen = Read(i, buf, sizeof(buf));
                    // log_debug("Read:%s len:%d\n", buf, readLen);
                    if (readLen == 0)
                    {
                        log_debug("client close");
                        FD_CLR(i, &allfdset);
                        break;
                    }
                    else if (readLen < 0)
                    {
                    }
                    else
                    {
                        Write(STDOUT_FILENO, buf, readLen);
                        Write(i, buf, readLen);
                    }
                }
            }
        }
    }
    Close(sockfd);
}

void net_server_select_array()
{
    char buf[255], str[INET_ADDRSTRLEN];
    int readLen, client[FD_SETSIZE];
    int maxfd, ret, maxi = -1, i;
    struct sockaddr_in server_client;
    socklen_t addrlen;
    fd_set rfdset, allfdset;
    int clientfd;

    for (int i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    int sockfd = net_server_srart();
    maxfd = sockfd;
    FD_ZERO(&allfdset);
    FD_SET(sockfd, &allfdset);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    while (1)
    {
        rfdset = allfdset;
        ret = select(maxfd + 1, &rfdset, (fd_set *)0, (fd_set *)0, &timeout);
        if (ret < 0)
        {
            perror("select error!");
            break;
        }
        else if (ret == 0)
        {
            // printf("timeout\n");
            continue;
        }
        else
        {
            if (FD_ISSET(sockfd, &rfdset))
            {
                clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
                if (clientfd != -1)
                {
                    log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));

                    for (i = 0; i < FD_SETSIZE; i++)
                    {
                        if (client[i] < 0)
                        {
                            client[i] = clientfd;
                            break;
                        }
                        if (i == FD_SETSIZE - 1)
                        {
                            perr_exit("client many\n");
                        }
                    }

                    if (i > maxi)
                        maxi = i;

                    FD_SET(clientfd, &allfdset);
                    if (maxfd < clientfd)
                        maxfd = clientfd;

                    if (--ret == 0)
                        continue;
                }
            }

            for (int i = 0; i <= maxi; i++)
            {
                clientfd = client[i];
                if (clientfd < 0)
                    continue;

                if (FD_ISSET(clientfd, &rfdset))
                {
                    memset(buf, 0, sizeof(buf));
                    readLen = Read(clientfd, buf, sizeof(buf));
                    // log_debug("Read:%s len:%d\n", buf, readLen);
                    if (readLen == 0)
                    {
                        Close(clientfd);
                        log_debug("client close");
                        FD_CLR(clientfd, &allfdset);
                        client[i] = -1;
                        break;
                    }
                    else if (readLen < 0)
                    {
                    }
                    else
                    {
                        Write(STDOUT_FILENO, buf, readLen);
                        Write(clientfd, buf, readLen);
                    }
                    if (--ret == 0)
                        break;
                }
            }
        }
    }
    Close(sockfd);
}

void net_server_poll()
{
    char buf[255];
    int readLen;
    struct sockaddr_in server_client;
    socklen_t addrlen;
    int clientfd;
    int i, ret;

    struct pollfd peerfd[1024];
    int nfds = 1;

    for (i = 0; i < FD_SETSIZE; i++)
        peerfd[i].fd = -1;

    int sockfd = net_server_srart();

    peerfd[0].fd = sockfd;
    peerfd[0].events = POLLIN;

    while (1)
    {

        ret = poll(peerfd, nfds, -1);
        if (ret < 0)
        {
            perror("poll error!");
            break;
        }
        else if (ret == 0)
        {
            // printf("timeout\n");
            continue;
        }
        else
        {
            if (peerfd[0].revents & POLLIN)
            {
                clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
                if (clientfd != -1)
                {
                    log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));

                    for (i = 1; i < FD_SETSIZE; i++)
                    {
                        if (peerfd[i].fd < 0)
                        {
                            peerfd[i].fd = clientfd;
                            break;
                        }
                        if (i == FD_SETSIZE - 1)
                        {
                            perr_exit("many client\n");
                            Close(clientfd);
                        }
                    }
                    peerfd[i].events = POLLIN;

                    if (i + 1 > nfds)
                        nfds = i + 1;

                    if (--ret == 0)
                        continue;
                }
            }

            for (i = 1; i < nfds; i++)
            {
                clientfd = peerfd[i].fd;
                if (clientfd < 0)
                    continue;

                if (peerfd[i].revents & POLLIN)
                {
                    memset(buf, 0, sizeof(buf));
                    readLen = Read(clientfd, buf, sizeof(buf));
                    // log_debug("Read:%s len:%d\n", buf, readLen);
                    if (readLen == 0)
                    {
                        Close(clientfd);
                        log_debug("client close");
                        // if (i + 1 == nfds)
                        //     nfds--;
                        peerfd[i].fd = -1;
                        break;
                    }
                    else if (readLen < 0)
                    {
                        if (errno == ECONNRESET)
                        {
                            Close(clientfd);
                            peerfd[i].fd = -1;
                           
                        }
                    }
                    else
                    {
                        Write(STDOUT_FILENO, buf, readLen);
                        Write(clientfd, buf, readLen);
                    }
                    if (--ret == 0)
                        break;
                }
            }
        }
    }
    Close(sockfd);
}

void net_server_epoll()
{
    char buf[255];
    int readLen;
    struct sockaddr_in server_client;
    socklen_t addrlen;
    int clientfd;
    int i, evnums;

    int sockfd = net_server_srart();

    int epollfd = epoll_create(256);
    if (epollfd < 0)
    {
        perr_exit("epoll_create\n");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
    {
        perr_exit("epoll_ctl\n");
    }
    struct epoll_event evs[256];
    while (1)
    {
        evnums = epoll_wait(epollfd, evs, 256, -1);
        if (evnums < 0)
        {
            perror("epoll error!");
            break;
        }
        else if (evnums == 0)
        {
            // printf("timeout\n");
            continue;
        }
        else
        {
            for (i = 0; i < evnums; ++i)
            {
                if (evs[i].data.fd == sockfd && evs[i].events & EPOLLIN)
                {
                    clientfd = Accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
                    if (clientfd != -1)
                    {
                        log_debug("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));

                        ev.data.fd = clientfd;
                        ev.events = EPOLLIN;
                        epoll_ctl(epollfd, EPOLL_CTL_ADD,
                                  clientfd, &ev);
                    }
                }
                else if (evs[i].events & EPOLLIN)
                {
                    clientfd = evs[i].data.fd;
                    memset(buf, 0, sizeof(buf));
                    readLen = Read(clientfd, buf, sizeof(buf));
                    // log_debug("Read:%s len:%d\n", buf, readLen);
                    if (readLen == 0)
                    {
                        Close(clientfd);
                        log_debug("client close");

                        epoll_ctl(epollfd, EPOLL_CTL_DEL,
                                  evs[i].data.fd, NULL);
                        break;
                    }
                    else if (readLen < 0)
                    {
                        if (errno == ECONNRESET)
                        {
                            Close(clientfd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL,
                                      evs[i].data.fd, NULL);
                            break;
                        }
                    }
                    else
                    {
                        Write(STDOUT_FILENO, buf, readLen);
                        Write(clientfd, buf, readLen);
                    }
                }
                else
                {
                }
            }
        }
    }
    Close(sockfd);
}