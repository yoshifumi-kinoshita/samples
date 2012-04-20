#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <netinet/ether.h>

void get_ifinfo(char *devname, struct ifreq *ifreq, int flavor) {
  int iofd;
  
  if ((iofd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("ioctl socket creation");
	exit(1);
  }
  
  memset(ifreq, '\0', sizeof(*ifreq));
  strcpy(ifreq->ifr_name, devname);
  
  if (ioctl(iofd, flavor, ifreq) < 0) {
	perror("ioctl");
	exit(1);
  }
  
  return;
}

void send_gratuitous_arp(const int ifIndex, const char *devname, const char *ipaddress, const char *src_mac_address, const char *dst_mac_address){
  int rawfd;
  struct ether_arp arpbody;
  struct sockaddr_ll ll_from, ll_to;
  struct ether_addr src_mac_addr;
  struct ether_addr dst_mac_addr;

  ether_aton_r(src_mac_address, &src_mac_addr);
  ether_aton_r(dst_mac_address, &dst_mac_addr);

  arpbody.arp_hrd = htons(ARPHRD_ETHER);
  arpbody.arp_pro = htons(ETH_P_IP);
  arpbody.arp_hln = 6;
  arpbody.arp_pln = 4;
  arpbody.arp_op  = htons(ARPOP_REQUEST);
  memcpy(&(arpbody.arp_sha[0]), src_mac_addr.ether_addr_octet, ETH_ALEN); //arpbody.arp_sha
  memcpy(&(arpbody.arp_tha[0]), dst_mac_addr.ether_addr_octet, ETH_ALEN); //arpbody.arp_tha
  *((int *)&(arpbody.arp_spa[0])) = inet_addr(ipaddress); //arpbody.arp_spa
  *((int *)&(arpbody.arp_tpa[0])) = inet_addr(ipaddress); //arpbody.arp_tpa

  
  ll_from.sll_family = AF_PACKET;
  ll_from.sll_protocol = htons(ETH_P_ARP);
  ll_from.sll_ifindex = ifIndex;
  ll_from.sll_hatype = 0; //ARPHRD_ETHER;
  ll_from.sll_pkttype = 0; //PACKET_BROADCAST;
  ll_from.sll_halen = ETH_ALEN;
  //ll_from.sll_addr = NULL;
  memcpy(ll_from.sll_addr, src_mac_addr.ether_addr_octet, sizeof(src_mac_addr.ether_addr_octet));
  
  ll_to.sll_family = AF_PACKET;
  ll_to.sll_protocol = htons(ETH_P_ARP);
  ll_to.sll_ifindex = ifIndex;
  ll_to.sll_hatype = 0; //ARPHRD_ETHER;
  ll_to.sll_pkttype = 0; //PACKET_BROADCAST;
  ll_to.sll_halen = ETH_ALEN;
  //ll_to.sll_addr = NULL;
  memcpy(ll_to.sll_addr, dst_mac_addr.ether_addr_octet, sizeof(dst_mac_addr.ether_addr_octet));
  
  if ((rawfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP))) < 0) {
	perror("raw socket creatin");
	exit(1);
  } 
  
  if (bind(rawfd, (struct sockaddr *)&ll_from, sizeof(ll_from)) < 0) {
	perror("bind");
	exit(1);
  }
  
  int size=0;
  if ( (size = sendto(rawfd, (void *)&arpbody, sizeof(struct ether_arp), 0, (struct sockaddr *)&ll_to, sizeof(ll_to))) <= 0) {
	perror("write");
	exit(1);
  }
}

int main(int argc, char **argv){
  int ifindex = 0;
  char *devname = "eth0";
  char *ipaddr = "10.0.7.1";
  char *src_macaddr = "00:12:34:56:78:9a";
  //char *src_macaddr = "00:1c:25:20:c7:63";
  char *dst_macaddr = "00:00:00:00:00:00";
  struct ifreq ifreq;

  memset(&ifreq, '\0', sizeof(ifreq));

  //ifindex
  get_ifinfo( devname, &ifreq, SIOCGIFINDEX);
  ifindex = ifreq.ifr_ifindex;

  send_gratuitous_arp(ifindex, devname, ipaddr, src_macaddr, dst_macaddr);
  return 0;
  
}
