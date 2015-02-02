Pack Pack Pack It Up!
=====================

Integrating Packing with the Malware Software
Development Lifecycle.

Currently, we rip out instructions from an unlinked
object file, encrypt it, and then place it in the
final executable. On run-time, we extract the payload,
decrypt it, and then put it back where the executable
expects it to be (since all jumps to functions are made
relative to that address). If we statically link the executable,
this seems to work alright.

In order to run this code, you will need to have openssl
available as a static library. What's more, running this test
can be a little finicky. If you already have openssl installed through
a package manager and it does not have a static library available, you
will need to compile the static libs yourself. Unfortunately, I
found you also need to compile the shared libs as well, because I use
the openssl shell command to encrypt instructions (which uses the shared
libs). For whatever reason, I found encrypting and decrypting yields the slightest
differences in decrypting when mixing a static and dynamic version of openssl of
slightly different versions.
