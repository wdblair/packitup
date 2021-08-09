Pack It Up
==========

Any code here is made purely for demonstration purposes for a class on software
security. It is developed out of academic curiosity and is _not_ intended to be 
used for constructing actual viruses/malware, nor would it be at all useful 
for such a purpose.

High Level Design
=================

This packer uses LLVM's just-in-time compilation engine to execute whole programs stored as bitcode within executable programs. This allows developers to pack complex software into an executable that can only be decrypted when executed in an environment specified by the developer. This prevents analysts from running the code in an analysis environment.

Evaluating Our Code
===================

To build a bitcode version of nginx statically linked with libpcre 
(also in bitcode), run 

  buildnginx.sh

It will download libpcre and nginx and build them to a bitcode payload that
will be copied to packitup/llvm/payload.bc. Next, you can go into llvm and run

  make

When you generate the key file, please save it to key.cpp. For generating the
hostid, probably the easiest thing is to just make it based off of the language,
en_US.UTF-8. This will allow the malware to run in the container, and you can 
change the LANG environment variable to see the malware fail. It will display
a loading bar when it is on an invalid target. 

This creates the boot program. After you run it, nginx should be running on 
port 8080. Try viewing its output using the following

   curl http://localhost:8080/person/secret.txt

nginx is configured to set up a directory index at /home, so this lets you
remotely view the files in users' home directories.

Index
=====

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

llvm/functionpass
====

A proof-of-concept of encryption with function-level granularity.
Uses LLVM's JIT and Pass Framework to package malware in such a way that each function
is decrypted, run, and re-encrypted separately at runtime.

r2
==

A demo showing a program that can resize the payload section of our packed executable.
The idea is that this could be implemented in our prototype's run-time so that the
malware could re-build itself on the fly.
