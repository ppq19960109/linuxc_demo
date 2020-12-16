#ifndef _NATIVESERVER_H_
#define _NATIVESERVER_H_

int nativeServerEpollMain();
int nativeServerEpollClose();
int nativeHylinkWrite(void *recv, unsigned int dataLen);
int nativeZigbeeWrite(void *recv, unsigned int dataLen);
#endif