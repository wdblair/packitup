
#include <llvm/IR/Module.h>

llvm::Module *unpack_program(
	llvm::LLVMContext &context, 
	const char*start,
        size_t size 
);

const char * generatePassword();
