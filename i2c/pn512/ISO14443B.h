#ifndef _ISO14443B_H_
#define _ISO14443B_H_

#include"pn512.h"

extern unsigned char ATQB[2];
extern unsigned char Attrib[2];
extern unsigned char UUID[10];

void PcdRequest_IDCard(void);

#endif

