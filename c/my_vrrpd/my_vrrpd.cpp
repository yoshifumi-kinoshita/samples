#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include "picojson.h"

#define PID_FILE "/var/run/my_vrrpd.pid"

void shutdown(int);
void write_pid();
void daemonize();

int main(int argc, char **argv){
  
  daemonize();
  
  write_pid();
  
  signal(SIGTERM, shutdown);
  
  while(1){
	sleep(1);
  }
}

void shutdown(int sig){
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
