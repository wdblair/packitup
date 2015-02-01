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
