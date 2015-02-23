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

Running Case Studies
====================

To try out an application that we compiled, run `make`, and then
copy the .bc file you want to run into `payload.bc`. Then, run
`make` again. We will revise the build system to allow the user to
specify the programs they wish to run.

case study: nginx
=================

Compiling nginx to bitcode and running it through the JIT works
really well. The nginx code is able to create a daemon process
and detach itself from the shell, spawn children, serve pages, all 
while being compiled on the fly inside the llvm JIT.

I have included the nginx.bc file since I haven't written down
the instructions to produce it yet from nginx source code. You 
just have to change which .bc file is loaded in vm.cpp (and 
have nginx installed since it reads from logs and config files, etc).

compiling nginx
===============

Lots of programs built with autotools are really finicky when you
want to use clang with the LLVM Gold plugin to produce bitcode for
object files instead of native code. Nonetheless, nginx's configure
script doesn't interrogate clang as much as others.

First, you need your system linker to use the Gold plugin. Next,
run

    export CC='clang -flto'
    export CXX='clang++ -flto'

Then go to the nginx source code and run the typical 

    ./configure
    make
    make install

Now, you will need to use llvm-link to put together all the object
files, which will be llvm bit code.

    llvm-link -o objs/nginx.bc *.o

The last command may not work, just be sure you give llvm-link all
the object files that comprise the nginx program.

Now, just copy nginx.bc to this directory and tell vm.cpp to run
nginx.bc. 

    ./vm

This should start the nginx webserver and you can see
webpages at localhost:80 (provided you don't have another web
server running already)

case study: calling llvm from within bitcode
============================================

If you look at load.cpp, you can see that we call the JIT
from code that is, itself, executed from within the JIT. The
reason we can do this is because when we build the executable (vm.cpp)
we tell the system linker to export all dynamic symbols.
This causes the statically linked LLVM functions to be available
for the JIT to use.

case study: running an ssh server
=================================

Unfortunately, building openssh using clang's link time optimization does
not entirely work. Given that the linker claimed there were syntax 
errors in the bitcode files produced for openssh, we figured it would be 
easier to try a simpler ssh tool.

The Dropbear ssh server is a perfect replacement given that it is 
relatively small and provides both an ssh server and client. The use 
case here is that the attacker can compile an ssh daemon to bitcode, 
wrap it up with our package, and then deploy it to their target 
environment.
