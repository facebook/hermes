#!/usr/bin/env bash

set -x
set -e

START_DIR="$PWD"
function finish () {
  cd "$START_DIR"
}

trap finish EXIT

# Detect the Hermes source dir
HERMES_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; echo "$PWD")
[ ! -e "$HERMES_DIR/utils/build_llvm.sh" ] && echo "Could not detect source dir" >&2 && exit 1

# shellcheck source=xplat/hermes/utils/commons.sh
source "$HERMES_DIR/utils/commons.sh"

# define a helper retry function
function retry() {
  RETRY=$1
  ATTEMPTS=0
  while (( ATTEMPTS <= RETRY )); do
    "${@:2}" && return 0
    ATTEMPTS=$((ATTEMPTS+1))
    echo "Retry: $ATTEMPTS failures so far"
  done
  echo "Retry depleted for command: ${*:2}"
  return 1
}

# HERMES_WS_DIR is the root directory for LLVM checkout and build dirs.
[ -z "$HERMES_WS_DIR" ] && echo "HERMES_WS_DIR must be set" >&2 && exit 1
[ "${HERMES_WS_DIR:0:1}" != "/" ] && echo "HERMES_WS_DIR must be an absolute path" >&2 && exit 1

mkdir -p "$HERMES_WS_DIR"
cd "$HERMES_WS_DIR"

LLVM_REV=4519ac3791135eb9c207f0684f4236dbc13ac83f

if [[ "$PLATFORM" == 'linux' ]]; then
  TARGET_PLATFORM="${TARGET_PLATFORM:-linux}"
  BUILD_SYSTEM="${BUILD_SYSTEM:-Ninja}"
  BUILD_CMD="${BUILD_CMD:-ninja}"
elif [[ "$PLATFORM" == 'macosx' ]]; then
  TARGET_PLATFORM="${TARGET_PLATFORM:-macosx}"
  BUILD_SYSTEM="${BUILD_SYSTEM:-Ninja}"
  BUILD_CMD="${BUILD_CMD:-ninja}"
elif [[ "$PLATFORM" == 'windows' ]]; then
  TARGET_PLATFORM="${TARGET_PLATFORM:-windows}"
  BUILD_SYSTEM="${BUILD_SYSTEM:-Visual Studio 15 2017 Win64}"
  # Retry 3 times if build fails.
  # This mitigates the issue that LLVM build with MSBuild fails intermittently
  # with "LINK : fatal error LNK1000: unknown error".
  # Build is incremental. As a result, retry should be fast.
  BUILD_CMD="${BUILD_CMD:-retry 3 MSBuild.exe LLVM.sln -target:build -maxcpucount -verbosity:normal}"
else
  TARGET_PLATFORM="${TARGET_PLATFORM:-unknown}"
fi

BUILD_DIR_SUFFIX=""
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

GIT="git"
if [ -n "$HTTP_PROXY" ]; then
  GIT+=" -c http.proxy=$HTTP_PROXY"
fi

if [[ "$PLATFORM" == 'windows' ]]; then
  GIT+=" -c core.filemode=false"
  GIT+=" -c core.autocrlf=false"
fi

if [ ! -e "./llvm/" ]; then
  # Clone the LLVM and Clang repos.
  # Retry 3 times if clone failed
  # shellcheck disable=SC2086
  retry 3 $GIT clone https://github.com/llvm-mirror/llvm.git
fi

(cd llvm; $GIT checkout $LLVM_REV)

# local edits
# There are a small number of edits we need to make to the llvm files.
# Use `git apply` instead of `patch` because `patch` may not be available
# on some Windows installations.
(cd llvm/include/llvm/ADT; $GIT apply "$HERMES_DIR"/utils/llvm-patches/StringExtras.h.patch)
(cd llvm/include/llvm/Support; $GIT apply "$HERMES_DIR"/utils/llvm-patches/raw_ostream.h.patch)
(cd llvm/lib/Support; $GIT apply "$HERMES_DIR"/utils/llvm-patches/Signals.cpp.patch)
(cd llvm/lib/Support; $GIT apply "$HERMES_DIR"/utils/llvm-patches/Host.cpp.patch)

#build llvm
FLAGS="-DLLVM_TARGETS_TO_BUILD= -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ -n "$BUILD_32BIT" ]; then
  FLAGS="$FLAGS -DLLVM_BUILD_32_BITS=On"
fi
if [[ "$PLATFORM" == 'windows' && "$(uname -m)" == 'x86_64' ]]; then
  # Visual Studio generators use the x86 host compiler by default, even for
  # 64-bit targets. This default setup leads to link failures for LLVM build.
  FLAGS="$FLAGS -Thost=x64"
fi
if [[ "$PLATFORM" == 'windows' ]]; then
  # The examples look for LLVMX86CodeGen.lib on Windows, which we don't have
  # because LLVM_TARGETS_TO_BUILD is set to empty.
  FLAGS="$FLAGS -DLLVM_INCLUDE_EXAMPLES=Off"
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
cmake -G "$BUILD_SYSTEM" "$(platform_path "$HERMES_WS_DIR")/llvm" $FLAGS
$BUILD_CMD
