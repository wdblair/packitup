#
# Use a linker and objcopy as a malware packer
#
all: payload

hello.o: hello.c
	gcc -fPIC -c -o $@ $<
	
hello-payload.o: hello.o
	# Get the payload out of the binary
	objcopy -O binary -j .text hello.o hello.text
	
	# Separate everything else (shouldn't be anything here)
	objcopy --rename-section .text=.useless $< hellotmp.o 
#		--rename-section .data=.pdata --rename-section .bss=.pbss \
#		--rename-section .rodata=.prodata $< hellotmp.o

	# Encrypt the program
	openssl aes-128-cbc -in hello.text -out hellosecret.text \
		-K 000102030405060708090A0B0C0D0E0F -iv 010203040506070
	
	# Add it back into our object file with the correct permissions
	objcopy --add-section .payload=hellosecret.text \
		--set-section-flags .payload=alloc,load,readonly,data,contents hellotmp.o hello-payload.o
	
pack.o: pack.c
	gcc -g -c -o $@ $<
	
payload: hello-payload.o pack.o
	gcc -o $@.o hello-payload.o pack.o -T link.x -lssl
	objcopy --remove-section .useless payload.o $@

.phony: clean

clean: 
	rm *.o payload
