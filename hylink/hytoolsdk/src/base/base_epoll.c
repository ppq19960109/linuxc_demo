#include "error_msg.h"
#include "base_api.h"

extern int base_epoll_create(int size)
{
	return epoll_create(size);
}

extern int base_epoll_ctl(int epfd, int op, int fd, base_epoll_event_t* event)
{
	return epoll_ctl(epfd, op, fd, event);
}

extern int base_epoll_wait(int epfd, base_epoll_event_t *events, int maxevents, int timeout)
{
	return epoll_wait(epfd, events, maxevents, timeout);
}



