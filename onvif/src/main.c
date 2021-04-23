#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.nsmap"

#include "wsddapi.h"
#include "onvif_server.h"

#define ONVIF_LISTEN_PORT 3702

void *probe_thread(void *arg)
{
  struct soap soap;
  struct ip_mreq mcast;

  soap_init1(&soap, SOAP_IO_UDP | SOAP_XML_IGNORENS); //| SOAP_XML_IGNORENS
  soap_set_namespaces(&soap, namespaces);

  printf("probe_thread soap.version = %d\n", soap.version);

  if (!soap_valid_socket(soap_bind(&soap, NULL, ONVIF_LISTEN_PORT, 10)))
  {
    soap_print_fault(&soap, stderr);
    exit(1);
  }

  mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
  mcast.imr_interface.s_addr = htonl(INADDR_ANY);

  if (setsockopt(soap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast)) < 0)
  {
    printf("setsockopt error! error code = %d,%s\n", errno, strerror(errno));
    return 0;
  }
  printf("call:%s\n", __func__);
  while (1)
  {
#if 1
    if (soap_wsdd_listen(&soap, 5) != SOAP_OK)
    {
      soap_print_fault(&soap, stderr);
      continue;
    }
#else
    if (soap_serve(&soap))
    {
      soap_print_fault(&soap, stderr);
    }
    soap_destroy(&soap);
    soap_end(&soap);
#endif

    //客户端的IP地址
    // printf("probe_thread client connection from IP = %u.%u.%u.%u socket = %d \r\n", ((soap.ip) >> 24) & 0xFF, ((soap.ip) >> 16) & 0xFF, ((soap.ip) >> 8) & 0xFF, (soap.ip) & 0xFF, (soap.socket));
  }
  soap_destroy(&soap);
  soap_end(&soap);
  //分离运行时的环境
  soap_done(&soap);
}

static struct soap ServerSoap;
void main_close(int sig)
{
  soap_destroy(&ServerSoap);
  soap_end(&ServerSoap);
  //分离运行时的环境
  soap_done(&ServerSoap);
  exit(0);
}

int main(void)
{
  signal(SIGINT, main_close);

  pthread_t tid;
  pthread_create(&tid, NULL, probe_thread, NULL);
  pthread_detach(tid);

  printf("call:%s\n", __func__);

  soap_init(&ServerSoap);
  soap_set_namespaces(&ServerSoap, namespaces);
  printf("ServerSoap.version = %d\n", ServerSoap.version);

  if (!soap_valid_socket(soap_bind(&ServerSoap, NULL, ONVIF_PORT, 10)))
  {
    soap_print_fault(&ServerSoap, stderr);
    exit(1);
  }

  // int sock_opt = 1;
  // if ((setsockopt(ServerSoap.master, SOL_SOCKET, SO_REUSEADDR, (void *)&sock_opt, sizeof(sock_opt))) < 0)
  // {
  //   perror("setsockopt:");
  //   soap_print_fault(&ServerSoap, stderr);
  //   exit(1);
  // }

  // if (soap_serve(&ServerSoap))
  // {
  //   soap_print_fault(&ServerSoap, stderr);
  // }
  // soap_destroy(&ServerSoap);
  // soap_end(&ServerSoap);
  int sock;
  while ((sock = soap_accept(&ServerSoap)))
  {
    if (soap_valid_socket(sock))
    {
      if (soap_serve(&ServerSoap) != SOAP_OK)
      {
        soap_print_fault(&ServerSoap, stderr);
        printf("soap->error:%d\n", ServerSoap.error);
      }
      soap_destroy(&ServerSoap);
      soap_end(&ServerSoap);
    }
    else
      soap_print_fault(&ServerSoap, stderr);
  }
  soap_destroy(&ServerSoap);
  soap_end(&ServerSoap);
  //分离运行时的环境
  soap_done(&ServerSoap);

  return 0;
}
