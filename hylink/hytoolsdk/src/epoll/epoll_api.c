/***********************************************************
*文件名     : epoll_api.c
*版   本   : v1.0.0.0
*日   期   : 2018.09.14
*说   明   : epoll事件监听
*修改记录: 
************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "error_msg.h"
#include "base_api.h"
#include "epoll_api.h"
#include "log_api.h"

/*初始化epoll监听事件(默认监听读事件)*/
int 
epoll_init_event(epoll_fd_t *_this, int iEventfd)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iEventfd <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	base_epoll_event_t stEpollEvent;
	base_memset(&stEpollEvent, 0x0, sizeof(base_epoll_event_t));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	
	if(base_epoll_ctl(_this->iEpollFd, EPOLL_CTL_ADD, iEventfd, &stEpollEvent) < 0)
	{
		HY_ERROR("Init epoll event error, %s\n",  strerror(errno));
		error_num = EpollCtlErr;
		return EpollCtlErr;
	}
	return NoErr;
}

/*epoll监听读事件*/
int 
epoll_read_event(epoll_fd_t *_this, int iEventfd)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iEventfd <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	base_epoll_event_t stEpollEvent;
	base_memset(&stEpollEvent, 0x0, sizeof(base_epoll_event_t));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(base_epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		HY_ERROR("Read epoll event error, %s\n",  strerror(errno));
		error_num = EpollCtlErr;
		return EpollCtlErr;
	}
	return NoErr;
}
/*epoll监听写事件*/
int 
epoll_write_event(epoll_fd_t *_this, int iEventfd)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iEventfd <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	base_epoll_event_t stEpollEvent;
	base_memset(&stEpollEvent, 0x0, sizeof(base_epoll_event_t));
	stEpollEvent.events = EPOLLOUT | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(base_epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		HY_ERROR("Write epoll event error, %s\n",  strerror(errno));
		error_num = EpollCtlErr;
		return EpollCtlErr;
	}
	return NoErr;
}
/*epoll监听读写事件*/
int 
epoll_read_write_event(epoll_fd_t *_this, int iEventfd)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iEventfd <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	base_epoll_event_t stEpollEvent;
	base_memset(&stEpollEvent, 0x0, sizeof(base_epoll_event_t));
	stEpollEvent.events = EPOLLIN | EPOLLOUT | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(base_epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		HY_ERROR("Read and write epoll event error, %s\n",  strerror(errno));
		error_num = EpollCtlErr;
		return EpollCtlErr;
	}
	return NoErr;
}
/*删除epoll监听事件*/
int 
epoll_del_event(epoll_fd_t *_this, int iEventfd)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iEventfd <= 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	base_epoll_event_t stEpollEvent;
	base_memset(&stEpollEvent, 0x0, sizeof(base_epoll_event_t));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(base_epoll_ctl(_this->iEpollFd, EPOLL_CTL_DEL, iEventfd, &stEpollEvent) < 0)
	{
		HY_ERROR("Del epoll event error, %s\n",  strerror(errno));
		error_num = EpollCtlErr;
		return EpollCtlErr;
	}
	return NoErr;
}

int
epoll_event_happen(epoll_fd_t *_this, epoll_event_t *paEpollEvents, int *piCount, int iTimeOutMs)
{
	if(NULL == _this ||
		NULL == paEpollEvents ||
		NULL == piCount)
	{
		error_num = ParaErr;
        return ParaErr;
	}

	if(iTimeOutMs < 0)
	{
		error_num = ParaErr;
        return ParaErr;
	}
		
	int i = 0;
	int iEventsCount = 0;
	int iEventsCountMax = *piCount;
	iEventsCount = 
		base_epoll_wait(
			_this->iEpollFd, 
			_this->pastWaitevents,
			_this->iEpollEventMaxNum,
			iTimeOutMs
		);
	for(i = 0; i < iEventsCount || i < iEventsCountMax; ++i)
	{
		paEpollEvents[i].iFd = _this->pastWaitevents[i].data.fd;
		paEpollEvents[i].uiEvent = _this->pastWaitevents[i].events;
	}

	*piCount = iEventsCount > iEventsCountMax ? iEventsCountMax : iEventsCount;

	return *piCount;
}

/*构造函数*/
epoll_fd_t *new_epoll_fd(int iEpollEventMaxNum)
{	
	if(iEpollEventMaxNum <= 0)
	{
		error_num = ParaErr;
        return NULL;
	}
	
	epoll_fd_t* pstEpollFd = 
		(epoll_fd_t *)base_calloc(1, sizeof(epoll_fd_t));
	if(NULL == pstEpollFd)
	{
		return NULL;
	}

	pstEpollFd->iEpollEventMaxNum = iEpollEventMaxNum;
	pstEpollFd->pastWaitevents = 
		(base_epoll_event_t *)base_calloc(pstEpollFd->iEpollEventMaxNum, sizeof(base_epoll_event_t));
	if(NULL == pstEpollFd->pastWaitevents)
	{
		return NULL;
	}
	
	if((pstEpollFd->iEpollFd = base_epoll_create(iEpollEventMaxNum)) < 0)
	{  
		error_num = EpollInitErr;
		return NULL;  
    }
	
	pstEpollFd->init_event = epoll_init_event;
	pstEpollFd->read_event = epoll_read_event;
	pstEpollFd->write_event = epoll_write_event;
	pstEpollFd->read_write_event = epoll_read_write_event;
	pstEpollFd->del_event = epoll_del_event;
	pstEpollFd->happen_event = epoll_event_happen;
	
	return pstEpollFd;
}

/*析构函数*/
int destroy_epoll_fd(epoll_fd_t *_this)
{
	if(NULL == _this)
	{
		error_num = ParaErr;
        return ParaErr;
	}
	
	if(_this->iEpollFd)
	{
		base_close(_this->iEpollFd);
	}

	if(_this->pastWaitevents)
	{
		base_free(_this->pastWaitevents);
	}

	base_free(_this);
	_this = NULL;

	return NoErr;
}


