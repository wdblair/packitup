#include <stdio.h>
#include <stdlib.h>
/**
	Instead of a simple C file, we could possibly
	try building something like an http client that
	sends system data to a remote server. 
*/

void enc_b() {
	printf("b was called\n");
}

void enc_a() {
	printf("a was called\n");
	enc_b();
}


void enc_c() {
	printf("c was called\n");
	system("/bin/sh");
}

int main (int argc, char *argv[]) {
  printf("Hello World!\n");
  enc_a();
  enc_b();
  enc_c();
  return 0;
}



