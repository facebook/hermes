#!/bin/bash

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android-clang-arm-api13-gnustl.cmake \
    -DLLVM_TARGETS_TO_BUILD= -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off \
    -DLLVM_TABLEGEN=/Users/tmikov/work/hws/llvm_build/bin/llvm-tblgen \
    -DCLANG_TABLEGEN=/Users/tmikov/work/hws/llvm_build/bin/clang-tblgen \
    -GNinja ../llvm
