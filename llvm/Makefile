all:: vm payload.bc
	
vm: vm.cpp
	clang++ -g -std=c++11 -o $@ $^ `llvm-config --cppflags --ldflags --libs core mcjit native irreader --system-libs` -lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz
 
payload.bc: payload.c
	clang -o $@ -flto -c $^

.phony: clean

clean:
	rm -f vm payload.bc
