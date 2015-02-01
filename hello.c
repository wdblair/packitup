/**
	Hello World!

*/
#include <stdio.h>
#include <unistd.h>

void payload () {
	char buf[16] = "Hello World!\n";
	char cmd[16] = "/bin/sh";
	
	printf(buf);

	system(cmd);

	int x = 5;
	int y = 10;
	int z = x + y;	
	printf("5 + 10 = %d\n", z);
		
	return;
}
