
#include <llvm/IR/Module.h>

llvm::Module *unpack_program(
	llvm::LLVMContext &context, 
	const char*start,
        size_t size 
);

extern const char *verSalt;
extern const char *decryptSalt;

const char *getHostId();
