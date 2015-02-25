#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/err.h>

/**
   Given a password and salt (provided by a python script, derive a key
*/
int main(int argc, const char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "usage: %s salt password", argv[0]);
        return 1;
    }

    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    const char *salt = argv[1];

    const int keylen = cipher->key_len + cipher->iv_len; 
    unsigned char *keybuf = (unsigned char*)malloc(keylen); 

    /**
        Get the system info as a password.
    */
    const char *password = argv[2];
 
    // printf("Password is %s\n", password);

    /**
        Derive a key for the password
    */
    if(!PKCS5_PBKDF2_HMAC(password, strlen(password),
                          (const unsigned char *)salt, 
                          strlen(salt), 10000, EVP_sha256(), 
                          keylen, keybuf)) {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }

    for(int i=0; i<keylen; ++i) { 
        printf("%02x", keybuf[i]); 
    } 
}
