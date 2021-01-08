#ifndef _CLOUDLINKREPORT_H_
#define _CLOUDLINKREPORT_H_

#include "hylink.h"
#include "cloudLinkListFunc.h"

int cloudReport(void *hydev, unsigned int hyAttr);
int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int hyAttr);
#endif