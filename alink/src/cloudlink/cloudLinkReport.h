#ifndef _CLOUDLINKREPORT_H_
#define _CLOUDLINKREPORT_H_

#include "hylink.h"
#include "cloudLinkListFunc.h"

extern const SAttrInfo g_GatewaAttr;
extern const SAttrInfo g_SCloudModel;
extern const SAttrInfo g_SCloudAttr[];

int cloudUpdate(HylinkDev *hylinkDev, const unsigned int devType);
int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int attrType);
#endif