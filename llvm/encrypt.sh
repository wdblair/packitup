#!/usr/bin/env bash

if=$1
of=$2

salt="Uj_y6L*-mhc@77d"
password=`python stub.py`

hash=`./genkey $salt $password`

key=`echo $hash | cut -c -32`
iv=`echo $hash | cut -c 33-`

openssl aes-128-cbc -in $if -out $of -p -K $key -iv $iv

echo $if encrypted!
