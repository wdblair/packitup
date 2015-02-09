#
# Use a linker and objcopy as a malware packer
#
all: payload

hello.o: hello.c
	gcc -fPIC -c -o $@ $<
	
hello-payload.o: hello.o
	# Give the instructions a separate section
	objcopy --rename-section .text=.useless $< hello-payload.o

pack.o: pack.c
	gcc -g -c -o $@ $<

payload: hello-payload.o pack.o
	gcc -o payload hello-payload.o pack.o -T link.x -lcrypto -ldl -lz -static
	./transplant.sh


	
.phony: clean showpayload

showpayload: payload
	objdump -d -j.useless $<	

clean: 
	rm *.o payload *.text
