#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>


#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

std::string Encrypt(const std::string & in, int key)
{
    std::string out(in);
    for(int i=0; i < in.length(); ++i)
    {
        out[i] += key;
    }
    return out;
}

extern "C" void runit (const char *bitcode) {
  LLVMContext context;
  SMDiagnostic error;
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();

  std::string dec = Encrypt(std::string(bitcode), -10);

  auto program = MemoryBuffer::getMemBuffer(dec, "", false);
  Module* M = parseAssembly(program->getMemBufferRef(), error, context).release();
 
  if (!M) {
    std::cout << "ERR\n";
  }


  // Create the JIT.  This takes ownership of the module.
      std::string ErrStr;
  
  ExecutionEngine *TheExecutionEngine;
  TheExecutionEngine = EngineBuilder(std::unique_ptr<Module>(M)).setErrorStr(&ErrStr).create();

  if (!TheExecutionEngine) {
    std::cout << "ERR:" << ErrStr << "\n";
  }
     Module::iterator be = M->begin(), ee = M->end();
    for (; be != ee; ++be) {
      std::cout << "JIT found function: ";
      std::cout << be->getName().str() << '\n';
      
      } 

  if (!M) {
    std::cout << "ERR\n";
  }
  Function *f = M->getFunction("encrypted");
  
  if(!f) {
    std::cout << "Function payload not defined!" << std::endl;
    exit(1);
  }
 
  TheExecutionEngine->finalizeObject();
  
  // JIT the function, returning a function pointer.
  void *FPtr = TheExecutionEngine->getPointerToFunction(f);

  typedef int(*mainfunc)();

  mainfunc FP = (mainfunc)FPtr;
  
  FP();
}

