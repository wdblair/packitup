Pack Pack Pack It Up!
=====================

Integrating Packing with the Malware Software
Development Lifecycle.

Currently, we rip out the payload from a linked executable, encrypt it, and then place
it back into the executable. The linker script takes care of adding enough space to the
section holding the payload so that a padded ciphertext will fit inside of it. 
On run-time, we extract the payload, decrypt it, and then put it back where the executable
expects it to be (since all jumps to functions are made relative to that address). 
If we statically link the executable, this seems to work alright. The whole idea is that
you can develop your attack as a single C function "payload" and then our scripts automatically
build an executable where the attack code is entirely hidden until run-time.

The goal of this project is to provide a software development environment for constructing
packed executables from C code. Currently, this is an experiment to see what the output of
such a system would be. Our goal is to replicate this process in a more durable way by 
developing a custom pass to the llvm compiler.

With llvm, we could possibly take this obfuscation a step further beyond encryption. Since
llvm has backend code generators for _many_ different architectures, you could imagine that
the payload could be compiled to MIPS, and then a MIPS emulator in the packed binary would
emulate the payload.

In order to run this code, you will need to have openssl available as a static library. 

Any code here is made purely in an academic way for a class on software
security. It is developed in the pursuit of academic curiosity and is _not_ 
intended to be used for constructing actual viruses/malware, nor would
it be at all useful for such a purpose.
