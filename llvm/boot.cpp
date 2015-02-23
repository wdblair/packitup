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

static ExecutionEngine *TheExecutionEngine;

extern char _binary_runtime_bc_start;
extern char _binary_runtime_bc_end;
extern char _binary_runtime_bc_size;

/**
    Unpack a program referred to by data.
    The decrypting stub code will go here.
*/
Module *unpack_program(LLVMContext &context, StringRef data) {
    SMDiagnostic error;

    auto program = 
    MemoryBuffer::getMemBuffer(data, "", false);
 
    Module *m = ParseIR(program, error, context);

    if(!m) {
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

  StringRef runtime_data(&_binary_runtime_bc_start, 
			 (size_t)&_binary_runtime_bc_size);


  Module *m = unpack_program(context, runtime_data);

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

  FP(argc, argv);

  return 0;
}
