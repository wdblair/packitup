#include <stdio.h>
#include <stdlib.h>

/**
	Instead of a simple C file, we could possibly
	try building something like an http client that
	sends system data to a remote server. 
*/
int main (int argc, char *argv[]) {
  printf("Hello World!\n");
  
  char *shell = "/bin/sh";
  system(shell);
  
  return 0;
}
