#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;
using namespace std;

static ExecutionEngine *TheExecutionEngine;

int main(int argc, const char *argv[])
{
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();
  
  LLVMContext context;
  
  SMDiagnostic error;
  Module *m = ParseIRFile("runtime.bc", error, context);

  /** 
     Uncomment to see human readable IR
  */
  if(m)
  {
    // m->dump();
  } else {
    cerr << "Module could not be found!" << endl;
  }

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
