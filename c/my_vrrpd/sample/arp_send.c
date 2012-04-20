#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include "arp_send.h"

int
main(int argc, char **argv)
{
    int rawfd;
    ssize_t size;
    struct sockaddr_ll ll_from, ll_to;
    struct arppkt      arppkt;
    unsigned int argsfound = 0;
    struct sockaddr_in saddr;
    char thostname[MAXHOSTNAMELEN];
    char ifname[IFNAMEMAX];

    if ((argsfound = parse_cmdline(argc, argv, thostname, ifname)) < ARGSMIN) {
	fprintf(stderr, 
		"Usage: %s -t hostaddr -i ifname\n",
		argv[0]);
	exit(0);
    }

    bzero(&saddr, sizeof(saddr));

    arp_init(&arppkt, &ll_from, &ll_to, &saddr, thostname, ifname);
    
    if ((rawfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP))) < 0) {
	perror("raw socket creatin");
	exit(1);
    } 

    if (bind(rawfd, (struct sockaddr *)&ll_from, sizeof(ll_from)) < 0) {
	perror("bind");
	exit(1);
    }
    
    if (size = sendto(rawfd, (void *)&arppkt, sizeof(struct arppkt), 0,
		      (struct sockaddr *)&ll_to, 
		      sizeof(ll_to)) < 0) {
	perror("write");
	exit(1);
    }

    return 0;
}

void
arp_init(struct arppkt *arppkt,  
	 struct sockaddr_ll *ll_from, 
	 struct sockaddr_ll *ll_to,
	 struct sockaddr_in *saddr,
	 char *thostname,
	 char *ifname)
{
    int iofd;
    struct ifreq       ifreq;
    u_char *p;
    
    bzero(arppkt, sizeof(*arppkt));
    
    arppkt->arpbody.arp_hrd = htons(ARPHRD_ETHER);
    arppkt->arpbody.arp_pro = htons(ETH_P_IP);
    arppkt->arpbody.arp_hln = 6;
    arppkt->arpbody.arp_pln = 4;
    arppkt->arpbody.arp_op  = htons(ARPOP_REQUEST); 
    
    ll_from->sll_family = AF_PACKET;
    ll_from->sll_protocol = htons(ETH_P_ARP);
    ll_from->sll_hatype = ARPHRD_ETHER;
    ll_from->sll_pkttype = PACKET_HOST;
    ll_from->sll_halen = ETH_ALEN;
    
    ll_to->sll_family = AF_PACKET;
    ll_to->sll_protocol = htons(ETH_P_ARP);
    ll_to->sll_hatype = ARPHRD_ETHER;
    ll_to->sll_pkttype = PACKET_BROADCAST;
    ll_to->sll_halen = ETH_ALEN;


    if ((iofd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("ioctl socket creation");
	exit(1);
    }

    get_ifinfo(iofd, ifname, &ifreq, SIOCGIFHWADDR);

    memcpy(&(ll_from->sll_addr[0]), ifreq.ifr_hwaddr.sa_data, ETH_ALEN);
    memcpy(&(arppkt->arpbody.arp_sha[0]), ifreq.ifr_hwaddr.sa_data, ETH_ALEN);

    memset(&(ll_to->sll_addr[0]), 0xff, ETH_ALEN);
    bzero(&(arppkt->arpbody.arp_tha[0]), ETH_ALEN);
   
    p = &(ll_from->sll_addr[0]);
    printf("MAC = [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
	   *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5));

    get_ifinfo(iofd, ifname, &ifreq, SIOCGIFADDR);

    bcopy(&(ifreq.ifr_addr), saddr, sizeof(*saddr));

    *((int *)&(arppkt->arpbody.arp_spa[0])) = saddr->sin_addr.s_addr; 
    *((int *)&(arppkt->arpbody.arp_tpa[0])) = translate_hostname(thostname); 
    
    p = &(arppkt->arpbody.arp_spa[0]);
    printf("spa = [%d.%d.%d.%d]\n", *p, *(p+1), *(p+2), *(p+3));

    p = &(arppkt->arpbody.arp_tpa[0]);
    printf("tpa = [%d.%d.%d.%d]\n", *p, *(p+1), *(p+2), *(p+3));

    get_ifinfo(iofd, ifname, &ifreq, SIOCGIFINDEX);

    ll_to->sll_ifindex = ll_from->sll_ifindex = ifreq.ifr_ifindex;

}

unsigned int
parse_cmdline(int argc, char *argv[], char *thostname, char *ifname)
{
    int c;
    unsigned int argsfound = 0;
    extern char *optarg;
    extern int optind;
    
    while ((c = getopt(argc, argv, "t:i:")) != -1) {
	switch ((char)c) {
	case 't':
	    argsfound |= THOSTFOUND;
	    strncpy(thostname, optarg, MAXHOSTNAMELEN);
	    fprintf(stderr, "target host %s\n", thostname);
	    break;
	case 'i':
	    argsfound |= IFNAMEFOUND;
	    strncpy(ifname, optarg, IFNAMEMAX);
	    fprintf(stderr, "interface %s\n", ifname);
	    break;
	default:
	    break;
	}
    }
    
    return argsfound;
}

unsigned long
translate_hostname(char *hostname)
{
        unsigned long addr;
        struct hostent *serv_host;

        if (isdigit(hostname[0])) {
                addr = inet_addr(hostname);
        } else {
                serv_host = gethostbyname(hostname);
                bcopy(serv_host->h_addr, (char *)&addr, sizeof(addr));
        }

        return addr;
}

void
get_ifinfo(int iofd, char *devname, struct ifreq *ifreq, int flavor)
{
    bzero(ifreq, sizeof(*ifreq));
    strcpy(ifreq->ifr_name, devname);

    if (ioctl(iofd, flavor, ifreq) < 0) {
	perror("ioctl");
	exit(1);
    }

    return;
}
