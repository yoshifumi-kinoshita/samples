#include <stdio.h>
#include <string.h>

int main(void){

	const char* s1 = "uidNumber: 13170";
	const char* s2 = "uid: oa";

	int result = strcoll(s1, s2);
	printf("Result: %d\n", result);

	result = memcmp(s1, s2, strlen(s2)-1);
	printf("Result: %d\n", result);

	exit(0);
}

