/**
    A beginning attempt at making a packing compiler.
    
    First, suppose the code we want to run is placed in
    its own section.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <sys/mman.h>

#include <openssl/evp.h>
#include <openssl/err.h>

extern
char useless_start;
extern
char useless_end;


void show_hex (const char *buf, int len) {
	/**
		Write out the contents of a buffer in hex where len is the
		size of buf in bytes.
	*/
	int i;
	
	for (i = 0; i < len; i++) {
		unsigned char c = buf[i];
		if (i > 0 && i % 4 == 0) {
			printf("  ");
		}
		printf ("%02x", c);

	}
}

int main (int argc, char *argv[]) {

	unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        unsigned char iv[] = {1,2,3,4,5,6,7,8};
        
 
        EVP_CIPHER_CTX ctx;

         /* Don't set key or IV right away; we want to check lengths */
         EVP_CIPHER_CTX_init(&ctx);
        
         EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, key, iv);
 
	printf("Finished loading cipher\n");
	
	printf("Target section is at: %016x\n", &useless_start);
	
	/* Allow enough space in output buffer for additional block "EVP_MAX_BLOCK_LENGTH"*/
	
	int inlen = (((unsigned long)(&useless_end)) - ((unsigned long)(&useless_start)));
	
	inlen += 16 - (inlen % 16); /** account for padding */
	
	printf ("Our encrypted payload has %d bytes\n", inlen);
	
	int outlen;
	
	/**
		mprotect requires that an address be aligned on a page size.
		
		void *instr = calloc (inlen + EVP_MAX_BLOCK_LENGTH, 1);
	*/
	const long pagesize = sysconf(_SC_PAGESIZE);
	
	void *instr = memalign(pagesize, inlen + EVP_MAX_BLOCK_LENGTH);
	
	
	/**
		Find the page that this section lies on
	*/
	
	const long pageoffset = ((unsigned long)instr) % pagesize;
		
	void *pack = &useless_start;

	
	if (!instr) {
		fprintf (stderr, "Malloc failed!\n");
		exit (1);
	}
	
	printf("Encrypted buffer:\n");
	show_hex ((const char *)pack, inlen);
	printf("\n");
	
	/**
		Decrypt the payload
	*/
	if(!EVP_DecryptUpdate(&ctx, instr, &outlen, (unsigned char *)pack, inlen)) {
		/* Error */
		EVP_CIPHER_CTX_cleanup(&ctx);
		fprintf(stderr, "Error decrypting!");
		return 0;
	}
		
	int tmp;
	
	if(!EVP_DecryptFinal_ex(&ctx, instr+outlen, &tmp)) {
		/* Error */
		ERR_print_errors_fp(stderr);
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
         }
         
         outlen += tmp;
         
	printf ("Our decrypted payload has %d bytes\n", outlen);
	
	printf("Decrypted buffer:\n");
	show_hex ((const char *)instr, outlen);
	printf("\n");
		
	if(mprotect (instr, outlen, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
		fprintf(stderr, "mprotect failed!\n");
		return 1;
	}
	
	/** Run the instructions */
	void (*p)();
	
	p = (void (*)())instr;
	
	p();
		
	return 0;
}
