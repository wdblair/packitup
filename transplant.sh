#!/usr/bin/env bash

# Getting info from the elf file is a little cumbersome in the makefile, so
# so we do the encrypting in this file.

target_offset=`echo $target_info | cut -f1 -d\ `

target_offset=`readelf --sections -W payload | grep useless | awk '{ offset=strtonum("0x"$5); print offset }'`

end_offset=`readelf -s payload | grep useless_end | awk '{ address=strtonum("0x"$2); print address - 0x400000 }'`

target_size=`expr $end_offset - $target_offset`

echo "Found target section at $target_offset with size $target_size"

# copy the linked code into its own file
dd if=./payload of=./payload.text bs=1 skip=$target_offset count=$target_size

# null out our payload in the executable
#dd if=/dev/zero of=./payload bs=1 seek=$target_offset count=$target_size conv=notrunc

# encrypt it
openssl aes-128-cbc -in payload.text -out payloadsecret.text \
	-K 000102030405060708090A0B0C0D0E0F -iv 0102030405060708

# put the encrypted info in the right place
dd if=./payloadsecret.text of=./payload bs=1 seek=$target_offset conv=notrunc