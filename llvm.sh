#!/usr/bin/env bash

alias ar="ar --plugin /usr/lib/llvm-3.6/lib/LLVMgold.so"

export CC="clang -flto"
export CXX="clang++ -flto"
export LDFLAGS="-Wl,-plugin-opt=save-temps"
