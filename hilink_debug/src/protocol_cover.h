#ifndef _PROTOCOL_COVER_H
#define _PROTOCOL_COVER_H

#ifdef __cplusplus
extern "C"
{
#endif

int read_from_bottom(const char *json);
int write_to_bottom(void *ptr);
#ifdef __cplusplus
}
#endif
#endif