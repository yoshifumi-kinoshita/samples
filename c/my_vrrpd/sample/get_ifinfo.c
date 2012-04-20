#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int main(int argc, char **argv) {
  struct ifreq ifreq;
  char *devname = "eth0";
  struct sockaddr_in saddr;
  
  //IP address
  get_ifinfo( devname, &ifreq, SIOCGIFADDR);
  memcpy(&saddr, &(ifreq.ifr_addr), sizeof(saddr));
  printf("%s\n", inet_ntoa(saddr.sin_addr));

  //MAC address
  get_ifinfo( devname, &ifreq, SIOCGIFHWADDR);
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n", (unsigned char)ifreq.ifr_hwaddr.sa_data[0], (unsigned char)ifreq.ifr_hwaddr.sa_data[1], (unsigned char)ifreq.ifr_hwaddr.sa_data[2], (unsigned char)ifreq.ifr_hwaddr.sa_data[3], (unsigned char)ifreq.ifr_hwaddr.sa_data[4], (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

  //ifIndex
  get_ifinfo( devname, &ifreq, SIOCGIFINDEX);
  printf("%d\n", ifreq.ifr_ifindex);

  return 0;
}
