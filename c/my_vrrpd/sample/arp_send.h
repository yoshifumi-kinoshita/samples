#ifndef __ARP_SEND_H__
#define __ARP_SEND_H__

#define THOSTFOUND 0x01
#define IFNAMEFOUND 0x02

#define ARGSMIN   IFNAMEFOUND

#define IFNAMEMAX 16

struct arppkt {
    struct ether_arp arpbody;
};

void
arp_init(struct arppkt *arppkt,  
	 struct sockaddr_ll *ll_from, 
	 struct sockaddr_ll *ll_to,
	 struct sockaddr_in *saddr,
	 char *thostname,
	 char *ifname);

unsigned int
parse_cmdline(int argc, char *argv[], char *thostname, char *ifname);


unsigned long
translate_hostname(char *hostname);

void
get_ifinfo(int iofd, char *devname, struct ifreq *ifreq, int flavor);


#endif
