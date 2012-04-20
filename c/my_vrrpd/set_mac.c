#include <stdio.h>

#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>

int
main()
{
 int fd;
 struct ifreq ifr;
 struct sockaddr_in *s_in;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 s_in = (struct sockaddr_in *)&ifr.ifr_addr;

 /* IPv4 */
 s_in->sin_family = AF_INET;
 /* 変更するIPアドレス */
 s_in->sin_addr.s_addr = inet_addr("10.0.7.3");

 /* IPアドレスを変更するインターフェースを指定 */
 strncpy(ifr.ifr_name, "eth0:0", IFNAMSIZ-1);

 /* IPアドレスを変更 */
 if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
   /* 失敗したら理由を表示 */
   perror("ioctl");
 }

 s_in = (struct sockaddr_in *)&ifr.ifr_netmask;
 s_in->sin_addr.s_addr = inet_addr("255.255.0.0");

 if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0) {
   /* 失敗したら理由を表示 */
   perror("ioctl");
 }

 ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
 /*
 ifr.ifr_hwaddr.sa_data[0] = 0xAA;
 ifr.ifr_hwaddr.sa_data[1] = 0xBB;
 ifr.ifr_hwaddr.sa_data[2] = 0xCC;
 ifr.ifr_hwaddr.sa_data[3] = 0xDD;
 ifr.ifr_hwaddr.sa_data[4] = 0xEE;
 ifr.ifr_hwaddr.sa_data[5] = 0xFF;
 */
 ifr.ifr_hwaddr.sa_data[0] = 0x00;
 ifr.ifr_hwaddr.sa_data[1] = 0x11;
 ifr.ifr_hwaddr.sa_data[2] = 0x11;
 ifr.ifr_hwaddr.sa_data[3] = 0x43;
 ifr.ifr_hwaddr.sa_data[4] = 0x35;
 ifr.ifr_hwaddr.sa_data[5] = 0x52;

 if (ioctl(fd, SIOCSIFHWADDR, &ifr) != 0) {
   perror("ioctl");
 }

 close(fd);

 return 0;
}
