#!/bin/bash

home=$1

INC_PATH="$home/fbsource/xplat"
INCS="-I $INC_PATH/hermes/include"
INCS+=" -I $INC_PATH/hermes/config"
INCS+=" -I $INC_PATH/hermes/public"
INCS+=" -I $INC_PATH/third-party/llvm/src/include"
INCS+=" -I $INC_PATH/third-party/llvm/gen-osx-host/include"

LIB_PATH="$home/fbsource/xplat/buck-out/gen"
LIBS="$LIB_PATH/hermes/lib/BCGen/BCGen#macosx-x86_64,static/libBCGen.a"
LIBS+=" $LIB_PATH/hermes/lib/BCGen/HBC/HBC#macosx-x86_64,static/libHBC.a"
LIBS+=" $LIB_PATH/hermes/lib/Regex/Regex#macosx-x86_64,static/libRegex.a"
LIBS+=" $LIB_PATH/hermes/lib/VM/VMLean#macosx-x86_64,static/libVMLean.a"
LIBS+=" $LIB_PATH/hermes/lib/VM/CPPUtil/CPPUtil#macosx-x86_64,static/libCPPUtil.a"
LIBS+=" $LIB_PATH/hermes/lib/Support/Support#macosx-x86_64,static/libSupport.a"
LIBS+=" $LIB_PATH/hermes/external/dtoa/dtoa#macosx-x86_64,static/libdtoa.a"
LIBS+=" $LIB_PATH/hermes/external/dtoa/dtoa-locks#macosx-x86_64,static/libdtoa-locks.a"
LIBS+=" $LIB_PATH/hermes/lib/Inst/Inst#macosx-x86_64,static/libInst.a"
LIBS+=" $LIB_PATH/third-party/llvm/support-c#macosx-x86_64,static/libsupport-c.a"
LIBS+=" $LIB_PATH/third-party/llvm/LLVMSupport#macosx-x86_64,static/libLLVMSupport.a"
LIBS+=" $LIB_PATH/fbsystrace/fbsystrace#macosx-x86_64,static/libfbsystrace.a"

FLAGS="-std=c++11"
FLAGS+=" -DHERMESVM_GC_NONCONTIG_GENERATIONAL"
FLAGS+=" -DHERMES_ENABLE_DEBUGGER"
FLAGS+=" -DHERMESVM_GCCELL_ID"
FLAGS+=" -DHERMESVM_CPP_RUNTIME"
FLAGS+=" -framework CoreFoundation"
FLAGS+=" -ltermcap"
FLAGS+=" -licucore"
FLAGS+=" -lcurses"
FLAGS+=" -lz"
FLAGS+=" -lm"
FLAGS+=" -g"
FLAGS+=" -fno-exceptions"
FLAGS+=" -fno-rtti"
FLAGS+=" -Werror"
FLAGS+=" -Wall"

tmpfile=$(mktemp)
# shellcheck disable=SC2086
c++ $INCS $LIBS $FLAGS -xc++ - -o "$tmpfile"
"$tmpfile"
