LLVM JIT Demo
=============

This is a demo of using the LLVM Just in Time Compiler
as an unpacker of sorts. We should be able to modify
this such that the bitcode is encrypted (which can be
part of a compiler pass) and that the vm.cpp file decrypts
the bitcode and runs it.

I got the lighttpd webserver to compile to LLVM bitcode
and run this way, but it didn't work because the JIT does
not export symbols defined in the bitcode to loaded modules.
Applications that work this way will not run with our approach,
but as a malware author they wouldn't be my concern since they
depend on external modules in the system.

Note, you _can_ have calls to dlopen in the payload and the JIT
will happily load the files for you, but if those modules depend
on symbols _inside_ of your malware, then that won't work.


nginx
=====

Compiling nginx to bitcode and running it through the JIT works
really well. The nginx code is able to create a daemon process
and detach itself from the parent, all from being executed from
the llvm bitcode. 

I have included the nginx.bc file since I haven't written down
the instructions to produce it yet. You just have to change
which .bc file is loaded in vm.cpp (and have nginx installed since
it reads from logs and config files, etc).. 
