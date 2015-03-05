

An experiment in using LLVM to produce self-reproducing malware.

The plan for this feature (which is only an idea now), would be
to embed the radare2 library into the runtime payload. That way,
we could create copies of the malware executable and resize their
payload sections in order to put different modules or obfuscated
versions of their current modules inside of them.

The rs program can resize the payload section of our malware
and create a new, functioning copy. 
