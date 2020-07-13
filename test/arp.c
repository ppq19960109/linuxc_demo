#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
#define ARP_REQUEST 1
typedef struct aprhdr{
	u_int16_t htype;	//Hardware type
	u_int16_t ptype;	//protocol type
	u_char hlen;		//Hardware address length
	u_char plen;		//protocol address length
	u_int16_t oper;		//operation code
	u_char sha[6];		//sender hardware address
	u_char spa[4];		//sender ip address
	u_char tha[6];		//target hardware address
	u_char tpa[4];		//target ip address
}arphdr_t;
 
#define MAX 2048
 
int main(void){
	int i = 0;
	bpf_u_int32 net = 0, mast = 0;
	struct bpf_program filter;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *des = NULL;
	struct pcap_pkthdr hk;
	const unsigned char *packet = NULL;
	arphdr_t *arp = NULL;
 
	memset(errbuf, 0, PCAP_ERRBUF_SIZE);
	
	des = pcap_open_live("eth0", MAX, 0, 512, errbuf);
	pcap_lookupnet("eth0", &net, &mast, errbuf);
        pcap_compile(des, &filter, "arp", 1, 0);
	pcap_setfilter(des, &filter);
 
	while(1){
		packet  = pcap_next(des, &hk);
		arp = (struct arphdr*)(packet + 14);
		if(arp != NULL){
			printf("\nRecived packet size:\t%d\n", hk.len);
			printf("Hardware type:\t%s\n", (ntohs(arp->htype) == 1) ? "Ethernet" : "Unknown");
			printf("Protocol type:\t%s\n", (ntohs(arp->ptype) == 0x0800) ? "IPv4" : "Unknown");
			printf("Operation:\t%s\n", (ntohs(arp->oper) == ARP_REQUEST) ? "ARP Request" : "ARP Reply");
 
			if(ntohs(arp->htype) == 1 && ntohs(arp->ptype) == 0x0800){
				printf("Sender MAC:\t");
				for(; i < 6; i++){
					printf("%02X:", arp->sha[i]);
				}
				printf("\nSender IP:\t");
				for(i = 0; i < 4; i++){
					printf("%d.", arp->spa[i]);
				}
				printf("\nTarget MAC\t");
				for(i = 0; i < 6; i++){
					printf("%02X:", arp->tha[i]);
				}
				printf("\nTarget IP:\t");
				for(i = 0; i < 4; i++){
					printf("%d.", arp->tpa[i]);
				}
			
			}
		}	
	}
	return 0;
}