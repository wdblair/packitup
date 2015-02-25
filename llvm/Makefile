all:: boot

boot: boot.cpp key.cpp runtime.o payload.o
	clang++ -g -std=c++11 -o $@ $^ \
	`llvm-config --cppflags --ldflags --libs core mcjit native irreader --system-libs` \
	-Wl,--export-dynamic -lpthread -lcrypt -lpcre -lcrypto -lz -lutil

# I don't think using bitcode produced by arbitrary C++ is entirely feasible yet.
# Using iostream requires some symbol that the execution can't find,
# but using -fno-use-cxa-atexit causes it to not need it
#
# http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-January/058748.html

runtime.bc: runtime.cpp payload.bc
	clang++ -flto -c -std=c++11 -o $@ $< -fno-use-cxa-atexit \
	`llvm-config --cppflags`

%.o: %.bc
	./encrypt.sh $< $<.enc
	objcopy -I binary -O elf64-x86-64 -B i386 $<.enc $@

payload.bc: payload.c
	clang -o $@ -flto -c $^

.phony: clean

clean:
	rm -f *.o *.enc boot genkey payload.bc runtime.bc
