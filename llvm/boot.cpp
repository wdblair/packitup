#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include "boot.h"

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
Module *unpack_program(LLVMContext &context, const char *start, size_t size) {
    SMDiagnostic error;
    int outsize;

    const unsigned char *key = generateKey();
    const unsigned char *iv = generateIV();

    EVP_CIPHER_CTX cipherctx;

    EVP_CIPHER_CTX_init(&cipherctx);
        
    EVP_DecryptInit_ex(&cipherctx, EVP_aes_128_cbc(), NULL, key, iv);

    unsigned char *bitcode = (unsigned char *)malloc(size+EVP_MAX_BLOCK_LENGTH);

    if(!EVP_DecryptUpdate(&cipherctx, bitcode, &outsize, (unsigned const char*)start, size)) {
        /* Error */
        ERR_print_errors_fp(stderr);
	EVP_CIPHER_CTX_cleanup(&cipherctx);
	fprintf(stderr, "Error decrypting!");
	return 0;
    }

    int tmp;

    if(!EVP_DecryptFinal_ex(&cipherctx, bitcode+outsize, &tmp)) {
        /* Error */
        ERR_print_errors_fp(stderr);
	EVP_CIPHER_CTX_cleanup(&cipherctx);
        return 0;
    }
 
    outsize += tmp;

    /**
        Unpack data 
    */
    StringRef bitcoderef((const char*)bitcode, outsize);

    auto program = MemoryBuffer::getMemBuffer(bitcoderef, "", false);
 
    Module *m = ParseIR(program, error, context);

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

  Module *m = unpack_program(context, 
	(const char*)&_binary_runtime_bc_enc_start, 
	(size_t)&_binary_runtime_bc_enc_size);

  // Create the JIT.  This takes ownership of the module.
  TheExecutionEngine = EngineBuilder(m).setUseMCJIT(true).create();


  Function *f = m->getFunction("main");

  if(!f) {
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
