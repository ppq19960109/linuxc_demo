#ifndef _CLOUDLINKREPORT_H_
#define _CLOUDLINKREPORT_H_

#include "cloudLinkListFunc.h"
char *generateCloudJson(const char *cloudKey, const char *hyValue, const unsigned char valueType);
int cloudReport(void *hydev, unsigned int hyAttr);

#endif