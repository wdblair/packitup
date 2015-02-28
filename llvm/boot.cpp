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

/**
    Unpack a program referred to by data.
    The decrypting stub code will go here.
*/
Module* unpack_program(LLVMContext &context, const char *start, size_t size) {
    SMDiagnostic error;
    
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    const char *salt = "Uj_y6L*-mhc@77d";
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};
   
    const int keylen = cipher->key_len + cipher->iv_len; 
    unsigned char *keybuf = (unsigned char*)malloc(keylen); 

    /**
        Get the system info as a password.
    */
    const char *password = generatePassword();
   
    printf("Password is %s\n", password);

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

    printf("Key: "); 
    for(int i=0; i<cipher->key_len; ++i) { 
        printf("%02x", keybuf[i]); 
    } printf("\n");

    printf("IV: "); 
    for(int i=cipher->key_len; i<keylen; ++i) { 
        printf("%02x", keybuf[i]); 
    } printf("\n");
    
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
 
    Module* m = parseIR(program->getMemBufferRef(), error, context).release();

    if (!m) {
        cerr << "Module could not be found!" << endl;
        exit(1);
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

  Module* m = unpack_program(context, 
  (const char*)&_binary_runtime_bc_enc_start, 
  (size_t)&_binary_runtime_bc_enc_size);

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

  cout << "Running runtime!" << endl;

  FP(argc, argv);

  return 0;
}
