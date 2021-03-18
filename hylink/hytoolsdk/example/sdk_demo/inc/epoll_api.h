/***********************************************************
*文件名     : epoll_api.h
*版   本   : v1.0.0.0
*日   期   : 2018.09.14
*说   明   : epoll事件监听
*修改记录: 
************************************************************/

#ifndef EPOLL_API_H
#define EPOLL_API_H

#include <sys/epoll.h>

typedef struct epoll_event_s
{
	int iFd;
	unsigned int uiEvent;
}epoll_event_t;

typedef struct epoll_fd_s
{
	int iEpollFd;
	int iEpollEventMaxNum;
	base_epoll_event_t *pastWaitevents;
	
	/*methods*/
	/*初始化监听事件*/
	int (*init_event)(struct epoll_fd_s *, int);
	/*监听读事件*/
	int (*read_event)(struct epoll_fd_s *, int);
	/*监听写事件*/
	int (*write_event)(struct epoll_fd_s *, int);
	/*监听读写事件*/
	int (*read_write_event)(struct epoll_fd_s *, int);
	/*删除监听事件*/
	int (*del_event)(struct epoll_fd_s *, int);

	/*获取当前发生的时间*/
	int (*happen_event)(struct epoll_fd_s *, epoll_event_t *, int *, int);
}epoll_fd_t;
/*构造函数*/
epoll_fd_t *new_epoll_fd(int epoll_fd_s);

/*析构函数*/
int destroy_epoll_fd(epoll_fd_t *_this);

#endif /* EPOLL_API_H */

