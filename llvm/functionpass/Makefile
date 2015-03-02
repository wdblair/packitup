# Optionally specify an include directory for LLVM source
LLVM_SRC := /home/john/llvm-3.6.0.src/

all:: runit.ll functionlevel payload.bc boot

.phony: clean

clean:
	rm -f *.o payload.bc boot.s boot.bc runit.ll runit.bc functionlevelpass.so

functionlevelpass.o: functionlevelpass.cpp
	g++ -o $@ $^ -c -g -fPIC -O0 -Wall `llvm-config --cxxflags --libs transformutils core ipo bitreader bitwriter asmparser irreader asmparser instrumentation scalaropts linker bitreader bitwriter irreader vectorize objcarcopts codegen mcjit native irreader` -Wextra -I$(LLVM_SRC)

functionlevelpass.so: functionlevelpass.o
	g++ -o $@ -shared $^ -Wl,-soname,$@ -fPIC -Wall -g

functionlevel: functionlevelpass.so

runit.bc: runit.cpp
	clang++ -flto -Wl,--no-as-needed -c -std=c++11 -o $@ $< -fno-use-cxa-atexit \
	`llvm-config --cppflags` -fPIC -O0

runit.ll: runit.bc
	llvm-dis $<

payload.bc: payload.c
	clang -o $@ -flto -c $^

boot.bc: payload.bc
	opt -load ./functionlevelpass.so -functionlevel < $< > $@

boot.s: boot.bc
	llc $<

boot: boot.s
	 g++ $< -o $@ `llvm-config --cppflags --ldflags --libs core mcjit native irreader --system-libs` -Wl,--export-dynamic -lpthread -lcrypt -lcrypto -lz -std=c++11