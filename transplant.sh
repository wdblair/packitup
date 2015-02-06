#!/usr/bin/env bash

# Getting info from the elf file is a little cumbersome in the makefile, so
# so we do the encrypting in this file.

info=`readelf --sections -W payload | grep .useless | \
	awk '{ offset=strtonum("0x"$5); address=strtonum("0x"$4); 
		printf "%d %x", offset, address-offset }'`

startoffset=`echo $info | cut -f1 -d\ `
baseaddr=`echo $info | cut -f2 -d\ `

endoffset=`readelf -s payload | grep useless_end | \
	awk -v baseaddr=$baseaddr \
		'{ address=strtonum("0x"$2); base=strtonum("0x"baseaddr); 
			print address - base}'`

# Need to compute the size ourselves since the section's size includes
# padding
size=`expr $endoffset - $startoffset`

echo "Found .useless section at $startoffset with size $size"

# copy the linked code into its own file
dd if=./payload of=./payload.text bs=1 skip=$startoffset count=$size

# encrypt it
openssl aes-128-cbc -in payload.text -out payloadsecret.text \
	-K 000102030405060708090A0B0C0D0E0F -iv 0102030405060708

# put the encrypted info in the right place
dd if=./payloadsecret.text of=./payload bs=1 seek=$startoffset conv=notrunc
