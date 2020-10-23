#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>

#define DIR_OUT_FILE "/userdata/app/daemon.log"

int init_daemon(void)
{
    int pid;
    int i;

    //忽略终端I/O信号，STOP信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid > 0)
    {
        exit(0); //结束父进程，使得子进程成为后台进程
    }
    else if (pid < 0)
    {
        return -1;
    }

    //建立一个新的进程组,在这个新的进程组中,子进程成为这个进程组的首进程,以使该进程脱离所有终端
    setsid();

    //再次新建一个子进程，退出父进程，保证该进程不是进程组长，同时让该进程无法再打开一个新的终端
    pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    else if (pid < 0)
    {
        return -1;
    }

    //将标准输入输出重定向到空设备

    // int fd;
    // fd = open("/dev/null", O_RDWR, 0);
    // if (fd != -1)
    // {
    //     dup2(fd, STDIN_FILENO);
    //     dup2(fd, STDOUT_FILENO);
    //     dup2(fd, STDERR_FILENO);
    //     if (fd > 2)
    //     {
    //         close(fd);
    //     }
    // }

    //关闭所有从父进程继承的不再需要的文件描述符
    for (i = 3; i < NOFILE; close(i++))
        ;

    //改变工作目录，使得进程不与任何文件系统联系
    chdir("/");

    //将文件当时创建屏蔽字设置为0
    umask(0);

    //忽略SIGCHLD信号
    signal(SIGCHLD, SIG_IGN);

    return 0;
}

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL)
        return filesize;
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fclose(fp);
    return filesize;
}

int get_pid(char *Name)
{
    char buf[4] = {0};
    char cmd[64] = {0};

    sprintf(cmd, "%s", DIR_OUT_FILE);
    if ((access(cmd, F_OK)) != 0)
    {
        sprintf(cmd, "touch %s", DIR_OUT_FILE);
        system(cmd);
    }
    sprintf(cmd, "pidof %s | awk '{print NF}' > %s", Name, DIR_OUT_FILE);
    system(cmd);

    int fd = open(DIR_OUT_FILE, O_CREAT | O_RDONLY, 0777);
    int nread = read(fd, buf, sizeof(buf));
    close(fd);
    // syslog(LOG_USER | LOG_INFO, "read:%d,%d,%d,get_file_size:%ld \n", nread,buf[0],buf[1], get_file_size(DIR_OUT_FILE));
    if (nread > 2)
        return -1;
    return buf[0];
}

struct pidInfo_t
{
    char *pidName;
    char *pidPath;
};

static const struct pidInfo_t pidInfo[] = {
    {.pidName = "hydevapp", .pidPath = "/userdata/hyapp"},
    {.pidName = "hy_server_iot", .pidPath = "/userdata/iotapp"},
    {.pidName = "hilinkapp", .pidPath = "/userdata/app"},
};

int main()
{
    char cmd[64] = {0};
    int i, pidCount;
    const int pidInfoNum = sizeof(pidInfo) / sizeof(pidInfo[0]);

    time_t now;
    init_daemon();
    syslog(LOG_USER | LOG_INFO, "DaemonProcess Start! \n");

    while (1)
    {
        sleep(60);

        signal(SIGCHLD, SIG_DFL);
        for (i = 0; i < pidInfoNum; i++)
        {
            pidCount = get_pid(pidInfo[i].pidName);

            if (pidCount != '1')
            {
                time(&now);
                syslog(LOG_USER | LOG_INFO, "SystemTime: \t%s\t\n", ctime(&now));
                syslog(LOG_USER | LOG_INFO, "%s pidCount %d\n", pidInfo[i].pidName, pidCount);
                if (pidCount == 0)
                {
                    sprintf(cmd, "%s/%s", pidInfo[i].pidPath, pidInfo[i].pidName);
                    if ((access(cmd, F_OK)) == 0)
                    {
                        sprintf(cmd, "cd %s;./%s &", pidInfo[i].pidPath, pidInfo[i].pidName);
                    }
                    else
                    {
                        sprintf(cmd, "cd /userdata/update/;./upgrade_backup.bin");
                    }
                    system(cmd);
                }
                else
                {
                    sprintf(cmd, "killall %s", pidInfo[i].pidName);
                    system(cmd);
                    sprintf(cmd, "cd %s;./%s &", pidInfo[i].pidPath, pidInfo[i].pidName);
                    system(cmd);
                }
            }
        }
        signal(SIGCHLD, SIG_IGN);
    }
}