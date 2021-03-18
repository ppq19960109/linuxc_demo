#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#include "error_msg.h"
#include "base_api.h"



extern BASE_FILE *base_fopen(char *pathname, char *mode)
{
	return fopen(pathname, mode);
}

extern int base_fclose(BASE_FILE *fp)
{
	return fclose(fp);
}

extern BASE_FILE *base_popen(char *cmd, char *type)
{
	return popen(cmd, type);
}

extern int base_pclose(BASE_FILE *fp)
{
	return pclose(fp);
}


extern int base_fgetc(BASE_FILE *fp)
{
	return fgetc(fp);
}
extern int base_fputc(char c, BASE_FILE *fp)
{
	return fputc(c, fp);
}
extern char *base_fgets(char *buf, int bufsize, BASE_FILE *fp)
{
	return fgets(buf, bufsize, fp);
}
extern int base_fputs(char *str, BASE_FILE *fp)
{
	return fputs(str, fp);
}
extern int base_fread(void *buffer, int size, int count, BASE_FILE *fp)
{
	return fread(buffer, size, count, fp);
}
extern int base_fwrite(void* buffer, int size, int count, BASE_FILE* fp)
{
	return fwrite(buffer, size, count, fp);
}
extern long base_ftell(BASE_FILE *fp)
{
	return ftell(fp);
}
extern int base_fseek(BASE_FILE *fp, long offset, int origin)
{
	return fseek(fp, offset, origin);
}

extern int base_close(int fd)
{
	return close(fd);
}
extern int base_read(int fd, void *buf, int count)
{
	return read(fd, buf, count);
}
extern int base_write(int fd, void *buf, int count)
{
	return write(fd, buf, count);
}
extern int base_lseek(int fd, int offset, int whence)
{
	return lseek(fd, offset, whence);
}
extern int base_stat(char *pathname, base_stat_t *buf)
{
	return stat(pathname, buf);
}
extern int base_fstat(int fd, base_stat_t *buf)
{
	return fstat(fd, buf);
}
extern int base_lstat(char *pathname, base_stat_t *buf)
{
	return lstat(pathname, buf);
}
extern int base_access(char *pathname, int mode)
{
	return access(pathname, mode);
}

extern int base_mkdir(char *pathname, int mode)
{
	return mkdir(pathname, mode);
}
extern int base_rmdir(char *pathname)
{
	return rmdir(pathname);
}
extern BASE_DIR *base_opendir(char * pathname)
{
	return opendir(pathname);
}
extern int base_closedir(BASE_DIR *dir)
{
	return closedir(dir);
}
extern base_dirent_t * base_readdir(BASE_DIR * dir)
{
	return (base_dirent_t *)readdir(dir);
}

extern int base_rename(char *oldname, char *newname)
{
	return rename(oldname, newname);
}
extern int base_remove(char *pathname)
{
	return remove(pathname);
}

