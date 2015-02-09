/**
    A beginning attempt at making a packing compiler.
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
        unsigned char iv[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
       	
	void *up = (void*)&useless_start;
	void *upend = (void*)&useless_end;
        
	int verbose = 0;

	if (argc > 1 && strcmp(argv[1], "-v") == 0) {
		verbose = 1;	
	}	 
 
        EVP_CIPHER_CTX ctx;

        EVP_CIPHER_CTX_init(&ctx);
        
        EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, key, iv);
 
	printf("Finished loading cipher\n");
	
	printf("Target section is at: %016x\n", &useless_start);
	
	/* Allow enough space in output buffer for additional block "EVP_MAX_BLOCK_LENGTH"*/
	
	int inlen = (((unsigned long)(upend)) - ((unsigned long)(up)));
	
	inlen += 16 - (inlen % 16); /** account for padding */
	
	printf ("Our encrypted payload has %d bytes\n", inlen);
	
	int outlen;
	
	/**
		mprotect requires that an address be aligned on a page size. 
		This is a little redundant since we no longer run the code from
		instr... (malloc should work fine).	
	*/
	const long pagesize = sysconf(_SC_PAGESIZE);
	
	void *instr = memalign(pagesize, inlen + EVP_MAX_BLOCK_LENGTH);

	if (!instr) {
		fprintf (stderr, "Malloc failed!\n");
		exit (1);
	}
		
	void *pack = up;
	
	if (verbose) {		
		printf("Encrypted buffer:\n");
		show_hex ((const char *)pack, inlen);
		printf("\n");
	}
	
	/**
		Decrypt the payload
	*/
	if(!EVP_DecryptUpdate(&ctx, instr, &outlen, (const unsigned char *)pack, inlen)) {
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
	
	/**
		Printing instructions from memory gives some weird results.
	if (verbose) {	
		printf("Decrypted buffer:\n");
		show_hex ((const char *)instr, outlen);
		printf("\n");
	}*/
	
	/**
		Find the page that the .useless section lies on.
	*/
	const unsigned long pageoffset = ((unsigned long)pack) % pagesize;
	void *page = pack - pageoffset;
	
	unsigned long pagelen = outlen;
	
	/**
		Make the length given to mprotect a multiple of pagesize.
	*/
	if(pagelen % pagesize != 0) {
		pagelen = outlen + (pagesize - (outlen % pagesize));
	}

	if(mprotect (page, pagelen, PROT_READ | PROT_WRITE) < 0) {
		fprintf(stderr, "mprotect failed!\n");
		return 1;
	}

	memcpy(pack, instr, outlen);

	/**
		Turn the permissions back to just read and execute
	*/
	if(mprotect (page, pagelen, PROT_READ | PROT_EXEC) < 0) {
		fprintf(stderr, "mprotect failed!\n");
		return 1;
	}
	
	/** Run the payload */
	void (*p)();
	
	p = (void (*)())pack;
	
	p();
	
	printf("Payload executed!\n");

	return 0;
}
