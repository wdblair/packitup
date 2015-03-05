#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileUtilities.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/CodeExtractor.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <streambuf>

using namespace llvm;

namespace {

// Basic Caesar cipher encryption.
// Cited: http://stackoverflow.com/questions/12761720/c-simple-caesar-cipher-algorithm
std::string Encrypt(const std::string & in, int key)
{
    std::string out(in);
    for(int i=0; i < in.length(); ++i)
    {
        out[i] += key;
    }
    return out;
}

/* Some (modified) methods from llvm/lib/Transforms/IPO/ExtractGV.cpp,
   for extracting portions of modules. 
*/
static void makeVisible(GlobalValue &GV, bool Delete) {
  bool Local = GV.hasLocalLinkage();
  if (Local || Delete) {
    GV.setLinkage(GlobalValue::ExternalLinkage);
    if (Local)
      GV.setVisibility(GlobalValue::HiddenVisibility);
    return;
  }

  if (!GV.hasLinkOnceLinkage()) {
    assert(!GV.isDiscardableIfUnused());
    return;
  }

  // Map linkonce* to weak* so that llvm doesn't drop this GV.
  switch(GV.getLinkage()) {
  default:
    llvm_unreachable("Unexpected linkage");
  case GlobalValue::LinkOnceAnyLinkage:
    GV.setLinkage(GlobalValue::WeakAnyLinkage);
    return;
  case GlobalValue::LinkOnceODRLinkage:
    GV.setLinkage(GlobalValue::WeakODRLinkage);
    return;
  }
}

class GVExtractorPass : public ModulePass {
  SetVector<GlobalValue *> Named;
  bool deleteStuff;
  public:
  static char ID;

  explicit GVExtractorPass(std::vector<GlobalValue*>& GVs, bool deleteS = true)
    : ModulePass(ID), Named(GVs.begin(), GVs.end()), deleteStuff(deleteS) {}

  bool runOnModule(Module &M) override {
    for (Module::global_iterator I = M.global_begin(), E = M.global_end();
         I != E; ++I) {
      bool Delete =
        deleteStuff == (bool)Named.count(I) && !I->isDeclaration();
      if (I->getName().find("llvm.global_ctors") != std::string::npos)
        continue;
      if (!Delete) {
        if (I->hasAvailableExternallyLinkage())
          continue;
        if (I->getName().find("llvm.global_ctors") != std::string::npos)
          continue;
      }
      // Make the global variable visible.
      makeVisible(*I, Delete);

      // Don't actually delete the variable. 
      // Otherwise, functions referencing global variables
      // such as constant strings will break.

    }

    // Visit the Functions.
    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
      if (!I) {
        continue;
      }
      bool Delete =
        deleteStuff == (bool)Named.count(I) && !I->isDeclaration();
      if (!Delete) {
        if (I->hasAvailableExternallyLinkage())
          continue;
      }

      makeVisible(*I, Delete);

      // Delete bodies of other functions. This accomplishes function-level
      // encryption. Leave the prototypes.

      if (Delete)
        I->deleteBody();

      // Rename the target function to "encrypted" so that runit() stub
      // can identify which function is the one of interest.
      if (Named.count(I)) {
        errs() << "Renamed " << I->getName() << " to encrypted()\n";
        I->setName("encrypted");
      }
    }

    // Visit the Aliases.
    for (Module::alias_iterator I = M.alias_begin(), E = M.alias_end();
         I != E;) {
      Module::alias_iterator CurI = I;
      ++I;

      bool Delete = deleteStuff == (bool)Named.count(CurI);
      makeVisible(*CurI, Delete);

      if (false) {//Delete) {
        Type *Ty =  CurI->getType()->getElementType();

        CurI->removeFromParent();
        llvm::Value *Declaration;
        if (FunctionType *FTy = dyn_cast<FunctionType>(Ty)) {
          Declaration = Function::Create(FTy, GlobalValue::ExternalLinkage,
                                         CurI->getName(), &M);

        } else {
          Declaration =
            new GlobalVariable(M, Ty, false, GlobalValue::ExternalLinkage,
                               nullptr, CurI->getName());

        }
        CurI->replaceAllUsesWith(Declaration);
        delete CurI;
      }
    }

    return true;
  }
};
char GVExtractorPass::ID = 0;
/* End code adapted from llvm/lib/Transforms/IPO/ExtractGV.cpp */

};  //  namespace

namespace llvm {

/* My pass :) */
struct FunctionLevelPass : public ModulePass {
  static char ID;
  FunctionLevelPass() : ModulePass(ID) {}

  virtual bool runOnModule(Module& m) {
    return this->_runOnModule(m);
  }

private:
  bool _runOnModule(Module& m) {
    bool runit_imported = false;  // Have we imported runit()?
    static Function *runit;  // Will hold reference to runit(), which we only import once.

    // Iterate over the functions of the module.
    Module::iterator b = m.begin(), e = m.end();
    for (; b != e; ++b) {
      // Is function of interest?
      if (b->getName().str() == "main" || b->getName().str().find("enc_") != std::string::npos) {
        errs() << "Module pass found function of interest:" << b->getName() << "\n";
        
        // Clone the module. We're going to only keep the function of interest.
        // We'll get rid of other functions with a GVExtractionPass (implemented above).
        Module* M = CloneModule(&m);

        if (!M) {
          errs() << "error";
          return 1;
        }

        // Use SetVector to avoid duplicates.
        SetVector<GlobalValue *> GVs;

        errs() << "Extracting " << b->getName() << ", placing in new module...\n";
        GlobalValue *GV = M->getFunction(b->getName());
        if (!GV) {
          errs() << "error\n";
          return 1;
        }
        GVs.insert(GV);  // We'll only keep a single GV, the function of interest.

        // Materialize requisite global values.
        for (size_t i = 0, e = GVs.size(); i != e; ++i) {
          GlobalValue *GV = GVs[i];
          if (std::error_code EC = GV->materialize()) {
            errs() << "error\n";
            return 1;
          }
        }

        // It's important that we maintain context, otherwise we'll get reference errors
        // when we destroy objects after the Pass is done.
        LLVMContext *context = &getGlobalContext();

        llvm::legacy::PassManager Passes;
        Passes.add(new DataLayoutPass()); // Use correct DataLayout
    
        std::vector<GlobalValue*> Gvs(GVs.begin(), GVs.end());

        Passes.add(createGVExtractionPass(Gvs, false));  // Delete unwanted extra function bodies.
        Passes.add(createGlobalDCEPass());  // Delete unreachable globals
        Passes.add(createStripDeadDebugInfoPass());  // Remove dead debug info
        Passes.add(createStripDeadPrototypesPass());  // Remove dead func decls
  
        std::error_code ErrInfo;
        std::string o_str;
        llvm::raw_ostream *out = new llvm::raw_string_ostream(o_str);

        // This seems to be the best way to get the IR of the module as a string.
        Passes.add(createPrintModulePass(*out));
        Passes.run(*M);
        // Do encryption...
        std::string o_str_e = Encrypt(o_str, 10);
        llvm::Constant* stringConstant = llvm::ConstantDataArray::getString(*context, o_str_e, true);

        // Create a new GV with the encrypted module contents.
        // Remember, the module contains only the function of interest (renamed as "encrypted")
        // and prototypes for other methods, as well as declarations for referenced global variables.
        std::string newname = std::string("enc_") + b->getName().str();

        errs() << "Creating global variable '" << newname << "' with encrypted contents of new module\n";
        llvm::GlobalVariable *g = new llvm::GlobalVariable(m,
                         stringConstant->getType(),
                         true,
                         llvm::GlobalValue::ExternalLinkage,
                         stringConstant,
                         newname);

        if (runit_imported==false) {
          // Import the runit() stub, which accepts a bitcode string, decrypts it, and runs the
          // "encrypted" function in LLVM's JIT.
          runit_imported = true;
  
          errs() << "Importing JIT stub into module...\n";
          std::ifstream t("runit.ll");  // Pre-compiled stub.
          std::string str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

          // Now that we have the IR, we need to parse it as a module.
          auto program = MemoryBuffer::getMemBuffer(str, "", false);
          SMDiagnostic error;
          Module* M2 = parseAssembly(program->getMemBufferRef(), error, *context).release();
          if (!M2) {
            errs() << "error\n";
          }      

          // Unfortunately, we now have the pain of bringing the runit() function from its own
          // module into the one that contains the payloads. Easier said than done...
          // Code below is adapted from llvm/lib/Transforms/Utils/CloneModule.cpp.
          SmallVector<ReturnInst*, 8> Returns; 
          ValueToValueMapTy VMap;

          // Import GVs, without initializers. This will contain a LOT of LLVM symbols
          // and is critical for success of the JIT from runit().
          for (Module::const_global_iterator I = M2->global_begin(), E = M2->global_end();
                 I != E; ++I) {
              GlobalVariable *GV = new GlobalVariable(m, 
                                                      I->getType()->getElementType(),
                                                      I->isConstant(), I->getLinkage(),
                                                      (Constant*) nullptr, I->getName(),
                                                      (GlobalVariable*) nullptr,
                                                      I->getThreadLocalMode(),
                                                      I->getType()->getAddressSpace());
              GV->copyAttributesFrom(I);
              VMap[I] = GV;
            }

          // Now, the function prototypes, including lots of JIT stuff.
          for (Module::const_iterator I = M2->begin(), E = M2->end(); I != E; ++I) {
            Function *NF =
              Function::Create(cast<FunctionType>(I->getType()->getElementType()),
                               Function::ExternalLinkage, I->getName(), &m);
            NF->copyAttributesFrom(I);
            VMap[I] = NF;
          }

          // Loop over the aliases in the module.
          for (Module::const_alias_iterator I = M2->alias_begin(), E = M2->alias_end();
               I != E; ++I) {
            auto *PTy = cast<PointerType>(I->getType());
            auto *GA =
                GlobalAlias::create(PTy->getElementType(), PTy->getAddressSpace(),
                                    I->getLinkage(), I->getName(), &m);
            GA->copyAttributesFrom(I);
            VMap[I] = GA;
          }

          // And global initializers.
          for (Module::const_global_iterator I = M2->global_begin(), E = M2->global_end();
               I != E; ++I) {

            GlobalVariable *GV = cast<GlobalVariable>(VMap[I]);
            if (I->hasInitializer())
              GV->setInitializer(MapValue(I->getInitializer(), VMap));
          }

          // Function bodies come next, now that we have the prototypes.
          for (Module::const_iterator I = M2->begin(), E = M2->end(); I != E; ++I) {
            Function *F = cast<Function>(VMap[I]);
            if (!I->isDeclaration()) {
              Function::arg_iterator DestI = F->arg_begin();
              for (Function::const_arg_iterator J = I->arg_begin(); J != I->arg_end();
                   ++J) {
                DestI->setName(J->getName());
                VMap[J] = DestI++;
              }

              SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
              CloneFunctionInto(F, I, VMap, /*ModuleLevelChanges=*/true, Returns);
            }
          }

          // Aliases
          for (Module::const_alias_iterator I = M2->alias_begin(), E = M2->alias_end();
                 I != E; ++I) {
              GlobalAlias *GA = cast<GlobalAlias>(VMap[I]);
              if (const Constant *C = I->getAliasee())
                GA->setAliasee(MapValue(C, VMap));
            }

          // And named metadata....
          for (Module::const_named_metadata_iterator I = M2->named_metadata_begin(),
                 E = M2->named_metadata_end(); I != E; ++I) {
            const NamedMDNode &NMD = *I;
            NamedMDNode *NewNMD = m.getOrInsertNamedMetadata(NMD.getName());
            for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
              NewNMD->addOperand(MapMetadata(NMD.getOperand(i), VMap));
          }

          // Finally!
          runit = m.getFunction("runit");
        }

        if (!runit) {
          errs() << "error\n";
        }

        // Now we need to construct a call to runit() to replace the original function body.
        // Thanks to the LLVMdev mailing list for this one...
        // Cited: http://lists.cs.uiuc.edu/pipermail/llvmdev/2012-September/053080.html
        std::vector<Constant*> const_ptr_7_indices;
        ConstantInt* const_int32_8 = ConstantInt::get(*context, APInt(32, StringRef("0"), 10));
        const_ptr_7_indices.push_back(const_int32_8);
        const_ptr_7_indices.push_back(const_int32_8);
        // A reference to the global variable with the encrypted contents of the original function.
        Constant* const_ptr_7 = ConstantExpr::getGetElementPtr(g, const_ptr_7_indices);

      
        errs() << "Deleting old function body; replacing with call to JIT stub...\n";
        std::vector<Value*> args;
        CallInst *A = CallInst::Create(runit, const_ptr_7);  // Our call instruction.
    
        BasicBlock* block = BasicBlock::Create(*context, "entry");
        IRBuilder<> builder(block);
        builder.Insert(A);
    
        // Right now we only support main() and void methods.
        if (b->getName().str() == "main") {
          builder.CreateRet(builder.getInt32(0));  // Return success.
        } else {
          builder.CreateRetVoid();  // Return void.
        }

        // Replace function body.
        b->getBasicBlockList().clear();
        b->getBasicBlockList().push_front(block);
      }
    }

    return true;  // Tell LLVM we made some modifications ;)
  }
};

}  // namespace llvm

/* Below is code imported from llvm/lib/Transforms/Utils/CloneModule.cpp.
   Without it, opt complains.
*/
ModulePass *llvm::createGVExtractionPass(std::vector<GlobalValue *> &GVs,
                                         bool deleteFn) {
  return new GVExtractorPass(GVs, deleteFn);
}

Module *llvm::CloneModule(const Module *M) {
  // Create the value map that maps things from the old module over to the new
  // module.
  ValueToValueMapTy VMap;
  return CloneModule(M, VMap);
}

Module *llvm::CloneModule(const Module *M, ValueToValueMapTy &VMap) {
  // First off, we need to create the new module.
  Module *New = new Module(M->getModuleIdentifier(), M->getContext());
  New->setDataLayout(M->getDataLayout());
  New->setTargetTriple(M->getTargetTriple());
  New->setModuleInlineAsm(M->getModuleInlineAsm());
   
  // Loop over all of the global variables, making corresponding globals in the
  // new module.  Here we add them to the VMap and to the new Module.  We
  // don't worry about attributes or initializers, they will come later.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    GlobalVariable *GV = new GlobalVariable(*New, 
                                            I->getType()->getElementType(),
                                            I->isConstant(), I->getLinkage(),
                                            (Constant*) nullptr, I->getName(),
                                            (GlobalVariable*) nullptr,
                                            I->getThreadLocalMode(),
                                            I->getType()->getAddressSpace());
    GV->copyAttributesFrom(I);
    VMap[I] = GV;
  }

  // Loop over the functions in the module, making external functions as before
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    Function *NF =
      Function::Create(cast<FunctionType>(I->getType()->getElementType()),
                       I->getLinkage(), I->getName(), New);
    NF->copyAttributesFrom(I);
    VMap[I] = NF;
  }

  // Loop over the aliases in the module
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    auto *PTy = cast<PointerType>(I->getType());
    auto *GA =
        GlobalAlias::create(PTy->getElementType(), PTy->getAddressSpace(),
                            I->getLinkage(), I->getName(), New);
    GA->copyAttributesFrom(I);
    VMap[I] = GA;
  }
  
  // Now that all of the things that global variable initializer can refer to
  // have been created, loop through and copy the global variable referrers
  // over...  We also set the attributes on the global now.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    GlobalVariable *GV = cast<GlobalVariable>(VMap[I]);
    if (I->hasInitializer())
      GV->setInitializer(MapValue(I->getInitializer(), VMap));
  }

  // Similarly, copy over function bodies now...
  //
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    Function *F = cast<Function>(VMap[I]);
    if (!I->isDeclaration()) {
      Function::arg_iterator DestI = F->arg_begin();
      for (Function::const_arg_iterator J = I->arg_begin(); J != I->arg_end();
           ++J) {
        DestI->setName(J->getName());
        VMap[J] = DestI++;
      }

      SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
      CloneFunctionInto(F, I, VMap, /*ModuleLevelChanges=*/true, Returns);
    }
  }

  // And aliases
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    GlobalAlias *GA = cast<GlobalAlias>(VMap[I]);
    if (const Constant *C = I->getAliasee())
      GA->setAliasee(MapValue(C, VMap));
  }

  // And named metadata....
  for (Module::const_named_metadata_iterator I = M->named_metadata_begin(),
         E = M->named_metadata_end(); I != E; ++I) {
    const NamedMDNode &NMD = *I;
    NamedMDNode *NewNMD = New->getOrInsertNamedMetadata(NMD.getName());
    for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
      NewNMD->addOperand(MapMetadata(NMD.getOperand(i), VMap));
  }

  return New;
}
/* End of imported code */

// Finally, register our pass.
char FunctionLevelPass::ID = 0;
static RegisterPass<FunctionLevelPass> X("functionlevel", "packitup function-level encryption", false, false);