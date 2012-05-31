#include <stdio.h>
#include <sys/timex.h>
#include <errno.h>

int set_status(int val){
	struct timex buf;
	buf.modes = ADJ_STATUS;
	buf.status = val;
	return adjtimex( &buf );
}

int main( int argc, char *argv[] ){
	if( argc == 1 ){
		printf("Usage: my_adjtimex ORed_param\n\ 
                    1   PLL updates enabled \n\
                    2   PPS freq discipline enabled \n\
                    4   PPS time discipline enabled \n\
                    8   frequency-lock mode enabled \n\
                   16   inserting leap second \n\
                   32   deleting leap second \n\
                   64   clock unsynchronized \n\
                  128   holding frequency \n\
                  256   PPS signal present \n\
                  512   PPS signal jitter exceeded \n\
                 1024   PPS signal wander exceeded \n\
                 2048   PPS signal calibration error \n\
                 4096   clock hardware fault \n\
");
		exit(-1);
	}
	int result = set_status( atoi(argv[1]) );
	printf( "Status: %d\nErrno: %d", result, errno );
	exit(0);
}
