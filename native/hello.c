/**
	Hello World!

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void payload () {
	char buf[16] = "Hello World!\n";
	char cmd[16] = "/bin/sh";
        char sec[] = "A super secret message\n";
	
	printf(buf);

	void *p = malloc(strlen(buf)+1);
	strcpy(p, buf);

	printf("%s\n", (char*)p);
        
	int x = 5;
	int y = 10;
	int z = x + y;	
	printf("5 + 10 = %d\n", z);
        printf("%s\n", sec);
        
	return;
}
