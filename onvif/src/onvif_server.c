#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsaapi.h"

#include "onvif_server.h"

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor,
                                          struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *_SOAP_ENV__Code,
                                          struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node,
                                          char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
    printf("call:%s\n", __func__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap *soap, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType *tdn__HelloResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap *soap, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType *tdn__ByeResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap *soap, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType *tdn__ProbeResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}

const char *onvif_get_xaddrs(char *addr, int len)
{
    char ip[18] = {0};
    getNetworkIp(ETH, ip, sizeof(ip));
    snprintf(addr, len, "http://%s:%d/onvif/device_service", ip, ONVIF_PORT);

    return addr;
}
const char *get_uuid(char *uuid, int len)
{
    char mac[18] = {0};
    getNetworkMac(ETH, mac, sizeof(mac), "");
    // build uuid using MAC address.
    snprintf(uuid, len,
             "urn:uuid:2419d68a-2dd2-21b2-a205-%s",
             mac);
    return uuid;
}
////---------------------------------------------------
#if 1
#include "wsddapi.h"
void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
{
    printf("call:%s\n", __func__);
}
void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{
    printf("call:%s\n", __func__);
}
void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{
    printf("call:%s\n", __func__);
}
void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{
    printf("call:%s\n", __func__);
}
soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
{
    printf("call:%s\n", __func__);
    printf("MessageID:%s,ReplyTo:%s,Types:%s,Scopes:%s,MatchBy:%s\n", MessageID, ReplyTo, Types, Scopes, MatchBy);

    soap_wsdd_init_ProbeMatches(soap, matches);

    char str_tmp[64] = {0};
    onvif_get_xaddrs(str_tmp, sizeof(str_tmp));

    char uuid[128] = {0};
    get_uuid(uuid, sizeof(uuid));
    soap_wsdd_add_ProbeMatch(soap, matches,
                             soap_strdup(soap, uuid + 4), "tdn:NetworkVideoTransmitter",
                             "onvif://www.onvif.org/type/NetworkVideoTransmitter",
                             NULL,
                             soap_strdup(soap, str_tmp), 1);

    struct wsa__Relationship *pWsa__RelatesTo = (struct wsa__Relationship *)soap_malloc(soap, sizeof(struct wsa__Relationship));
    soap_default__wsa__RelatesTo(soap, pWsa__RelatesTo);
    pWsa__RelatesTo->__item = soap->header->wsa__MessageID;
    soap->header->wsa__RelatesTo = pWsa__RelatesTo;
    soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";

    // soap->header->wsa__MessageID = (char *)soap_malloc(soap, sizeof(char) * 128);
    // strcpy(soap->header->wsa__MessageID, uuid + 4);
    soap->header->wsa__MessageID = soap_strdup(soap, uuid + 4);

    return SOAP_WSDD_MANAGED; //SOAP_WSDD_ADHOC
}
soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
    printf("call:%s\n", __func__);
    return SOAP_WSDD_MANAGED;
}
#else
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *soap, struct wsdd__HelloType *wsdd__Hello)
{
    printf("call:%s\n", __func__);
    return 0;
}
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *soap, struct wsdd__ByeType *wsdd__Bye)
{
    printf("call:%s\n", __func__);
    return 0;
}
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *soap, struct wsdd__ProbeType *wsdd__Probe)
{
    printf("call:%s\n", __func__);

    // response ProbeMatches
    struct wsdd__ProbeMatchesType wsdd__ProbeMatches = {0};
    struct wsdd__ProbeMatchType *pProbeMatchType = NULL;
    struct wsa__Relationship *pWsa__RelatesTo = NULL;

    pProbeMatchType = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
    soap_default_wsdd__ProbeMatchType(soap, pProbeMatchType);

    char str_tmp[64] = {0};
    onvif_get_xaddrs(str_tmp, sizeof(str_tmp));

    pProbeMatchType->XAddrs = soap_strdup(soap, str_tmp);
    if (wsdd__Probe->Types && strlen(wsdd__Probe->Types))
        pProbeMatchType->Types = soap_strdup(soap, wsdd__Probe->Types);
    else
        pProbeMatchType->Types = soap_strdup(soap, "dn:NetworkVideoTransmitter tds:Device");

    pProbeMatchType->MetadataVersion = 1;

    // Build Scopes Message
    struct wsdd__ScopesType *pScopes = NULL;
    pScopes = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
    soap_default_wsdd__ScopesType(soap, pScopes);
    pScopes->MatchBy = NULL;
    pScopes->__item = soap_strdup(soap, "onvif://www.onvif.org/type/NetworkVideoTransmitter");
    pProbeMatchType->Scopes = pScopes;

    char g_uuid[64];
    sprintf(g_uuid, "%s", soap_wsa_rand_uuid(soap));
    // sprintf(g_uuid, "%s", "urn:uuid:2419d68a-2dd2-21b2-a205-1234567890");
    printf("g_uuid:%s\n", g_uuid);
    pProbeMatchType->wsa__EndpointReference.Address = soap_strdup(soap, g_uuid + 4);

    wsdd__ProbeMatches.__sizeProbeMatch = 1;
    wsdd__ProbeMatches.ProbeMatch = pProbeMatchType;

    // Build SOAP Header
    pWsa__RelatesTo = (struct wsa__Relationship *)soap_malloc(soap, sizeof(struct wsa__Relationship));
    soap_default__wsa__RelatesTo(soap, pWsa__RelatesTo);
    pWsa__RelatesTo->__item = soap->header->wsa__MessageID;
    soap->header->wsa__RelatesTo = pWsa__RelatesTo;
    soap->header->wsa__Action = soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches");
    soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"; // soap_strdup(soap, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");

    // soap->header->wsa__MessageID = soap_strdup(soap, g_uuid);

    if (SOAP_OK == soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &wsdd__ProbeMatches))
    {
        printf("send ProbeMatches success !\n");
        return SOAP_OK;
    }

    printf("[%d] soap error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));

    return soap->error;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *soap, struct wsdd__ResolveType *wsdd__Resolve)
{
    return 0;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *soap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
    return 0;
}
#endif