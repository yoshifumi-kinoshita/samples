#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	usleep(atoi(argv[1]) * 1000000);
	exit(0);
}

