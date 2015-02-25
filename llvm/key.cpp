#include "boot.h"

static unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; 
static unsigned char iv[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

/**
   The process to generate a key.
   Put the key generation procedure here. Right
   now we just give a pointer to a static key. 
*/
const unsigned char * generateKey() {
	return key;
}

const unsigned char * generateIV() {
	return iv;
}
