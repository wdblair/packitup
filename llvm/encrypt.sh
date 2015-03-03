#!/usr/bin/env bash

if=$1
of=$2

hash=`cat secret.key`

# python gives us a 32 byte value
# first 16 bytes - key
# second 16 bytes - iv
key=`echo $hash | cut -c -32`
iv=`echo $hash | cut -c 33-64`

openssl aes-128-cbc -in $if -out $of -p -K $key -iv $iv

echo $if encrypted!
