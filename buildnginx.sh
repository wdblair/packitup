#!/usr/bin/env bash

wget http://nginx.org/download/nginx-1.7.10.tar.gz
wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.36.tar.gz 

source llvm.sh

cd nginx-1.7.10

make clean

cd ..

cd pcre-8.36

make clean

cd ..

cd nginx-1.7.10

./configure --with-pcre=../pcre-8.36


sed -i -- 's/^LINK.*/LINK = \$\(CC\) -Wl,-plugin-opt=save-temps/g' objs/Makefile

cd nginx-1.7.10

make -j4

cd ../

echo "Copying nginx.bc to payload.bc"

cp nginx-1.7.10/objs/nginx.bc /root/packitup/llvm/payload.bc
