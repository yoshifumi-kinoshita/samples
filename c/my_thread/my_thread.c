#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>


static int num_of_thread = 100;

void * thread_func( void * arg ) {
  pthread_detach( pthread_self());

  while(1){
    //Do something here.
    sleep(1);
  }
}

int start_threads(){
  pthread_t pt[num_of_thread];
  int i;
  for(i=0; i<num_of_thread; i++){
	pthread_create( &(pt[i]), NULL, &thread_func, NULL );
  }
  return 1;
}

int main(int argc, char **argv){

  start_threads();
  
  while(1){
	sleep(1);
  }
  exit(EXIT_SUCCESS);
}

