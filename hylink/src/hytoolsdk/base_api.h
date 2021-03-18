/***********************************************************
*文件名     : base_api.h
*版   本   : v1.0.0.0
*日   期   : 2019.09.30
*说   明   : 系统基础接口
*修改记录: 将系统接口简单封装，方便后续多平台移植
************************************************************/

#ifndef BASE_API_H
#define BASE_API_H

#include <stdio.h>
#include <dirent.h>
#include <sys/epoll.h>
#include "param_check.h"

//#define MEM_DEBUG
/*base_malloc.c*/
extern void* _base_calloc(unsigned int num, unsigned int size, const char *pfile, const char *pfun, int line);
extern void* _base_malloc(unsigned int size, const char *pfile, const char *pfun, int line);
extern void* _base_realloc(void *mem, unsigned int newsize, const char *pfile, const char *pfun, int line);
extern void _base_free(void *mem, const char *pfile, const char *pfun, int line);


#define base_calloc(num, size)\
		(_base_calloc((num), (size), __FILE__, __FUNCTION__, __LINE__))
#define base_malloc(size)\
		(_base_malloc((size), __FILE__, __FUNCTION__, __LINE__))
#define base_realloc(mem, size)\
		(_base_realloc((mem), (size), __FILE__, __FUNCTION__, __LINE__))
#define base_free(mem)\
		(_base_free((mem), __FILE__, __FUNCTION__, __LINE__))

/*获取堆空间使用量*/
extern int base_heap_usage(void);
extern int base_heap_usage_printf(void);

/*base_mem.c*/
extern void * base_memset (void * ptr, int value, unsigned int num);
extern int base_memcmp (void *str1, void *str2, unsigned int n);
extern void * base_memcpy (void *dest, void *src, unsigned int n);

/*base_string.c*/
extern char *base_strcpy(char *destin, char *source);
extern char *base_strncpy(char *dest, char *src, int n);
extern char *base_strcat(char *destin, char *source);
extern char *base_strchr(char *str, char c);
extern char *base_strrchr(char *str, int c);
extern int base_strcmp(char *str1, char *str2);
extern int base_strncmp(char *str1, char *str2, int n);
extern int base_strcasecmp(char *str1, char *str2);
extern int base_strlen(char *str);
extern char *base_strtok_r(char *str, char *delim, char **ptrptr);
#define base_snprintf(str, size, format, ...) \
		(snprintf(str, size, format, __VA_ARGS__))
extern int base_isprint(char c);
extern int base_atoi(char *str);
extern long int base_atol(char *str);
extern double base_atof(char *str);
extern unsigned long int base_strtoul(char *nptr, char **endptr, int base);
/*************************************************************
*函数:	base_version_comp
*参数:	pcNewVer: 新的版本号
*		pcOldVer: 旧的版本号
*返回:	1表示新的版本号高于旧的版本号, 0表示新旧版本号相同,
*		-1表示新的版本号低于旧的版本号,-2表示新旧版本号格式错误
*描述:	版本号比较接口，该接口传入的新旧版本号格式要求相同
*************************************************************/
extern int
base_version_comp(char *pcNewVer, char *pcOldVer);

/*base_time.c*/
typedef struct base_timeval_s
{
   unsigned int uiSec;
   unsigned int uiMsec;
   unsigned int uiUsec;
}base_timeval_t;

extern int base_time_get(base_timeval_t *pstTimeVal);
extern int base_time_str_get(char *pacTimeStr);
extern void base_delay_s(unsigned int s);
extern void base_delay_ms(unsigned int ms);
extern void base_delay_us(unsigned int us);



/*互斥锁*/
extern void *base_mutex_lock_create(void);
extern int base_mutex_lock_destroy(void *mutex);
extern int base_mutex_lock(void *mutex);
extern int base_mutex_trylock(void *mutex);
extern int base_mutex_unlock( void *mutex);

/*线程接口*/
#include <pthread.h>

typedef  pthread_t thread_t;

typedef void* (*ThreadTaskFunc)(void*);
typedef void (*ThreadCleanUpFunc)(void*);

/*线程池相关接口*/

/*初始化线程池*/
extern int thread_pool_init(void **pool, int num);  
/*添加任务*/
extern int thread_pool_add_worker(void *pool, ThreadTaskFunc process, void *arg, int arg_len); 
/*终止任务*/
extern int thread_pool_del_worker(void *pool, int worker_id);

/*销毁线程池*/
extern int thread_pool_destroy(void *pool); 

/*创建分离线程*/
extern int
base_thread_create(thread_t *pid,
              ThreadTaskFunc pthread_handle,
              void *arg);

/*开启线程取消*/
#define base_thread_set_cancel()\
	do{\
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);\
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);\
	}while(0)

/*线程取消点*/
#define base_thread_cancel_point()\
	do{\
		pthread_testcancel();\
	}while(0)

/*线程取消*/
#define base_thread_cancel(pid)\
	do{\
		pthread_cancel(pid);\
	}while(0)

/*清理函数入栈*/
#define base_thread_cleanup_push(routine, arg)\
	pthread_cleanup_push(routine, arg)

/*清理函数出栈*/
#define base_thread_cleanup_pop(execute)\
	pthread_cleanup_pop(execute)


/*信号量*/
extern void * base_sem_create (int pshared, unsigned int value);
extern int base_sem_destroy(void * sem);
extern int base_sem_wait(void * sem);
extern int base_sem_trywait(void * sem);
extern int base_sem_timedwait(void *sem, base_timeval_t *pstTimeout);
extern int base_sem_post(void * sem);

/*文件、IO操作接口*/
typedef FILE BASE_FILE;
typedef struct stat base_stat_t;

extern BASE_FILE *base_fopen(char *pathname, char *mode);
extern int base_fclose(BASE_FILE *fp);
extern BASE_FILE *base_popen(char *cmd, char *type);
extern int base_pclose(BASE_FILE *fp);
extern int base_fgetc(BASE_FILE *fp);
extern int base_fputc(char c, BASE_FILE *fp);
extern char *base_fgets(char *buf, int bufsize, BASE_FILE *fp);
extern int base_fputs(char *str, BASE_FILE *fp);
#define base_fprintf(stream, format, ...) \
		(fprintf(stream, format, ##__VA_ARGS__))
#define base_fscanf(stream, format, ...) \
		(fscanf(stream, format, ##__VA_ARGS__))
extern int base_fread(void *buffer, int size, int count, BASE_FILE *fp);
extern int base_fwrite(void* buffer, int size, int count, BASE_FILE* fp);
extern long base_ftell(BASE_FILE *fp);
extern int base_fseek(BASE_FILE *fp, long offset, int origin);

#define base_open(pathname, ...) \
		(open(pathname, ##__VA_ARGS__))
extern int base_close(int fd);
extern int base_read(int fd, void *buf, int count);
extern int base_write(int fd, void *buf, int count);
extern int base_lseek(int fd, int offset,int whence);
#define base_ioctl(fd, request, ...) \
		(ioctl(fd, request, ##__VA_ARGS__))
extern int base_stat(char *pathname, base_stat_t *buf);
extern int base_fstat(int fd, base_stat_t *buf);
extern int base_lstat(char *pathname, base_stat_t *buf);
extern int base_access(char * pathname, int mode);

//typedef DIR BASE_DIR;
typedef struct __dirstream BASE_DIR;
typedef struct dirent base_dirent_t;

extern int base_mkdir(char *pathname, int mode);
extern int base_rmdir(char *pathname);
extern BASE_DIR * base_opendir(char * pathname);
extern int base_closedir(BASE_DIR *dir);
extern base_dirent_t * base_readdir(BASE_DIR * dir);

extern int base_rename(char *oldname, char *newname);
extern int base_remove(char *pathname);


/*epoll相关接口*/
typedef struct epoll_event base_epoll_event_t;

extern int base_epoll_create(int size);
extern int base_epoll_ctl(int epfd, int op, int fd, base_epoll_event_t * event); 
extern int base_epoll_wait(int epfd, base_epoll_event_t * events, int maxevents, int timeout);


/*base_system.c*/
/*************************************************************
*函数:	base_system
*参数:	pcCmd: 要执行的命令
*		pcResult: 命令返回结果,如果不需要关心返回结果,
*				该值可赋值为NULL,此时iBuffSize的值失去意义.
*		iBuffSize: 用于存储命令返回值的缓存大小
*描述:	system函数
*************************************************************/
extern int base_system(char *pcCmd, char *pcResult, int iBuffSize);

#endif /*BASE_API_H*/

