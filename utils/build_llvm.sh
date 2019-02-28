#!/usr/bin/env bash

set -x
set -e

# We assume this script is run from the Hermes directory, that it is
# not copied elsewhere.  This is necessary because it references files
# relative to the script directory.  So save the SCRIPT_DIR.
# This trick is from
# https://stackoverflow.com/questions/59895/get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

START_DIR="$PWD"
function finish () {
  cd "$START_DIR"
}

trap finish EXIT

# HERMES_WS_DIR is the root directory for LLVM checkout and build dirs.
[ -z "$HERMES_WS_DIR" ] && echo "HERMES_WS_DIR must be set" >&2 && exit 1
[ "${HERMES_WS_DIR:0:1}" != "/" ] && echo "HERMES_WS_DIR must be an absolute path" >&2 && exit 1

mkdir -p "$HERMES_WS_DIR"
cd "$HERMES_WS_DIR"

LLVM_REV=4519ac3791135eb9c207f0684f4236dbc13ac83f

BUILD_SYSTEM="${BUILD_SYSTEM:-Ninja}"
BUILD_CMD="${BUILD_CMD:-ninja}"
BUILD_DIR_SUFFIX=""

if [[ `uname` == 'Linux' ]]; then
  TARGET_PLATFORM="${TARGET_PLATFORM:-linux}"
elif [[ `uname` == 'Darwin' ]]; then
  TARGET_PLATFORM="${TARGET_PLATFORM:-macosx}"
elif [[ `uname` == 'MSYS_NT-10.0' ]]; then
  # This is the only valid configuration on windows:
  BUILD_SYSTEM="Visual Studio 14 2015 Win64"
  BUILD_CMD=" devenv llvm_build/LLVM.sln /build"
  TARGET_PLATFORM="${TARGET_PLATFORM:-windows}"
else
  TARGET_PLATFORM="${TARGET_PLATFORM:-unknown}"
fi

if [ -n "$ENABLE_ASAN" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_asan";
fi
if [ -n "$DISTRIBUTE" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_release";
fi
if [ -n "$BUILD_32BIT" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_32";
fi

# If env DISTRIBUTE is set, we will use release build.
if [ -n "$DISTRIBUTE" ]; then
  BUILD_TYPE="${BUILD_TYPE:-MinSizeRel}"
else
  BUILD_TYPE="${BUILD_TYPE:-Debug}"
fi

LLVM_BUILD_DIR="${LLVM_BUILD_DIR:-llvm_build$BUILD_DIR_SUFFIX}"

echo "Using Build system: $BUILD_SYSTEM"
echo "Using Build command: $BUILD_CMD"
echo "Build Dir: $LLVM_BUILD_DIR"
echo "Build Type: $BUILD_TYPE"

if [ -n "$HTTP_PROXY" ]; then
  git config --global http.proxy $HTTP_PROXY
fi

if [ ! -e "./llvm/" ]; then
  # Clone the LLVM and Clang repos.
  # Retry 3 times if clone failed
  RETRY=3
  n=0
  until [ $n -ge $RETRY ]; do
    git clone https://github.com/llvm-mirror/llvm.git && break
    n=$[$n+1]
  done
  if [[ $n -ge $RETRY ]]; then
    exit 1
  fi
fi

(cd llvm; git checkout $LLVM_REV)

#local edits
#There are a small number of edits we need to make to the llvm files:
(cd llvm/include/llvm/ADT; patch < "$SCRIPT_DIR"/llvm-patches/StringExtras.h.diff)
(cd llvm/include/llvm/Support; patch < "$SCRIPT_DIR"/llvm-patches/raw_ostream.h.diff)
(cd llvm/lib/Support; patch <  "$SCRIPT_DIR"/llvm-patches/Signals.cpp.diff)

#build llvm
FLAGS="-DLLVM_TARGETS_TO_BUILD= -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ -n "$BUILD_32BIT" ]; then
  FLAGS="$FLAGS -DLLVM_BUILD_32_BITS=On"
fi
if [ -z "$DISTRIBUTE" ]
then
  FLAGS="$FLAGS -DLLVM_ENABLE_ASSERTIONS=On"
fi
if [ -n "$ENABLE_ASAN" ]
then
  FLAGS="$FLAGS -DLLVM_USE_SANITIZER=Address"
fi
# Disable host/target info in version prnter to reduce binary size.
FLAGS="$FLAGS -DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off"

# These versions need to match .buckconfig's [apple] section

if [[ "$TARGET_PLATFORM" == iphone* ]]; then
  FLAGS="$FLAGS \
         -DCMAKE_C_FLAGS=-miphoneos-version-min=8.0 \
         -DCMAKE_CXX_FLAGS=-miphoneos-version-min=8.0 \
         -DCMAKE_TOOLCHAIN_FILE=../llvm/cmake/platforms/iOS.cmake \
         -DLLVM_BUILD_RUNTIME=Off -DLLVM_INCLUDE_TESTS=Off \
         -DLLVM_INCLUDE_EXAMPLES=Off -DLLVM_ENABLE_BACKTRACES=Off \
         -DLLVM_INCLUDE_UTILS=Off -DLLVM_ENABLE_TERMINFO=Off"
fi

if [[ "$TARGET_PLATFORM" == iphoneos ]]; then
  FLAGS="$FLAGS -DCMAKE_OSX_ARCHITECTURES=armv7;arm64"
elif [[ "$TARGET_PLATFORM" == iphonesimulator ]]; then
  FLAGS="$FLAGS -DCMAKE_OSX_ARCHITECTURES=x86_64 \
         -DCMAKE_OSX_SYSROOT=`xcodebuild -version -sdk iphonesimulator Path`"
elif [[ "$TARGET_PLATFORM" == macosx ]]; then
  FLAGS="$FLAGS \
         -DCMAKE_C_FLAGS=-mmacosx-version-min=10.9 \
         -DCMAKE_CXX_FLAGS=-mmacosx-version-min=10.9"
fi

echo "cmake flags: $FLAGS"

mkdir -p "$LLVM_BUILD_DIR"
cd "$LLVM_BUILD_DIR"
# shellcheck disable=SC2086
cmake -G "$BUILD_SYSTEM" "$HERMES_WS_DIR/llvm" $FLAGS
$BUILD_CMD
