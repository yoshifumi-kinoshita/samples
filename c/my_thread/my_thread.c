#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

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

