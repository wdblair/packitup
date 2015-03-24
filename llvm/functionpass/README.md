functionpass
=======================

This is a proof-of-concept implementation of an LLVM ModulePass which provides 
function-level encryption of an input payload C++ file.

To run, simply use

	make

To view the human-readable IR assembly of the payload, convert the boot.bc file
to a boot.ll file

	llvm-dis boot.bc

You can then see the encrypted global variables for each function of interest 
in the payload, the new bodies of the payload functions, and the runit() function
that they reference.

	cat boot.ll | grep enc_

Note that runit.ll contains the human-readable LLVM IR of the stub responsible for
decrypting and running functions with LLVM's JIT. Its implementation is in runit.cpp.
