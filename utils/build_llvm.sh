#!/usr/bin/env bash

set -x
set -e

function finish () {
  local ret=$?
  echo "Exiting with $ret after $SECONDS seconds"
  return $ret
}

trap finish EXIT

# Detect the Hermes source dir
HERMES_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; echo "$PWD")
[ ! -e "$HERMES_DIR/utils/build_llvm.sh" ] && echo "Could not detect source dir" >&2 && exit 1

# shellcheck source=xplat/hermes/utils/commons.sh
source "$HERMES_DIR/utils/commons.sh"

# HERMES_WS_DIR is the root directory for LLVM checkout and build dirs.
[ -z "$HERMES_WS_DIR" ] && echo "HERMES_WS_DIR must be set" >&2 && exit 1
[ "${HERMES_WS_DIR:0:1}" != "/" ] && echo "HERMES_WS_DIR must be an absolute path" >&2 && exit 1

mkdir -p "$HERMES_WS_DIR"
cd "$HERMES_WS_DIR"

LLVM_REV=c179d7b006348005d2da228aed4c3c251590baa3

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
  # If release build is also intended, run the above command with
  # the additional flag: -property:Configuration=MinSizeRel
else
  TARGET_PLATFORM="${TARGET_PLATFORM:-unknown}"
fi

BUILD_DIR_SUFFIX=""
if [ -n "$ENABLE_ASAN" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_asan";
fi
# For multi config generators, user of this script should avoid setting
# DISTRIBUTE because it adds confusion and does not do anything meaningful.
# In CMake terminology, a multi config generator is one that generates all
# configurations (Debug, MinSizeRel, Release, RelWithDebInfo) in
# a single invocation of CMake. For example, MSVC and XCode are multi
# config generator. You choose which build configuration you want when
# you invoke the respective build tools, after CMake finishes.
# * Setting this env variable adds the _release suffix to directory name.
#   However, with a multi config generator, both release and debug builds
#   are generated in one shot.
# * Setting this env variable also changes BUILD_TYPE to MinSizeRel,
#   which is ignored by multi config generators because they generate all
#   configurations.
if [ -n "$DISTRIBUTE" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_release";
fi
if [ -n "$BUILD_32BIT" ]; then
    BUILD_DIR_SUFFIX="${BUILD_DIR_SUFFIX}_32";
fi

# If env DISTRIBUTE is set, we will use release build.
# For multi config generators (see comments above), BUILD_TYPE is meaningless
if [ -n "$DISTRIBUTE" ]; then
  BUILD_TYPE="${BUILD_TYPE:-MinSizeRel}"
else
  BUILD_TYPE="${BUILD_TYPE:-Debug}"
fi

# If CROSSCOMPILE_ONLY is set, we'll only build tblgen since
# that's the only thing we need to cross-compile LLVM for Android.
if [ -n "$CROSSCOMPILE_ONLY" ]
then
  BUILD_CMD="$BUILD_CMD bin/llvm-tblgen"
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
if ! (cd llvm; $GIT diff-index --quiet HEAD); then
  echo "llvm dir is dirty (contains uncommitted changes)" >&2
  exit 1
fi
# Use `git apply` instead of `patch` because `patch` may not be available
# on some Windows installations.
(cd llvm; $GIT apply "$HERMES_DIR"/utils/llvm-changes-for-hermes.patch)
# Commit the patches we applied. Since we're in detached HEAD mode,
# committing ensures that the patch operations above are idempotent
(cd llvm; $GIT -c user.name='nobody' -c user.email='nobody@example.com' commit -a -m "Patch by Hermes build script")

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
