---
id: cross-compilation
title: Cross Compilation
---

This document describes how to build Hermes in a cross compilation setting, e.g.
building for Android, WASM (with Emscripten), or any other platforms different
than the host development machines.

## Android compilation

Edit paths:
* `CMAKE_TOOLCHAIN_FILE` should point to whatever `android_ndk` you need.
* `IMPORT_HOST_COMPILERS` should point to whatever SH host build you want to use for InternalBytecode compilation.
* `-B` path is to your build directory, or omit it to build in current directory.

Ensure that `CMAKE_CXX_FLAGS_RELEASE` is `-O3 -DNDEBUG` if you're in Release mode.

Then you can use `adb push` to copy any files into `/data/local/tmp/` on an Android device and run it there using 'adb shell'.

```
cmake ~/fbsource/xplat/static_h \
  -B ~/static_h/build_android \
  -DHERMES_FACEBOOK_BUILD=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/opt/android_ndk/r17fb2/build/cmake/android.toolchain.cmake \
  -DHERMES_UNICODE_LITE=ON \
  -DIMPORT_HOST_COMPILERS=~/static_h/build_release/ImportHostCompilers.cmake \
  -DHERMES_ENABLE_DEBUGGER=OFF \
  -DANDROID_ABI=arm64-v8a -G Ninja
```

`HERMES_UNICODE_LITE=ON` selects the self-contained no-op Unicode backend.
Without it, an Android build uses the Java backend, which needs a JVM at
runtime and so is not usable from a binary pushed with `adb push` and run
under `adb shell`.

No ICU is needed here, or in any cross-compilation target. The
`platform_unicode` layer -- case conversion, normalization, collation and
date formatting -- is self-contained. ICU is used only by the `impl_icu`
Intl backend, which is off unless `-DHERMES_ENABLE_INTL=ON`, and which
Android does not use even then: it has its own `PlatformIntlAndroid`.

## Emscripten

For Emscripten, you can find an example from the `test-emscripten` job from `hermes/.circleci/config.yml`. Also see more details at [Building with Emscripten](./Emscripten.md)

## Compiling and Running ARM32 Static Hermes from ARM64

On the ARM64 Linux machine you need clang-16 and lld-16.

On the ARM64 Linux machine on which you're compiling:
```
# Add architecture
dpkg --add-architecture armhf
# Update
sudo apt update
# For compiling
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf binutils-arm-linux-gnueabihf libc6:armhf libstdc++6:armhf
```

If you're executing on an ARM64 machine, set it up as well:
```
# Add architecture
dpkg --add-architecture armhf
# Update
sudo apt update
# For running
sudo apt install libc6:armhf libstdc++6:armhf
```

There's a CMake toolchain file in the source directory that will then set up Clang to compile for ARM32. To use it:

```
CC=clang-16 CXX=clang++-16 cmake ${SRC_DIR?} -GNinja -DCMAKE_TOOLCHAIN_FILE=${SRC_DIR?}/armhf/arm-linux-gnueabihf.toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

Then you can run `ninja` and build 32-bit binaries and libraries. They'll work on the machine on which you're executing.
