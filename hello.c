/**
	Hello World!

*/
#include <stdio.h>
#include <unistd.h>

void payload () {
	char buf[16] = "Hello World!\n";
	char cmd[16] = "/bin/sh";
	/**
	
	We cannot use libc functions because relocations
        for this file are not described in the final
	executable. We could get around this, but this is a
	suitable demo for when we implement this in llvm.
 
	printf(buf);
	*/
	system(cmd);
	int x = 5;
	int y = 10;
	int z = x + y;
	
	int f;
	if (x < 2) {
		f = y / 2;
	} else {
		f = x / 2;
	}
		
	return;
}
