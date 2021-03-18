#include <string.h>

#include "error_msg.h"
#include "base_api.h"


extern void * base_memset (void * ptr, int value, unsigned int num)
{
	return memset(ptr, value, num);
}

extern int base_memcmp (void *str1, void *str2, unsigned int n)
{
	return memcmp(str1, str2, n);
}

extern void * base_memcpy (void *dest, void *src, unsigned int n)
{
	return memcpy(dest, src, n);
}


