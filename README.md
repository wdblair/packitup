Pack! Pack! Pack it up!
=======================

Any code here is made purely for demonstration purposes for a class on software
security. It is developed out of academic curiosity and is _not_ intended to be 
used for constructing actual viruses/malware, nor would it be at all useful 
for such a purpose.


This repo contains the following directories.


demo
====

Scripts used for our demo to the class.

native
======

The first prototype that used gcc with binutils to encrypt and pack native code.

llvm
====

Our final prototype that packs programs compiled to LLVM bitcode into an executable.
At run-time, the bitcode is extracted and ran through the LLVM JIT, which is statically
linked with the executable.

r2
==

A demo showing a program that can resize the payload section of our packed executable.
The idea is that this could be implemented in our prototype's run-time so that the
malware could re-build itself on the fly.
