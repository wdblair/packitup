#!/usr/bin/env bash

# Getting info from the elf file is a little cumbersome in the makefile, so
# so we do the encrypting in this file.

# 1. encrypt payload code
# 2. zero out the code in the executable
# 3. write the encrypted code into the .payload section of the executable

target_info=`readelf --sections -W ./payload | grep .useless | \
	awk '{offset=strtonum("0x"$5); count=strtonum("0x"$6); print offset, count}'`

payload_info=`readelf --sections -W ./payload | grep .payload | \
	awk '{offset=strtonum("0x"$5); count=strtonum("0x"$6); print offset, count}'`

# split up the useful information

target_offset=`echo $target_info | cut -f1 -d\ `
target_size=`echo $target_info | cut -f2 -d\ `

payload_offset=`echo $payload_info | cut -f1 -d\ `
payload_size=`echo $payload_info | cut -f2 -d\ `

echo "Found target section at $target_offset with size $target_size"
echo "Found payload section at $payload_offset with size $payload_size"

# copy the linked code into its own file
dd if=./payload of=./payload.text bs=1 skip=$target_offset count=$target_size

# null out our payload in the executable
dd if=/dev/zero of=./payload bs=1 seek=$target_offset count=$target_size conv=notrunc

# encrypt it
openssl aes-128-cbc -in payload.text -out payloadsecret.text \
	-K 000102030405060708090A0B0C0D0E0F -iv 010203040506070

# put the encrypted info in the right place
dd if=./payloadsecret.text of=./payload bs=1 seek=$payload_offset count=$payload_size conv=notrunc
