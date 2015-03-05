#include <iostream>

#include "boot.h"

#include <string.h>

#include <openssl/evp.h>
#include <openssl/err.h>

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hostid\n", argv[0]);
		return 1;
	}
	
	const char *password = argv[1];
	const EVP_CIPHER *cipher = EVP_aes_128_cbc();

	const char *salt = decryptSalt;
	const char *verify_salt = verSalt;
 
	printf("Using password = %s\n", password);
 
        printf("Using %s as the encryption salt\n", salt);
        printf("Using %s as the verification salt\n", verify_salt);

	unsigned char key[16] = {0};
	unsigned char iv[16] = {0};
   
	const int keylen = cipher->key_len + cipher->iv_len; 
	unsigned char *keybuf = (unsigned char*)malloc(keylen);
	unsigned char *verify_keybuf = (unsigned char*)malloc(keylen);

	printf("Keylen is %d\n", keylen);
	
	/**
		Derive the encryption key and a verification key.
    	*/
	if(!PKCS5_PBKDF2_HMAC(password, strlen(password),
        	(const unsigned char *)salt, 
                strlen(salt), 10000, EVP_sha256(), 
                keylen, keybuf)) {
		fprintf(stderr, "EVP_BytesToKey failed\n");
		exit(1);
	}

	if(!PKCS5_PBKDF2_HMAC(password, strlen(password),
        	(const unsigned char *)verify_salt, 
                strlen(verify_salt), 10000, EVP_sha256(), 
                keylen, verify_keybuf)) {
		fprintf(stderr, "EVP_BytesToKey failed\n");
		exit(1);
	}

	FILE *fp;

	fp = fopen("verify.key", "w");

	int written = fwrite(verify_keybuf, sizeof(char), keylen, fp);
	
	assert(written == keylen);
	
	fclose(fp);

	fp = fopen("secret.key", "w");

	printf("Saving encryption key.\n");
	for(int i = 0; i < keylen; i++) {
		fprintf(fp, "%02x", keybuf[i]);
	}
        printf("\n");

	fclose(fp);
}
