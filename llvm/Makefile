all:: boot

# Assume for the demo we are using nginx.
# dropbear (ssh) requires libutil, which we could compile to bitcode possibly

boot: boot.cpp key.cpp runtime.o payload.o verify.o
	clang++ -g -std=c++11 -o $@ $^ \
	`llvm-config --cppflags --ldflags --libs core mcjit native irreader --system-libs` \
	-Wl,--export-dynamic -lpthread -lcrypt -lcrypto -lz

derivekeys: derivekeys.cpp key.cpp
	clang++ `llvm-config --cxxflags` $^ -o $@ -lcrypto 

# I don't think using bitcode produced by arbitrary C++ is entirely feasible yet.
# Using iostream requires some symbol that the execution can't find,
# but using -fno-use-cxa-atexit causes it to not need it
#
# http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-January/058748.html

runtime.bc: runtime.cpp payload.bc
	clang++ -flto -c -std=c++11 -o $@ $< -fno-use-cxa-atexit \
	`llvm-config --cppflags`

hostid:
	python hie_cli.py

secret.key: derivekeys hostid
	./derivekeys `cat hostid`

%.o: %.bc secret.key
	./encrypt.sh $< $<.enc
	objcopy -I binary -O elf64-x86-64 -B i386 $<.enc $@

verify.o: verify.key
	objcopy -I binary -O elf64-x86-64 -B i386 $< $@

payload.bc: payload.c
	clang -o $@ -flto -c $^

.phony: clean

clean:
	rm -f *.o *.enc boot genkey \
		payload.bc runtime.bc verify.key secret.key \
		hostid
