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

#define PROGRAM_NAME "my_vrrpd"

#define CONF_DIR "/etc/my_vrrpd/conf.d"
#define VRRPD_PORT "40000"
#define PID_FILE "/var/run/my_vrrpd.pid"
#define MAX_PATH 512
#define MAX_VRRP_INSTANCE 256

#define VRRPD_INACTIVE 0
#define VRRPD_ACTIVE 1

typedef struct {
  int status;
  int vrid;
  char *device;
  char *hwaddr;
  char *ipaddr;
  char *netmask;
  int priority;
  int timeout;
} vrrpd_conf;

void vrrpd_shutdown(int);
void write_pid();
void daemonize();
void parse_args(int argc, char **argv);
void usage();
int load_configs();
int _load_config(char *path);
int init_socket();
int start_threads();
int start_server();
void signal_handler(int sig);

int startsWith(const char* s, const char* t);
int endsWith(const char* s, const char* t);
int trim(char** s);
int dump_vrrpd_conf(vrrpd_conf *);
int free_vrrpd_conf(vrrpd_conf *);
void * adv_thread_func( void * arg );
int send_advertisement(vrrpd_conf *);
int activate(vrrpd_conf *);
int parse_vrrp_msg( char*, vrrpd_conf* );
void parse_line(char *line, vrrpd_conf* conf);
int process_vrrp_msg( vrrpd_conf* );
vrrpd_conf* search_vrrpd_conf(int vrid);
int if_down(const char* dev);
int if_up(const char* device, const char* hwaddr, const char* ipaddr, const char* netmask );
void send_gratuitous_arp(const int ifIndex, const char *devname, const char *ipaddress, const char *src_mac_address, const char *dst_mac_address);
void get_ifinfo(char *devname, struct ifreq *ifreq, int flavor);

static char *port = NULL;
static char *conf_dir = NULL;
static vrrpd_conf *confs = NULL;
static int num_of_vrrp = 0;
static int vrrpd_timeouts[ MAX_VRRP_INSTANCE ];
static int sockfd = -1;
static pid_t pid = 0;

int main(int argc, char **argv){

  openlog(PROGRAM_NAME, LOG_PID|LOG_PERROR, LOG_LOCAL0);
  
  daemonize();
  
  write_pid();
  
  signal(SIGTERM, vrrpd_shutdown);
  
  //parse args
  parse_args(argc, argv);
  
  //load config and spawn adv threads
  load_configs();

  //initialize socket;
  init_socket();

  while(1){
	pid = fork();
	if(pid==0){
	  signal(SIGTERM, signal_handler);
	  goto CHILDREN;
	}
	else{
	  int status;
	  wait( &status );
	}
	usleep(100);
  }

 CHILDREN:
  //run ad threads
  start_threads();
  
  //start listening
  start_server();
  
  while(1){
	sleep(1);
  }
  exit(EXIT_SUCCESS);
}

void vrrpd_shutdown(int sig){
  //kill child
  kill(pid, SIGTERM);
  
  int i;
  for(i=0; i<num_of_vrrp; i++){
	if_down(confs[i].device);
  }

  //ToDo free variables

  closelog();
  exit(0);
}

void signal_handler(int sig){
  exit(0);
}

void write_pid(){
  FILE *fp;
  fp = fopen(PID_FILE, "w");
  fprintf(fp, "%d", getpid());
  fclose(fp);
}

void daemonize(){
  pid_t pid, sid;
  
  if( chdir("/") < 0 )	exit(EXIT_FAILURE);
  
  pid = fork();
  if( pid < 0 ) exit(EXIT_FAILURE);
  
  if( pid > 0 ) exit(EXIT_SUCCESS);
  
  sid = setsid();
  if( sid < 0 ) exit(EXIT_FAILURE);

  
  umask(0);
  
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  
  return;
}

void parse_args(int argc, char **argv){
  int c;
  while(1){
	static struct option long_options[] =
	  {
		{"port", no_argument, 0, 'p'},
		{"conf_dir", no_argument, 0, 'c'},
		{0,0,0,0}
	  };
	int option_index = 0;
	c = getopt_long( argc, argv, "p:c:h", long_options, &option_index);
	
	if( c==-1 ) break;
	
	switch(c){
	case 'p':
	  port = (char *)malloc( sizeof(char) * (strlen(optarg)+1) );
	  strncpy( port, optarg, strlen(optarg)+1 );
	  syslog(LOG_INFO, "port: %s", port);
	  break;
	case 'c':
	  conf_dir = (char *)malloc( sizeof(char) * (strlen(optarg)+1) );
	  strncpy( conf_dir, optarg, strlen(optarg)+1 );
	  syslog(LOG_INFO, "conf_dir: %s", conf_dir);
	  break;
	case 'h':
	  usage();
	  exit(EXIT_SUCCESS);
	default:
	  break;
	}
  }
  if(!conf_dir){
	conf_dir = (char *)malloc( sizeof(char) * (strlen(CONF_DIR)+1) );
	strncpy( conf_dir, CONF_DIR, strlen(CONF_DIR)+1 );
  }
  if(!port){
	port = (char *)malloc( sizeof(char) * (strlen(VRRPD_PORT)+1) );
	strncpy( port, VRRPD_PORT, strlen(VRRPD_PORT)+1 );
  }
}

void usage(){
  printf("Usage: my_vrrpd -p <PORT NUMBER> -c <CONF DIR>\n");
}

int load_configs(){
  confs = (vrrpd_conf *)malloc( sizeof(vrrpd_conf) * MAX_VRRP_INSTANCE );
  memset( confs, '\0', sizeof(vrrpd_conf)*MAX_VRRP_INSTANCE );

  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  if(( dp = opendir(conf_dir) ) == NULL ){
	perror("opendir");
	syslog(LOG_ERR, "can't open dir: %s", conf_dir);
	exit( EXIT_FAILURE );
  }
  
  while((entry = readdir(dp)) != NULL){
	stat(entry->d_name, &statbuf);
	char path_to_conf[MAX_PATH];
	if( endsWith("conf", entry->d_name) ){
	  sprintf( path_to_conf, "%s/%s", conf_dir, entry->d_name );
	  _load_config(path_to_conf);
	}
	
  }
  return 1;
}

int _load_config(char *path){
  FILE *fp;
  char buf[BUFSIZ];
  if((fp=fopen(path,"r"))==NULL){
	return -1;
  }

  while(fgets(buf,sizeof(buf),fp)!=NULL){
	if( startsWith("#", buf) ) continue;
	if( !index(buf, '=') ) continue;
	char *tp;
	char *saveptr;
	//find KEY
	tp = strtok_r( buf, "=", &saveptr );
	char **tpp = &tp;
	trim(tpp);

	char *key = tp;

	//find VALUE
	tp = strtok_r( NULL, "=", &saveptr );
	tpp = &tp;
	trim(tpp);

	char *val = tp;
	
	if( strncmp("VRID", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].vrid = atoi( val );
	}
	else if( strncmp("DEVICE", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].device = (char *)malloc( sizeof(char) * strlen(val)+1 );
	  strcpy(confs[ num_of_vrrp ].device, val);
	}
	else if( strncmp("HWADDR", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].hwaddr = (char *)malloc( sizeof(char) * strlen(val)+1 );
	  strcpy(confs[ num_of_vrrp ].hwaddr, val);
	}
	else if( strncmp("IPADDR", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].ipaddr = (char *)malloc( sizeof(char) * strlen(val)+1 );
	  strcpy(confs[ num_of_vrrp ].ipaddr, val);
	}
	else if( strncmp("NETMASK", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].netmask = (char *)malloc( sizeof(char) * strlen(val)+1 );
	  strcpy(confs[ num_of_vrrp ].netmask, val);
	}
	else if( strncmp("PRIORITY", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].priority = atoi( val );
	}
	else if( strncmp("TIMEOUT", key, strlen(key))==0 ){
	  confs[ num_of_vrrp ].timeout = atoi( val );
	}
  }

  //ToDo: check all values are correct.
  if( !confs[ num_of_vrrp ].vrid
	  || !confs[ num_of_vrrp ].device
	  || !confs[ num_of_vrrp ].hwaddr
	  || !confs[ num_of_vrrp ].ipaddr
	  || !confs[ num_of_vrrp ].netmask
	  || !confs[ num_of_vrrp ].priority
	  || !confs[ num_of_vrrp ].timeout ){
	free_vrrpd_conf( &confs[num_of_vrrp] );
	return 0;
  }

  num_of_vrrp++;

  return 1;
}

int start_server(){
  struct sockaddr_in addr ;
  int addrlen;
  int size;

  while(1){
	char buf[BUFSIZ];
	size = recvfrom( sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, (socklen_t *)&addrlen );
	//ToDo: check confs and acivate/deactivate IPs.
	vrrpd_conf conf;
	parse_vrrp_msg( buf, &conf );
	process_vrrp_msg( &conf );
  }
  return 1;
}

int start_threads(){
  pthread_t pt[num_of_vrrp];
  int i;
  for(i=0; i<num_of_vrrp; i++){
	pthread_create( &(pt[i]), NULL, &adv_thread_func, &(confs[i]) );
  }
  return 1;
}


int startsWith(const char* s, const char* t){
  if(strlen(s) > strlen(t)) return 0;

  int i=0;
  while(i<strlen(s)){
	if( s[i] != t[i] ) return 0;
	i++;
  }
  return 1;
}

int endsWith(const char* s, const char* t){
  if(strlen(s) > strlen(t)) return 0;

  int i=strlen(s)-1;
  int end_pos = strlen(t)-1;
  while(i>=0){
	if( s[i] != t[end_pos] ) return 0;
	i--;
	end_pos--;
  }
  return 1;
  
}

int trim(char** s){
  if((*s)==NULL) return 0;
  int end_pos = strlen(*s)-1;
  while(' '==(*s)[end_pos] || '\t'==(*s)[end_pos] || '\r'==(*s)[end_pos] || '\n'==(*s)[end_pos] ){
	(*s)[end_pos] = '\0';
	end_pos--;
  }

  while(' '==*s[0] || '\t'==*s[0] || '\r'==*s[0] || '\n'==*s[0]){
	*s = *s+1;
  }

  return 1;
}

int dump_vrrpd_conf(vrrpd_conf *conf){
  printf("===\n");
  printf("VRID: %d\n", conf->vrid );
  printf("DEVICE: %s\n", conf->device );
  printf("HWADDR: %s\n", conf->hwaddr );
  printf("IPADDR: %s\n", conf->ipaddr );
  printf("NETMASK: %s\n", conf->netmask );
  printf("PRIORITY: %d\n", conf->priority );
  printf("TIMEOUT: %d\n", conf->timeout );
  return 0;
}

int free_vrrpd_conf(vrrpd_conf* conf){
  if(!conf->device) free(conf->device);
  if(!conf->hwaddr) free(conf->hwaddr);
  if(!conf->ipaddr) free(conf->ipaddr);
  if(!conf->netmask) free(conf->netmask);
  return 0;
}

void * adv_thread_func( void * arg ) {
  pthread_detach( pthread_self());

  vrrpd_conf* conf = (vrrpd_conf*)arg;

  while(1){
	sleep(1);
	if( conf->status==VRRPD_ACTIVE ){
	  send_advertisement(conf);
	  continue;
	}
	
	if( conf->timeout > vrrpd_timeouts[ conf->vrid ] ){
	  vrrpd_timeouts[ conf->vrid ]++;
	  continue;
	}
	else {
	  vrrpd_timeouts[ conf->vrid ] = 0;
	  send_advertisement(conf);
	  activate(conf);
	  continue;
	}
	dump_vrrpd_conf( (vrrpd_conf*)arg );
  }
}

int init_socket(){
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  int result=0;
  
  struct sockaddr_in src_address;
  src_address.sin_family = AF_INET;
  src_address.sin_addr.s_addr = inet_addr("10.0.7.2");
  src_address.sin_port = htons(atoi(port));
  int len = sizeof(src_address);
  
  result = bind(sockfd, (struct sockaddr *)&src_address, len);
  /*
  if(result != 0){
	switch(errno){
	case EBADF:
	  printf("EBADF");
	  break;
	case ENOTSOCK:
	  printf("ENOTSOCK");
	  break;
	case EINVAL:
	  printf("EINVAL");
	  break;
	case EADDRNOTAVAIL:
	  printf("EADDRNOTAVAIL");
	  break;
	case EADDRINUSE:
	  printf("EADDRINUSE");
	default:
	  break;
	}
  }
  */
  return 0;
}


int send_advertisement(vrrpd_conf *conf){
  int result=0;

  struct sockaddr_in dst_address;
  dst_address.sin_family = PF_INET;
  dst_address.sin_addr.s_addr = inet_addr("224.0.0.1");
  dst_address.sin_port = htons(atoi(port));
  int len = sizeof(dst_address);

  char buf[BUFSIZ];
  sprintf(buf, "VRID: %d\r\nPRIORITY: %d", conf->vrid, conf->priority );
  result = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&dst_address, len);

  return 1;

}

int activate(vrrpd_conf *conf){
  conf->status=VRRPD_ACTIVE;
  if_up(conf->device, conf->hwaddr, conf->ipaddr, conf->netmask );
  
  struct ifreq ifreq;
  memset(&ifreq, '\0', sizeof(ifreq));
  get_ifinfo( conf->device, &ifreq, SIOCGIFINDEX); //ifindex
  send_gratuitous_arp(ifreq.ifr_ifindex, conf->device, conf->ipaddr, conf->hwaddr, "00:00:00:00:00:00");
  
  return 1;
}

int parse_vrrp_msg( char* buf, vrrpd_conf* conf ){
  char *tp;
  char *saveptr;
  tp = strtok_r( buf, "\r\n", &saveptr );
  
  parse_line(tp, conf);
  
  while( (tp = strtok_r(NULL, "\r\n", &saveptr)) != NULL){
	parse_line(tp, conf);
  }
  
  return 1;
}

void parse_line(char *line, vrrpd_conf* conf){
  char *tp;
  char *val;
  char *saveptr;
  tp = strtok_r( line, ":", &saveptr );
  if(0 == strcmp("VRID", tp)){
	val = strtok_r( NULL, ":", &saveptr );
	conf->vrid = atoi( val );
  }
  else if(0 == strcmp("PRIORITY", tp)){
	val = strtok_r( NULL, ":", &saveptr );
	conf->priority = atoi( val );
  }
}

int process_vrrp_msg( vrrpd_conf* conf ){
  vrrpd_conf* my_conf = search_vrrpd_conf(conf->vrid);
  if( my_conf == NULL ) return 0;
  
  if( my_conf->priority < conf->priority && my_conf->status==VRRPD_ACTIVE ){
	my_conf->status=VRRPD_INACTIVE;
	if_down(my_conf->device);
  }
  else {
	vrrpd_timeouts[ conf->vrid ] = 0;
  }
  
  return 1;
}

vrrpd_conf* search_vrrpd_conf(int vrid){
  int i=0;
  while( (confs+i) && i <= MAX_VRRP_INSTANCE ){
	if( confs[i].vrid == vrid ) return &(confs[i]);
	i++;
  }
  return NULL;
}

int if_down(const char* device){
  int fd;
  struct ifreq ifr;
  struct sockaddr_in *s_in;
  
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  
  s_in = (struct sockaddr_in *)&ifr.ifr_addr;
  
  s_in->sin_family = AF_INET;
  s_in->sin_addr.s_addr = inet_addr("0.0.0.0");
  
  strncpy(ifr.ifr_name, device, IFNAMSIZ-1);
  
  if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
	perror("ioctl");
  }
  
  close(fd);
  return 1;
}

int if_up(const char* device, const char* hwaddr, const char* ipaddr, const char* netmask ){
  int fd;
  struct ifreq ifr;
  struct sockaddr_in *s_in;
  
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  
  s_in = (struct sockaddr_in *)&ifr.ifr_addr;
  
  s_in->sin_family = AF_INET;
  s_in->sin_addr.s_addr = inet_addr(ipaddr);
  
  strncpy(ifr.ifr_name, device, IFNAMSIZ-1);
  
  if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
	perror("ioctl");
  }
  
  s_in = (struct sockaddr_in *)&ifr.ifr_netmask;
  s_in->sin_addr.s_addr = inet_addr(netmask);
  
  if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0) {
	perror("ioctl");
  }

  close(fd);
  return 1;
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

