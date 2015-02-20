#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;
using namespace std;

static ExecutionEngine *TheExecutionEngine;

extern char _binary_runtime_bc_start;
extern char _binary_runtime_bc_end;
extern char _binary_runtime_bc_size;

int main(int argc, const char *argv[])
{
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  LLVMContext context;
  SMDiagnostic error;

  StringRef runtime_data(&_binary_runtime_bc_start, 
			 (size_t)&_binary_runtime_bc_size);
  auto runtime = 
	MemoryBuffer::getMemBuffer(runtime_data, "", false);
 
  Module *m = ParseIR(runtime, error, context);

  if(!m)
  {
    cerr << "Module could not be found!" << endl;
    exit(1);
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
