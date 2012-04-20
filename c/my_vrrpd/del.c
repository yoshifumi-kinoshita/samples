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

 s_in->sin_family = AF_INET;
 s_in->sin_addr.s_addr = inet_addr("0.0.0.0");

 /* IPアドレスを変更するインターフェースを指定 */
 strncpy(ifr.ifr_name, "eth0:0", IFNAMSIZ-1);

 if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
   perror("ioctl");
 }

 close(fd);

 return 0;
}
