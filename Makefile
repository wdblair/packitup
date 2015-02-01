#
# Use a linker and objcopy as a malware packer
#
all: payload

hello.o: hello.c
	gcc -fPIC -c -o $@ $<
	
hello-payload.o: hello.o
	# Get the payload out of the binary
	objcopy -O binary -j .text hello.o hello.text
	
	# Separate the instructions (shouldn't be anything here)
	objcopy --rename-section .text=.useless $< hello-payload.o

pack.o: pack.c
	gcc -g -c -o $@ $<
	
payload: hello-payload.o pack.o
	gcc -o $@.o hello-payload.o pack.o -T link.x -lcrypto -ldl
	objcopy --set-section-flags .payload=alloc,load,contents,code $@.o $@
	./transplant.sh
	
.phony: clean

clean: 
	rm *.o payload
