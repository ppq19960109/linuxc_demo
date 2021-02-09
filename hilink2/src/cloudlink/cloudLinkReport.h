#ifndef _CLOUDLINKREPORT_H_
#define _CLOUDLINKREPORT_H_

#include "cloudLinkListFunc.h"
char *getCloudJson(const char *cloudKey, const char *hyValue, const unsigned char valueType);
int cloudReport(void *hydev, unsigned int hyAttr);
int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int hyAttr);
#endif