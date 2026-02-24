---
id: emscripten
title: Building with Emscripten
---

# Compiling to Wasm

## Prerequisites

This guide assumes you are familiar with building Hermes locally and have the necessary
tools installed (e.g., CMake, Ninja, and a compatible toolchain). The instructions are
written for macOS but should work on Linux with minimal adjustments.

## Install Emscripten

Install `emsdk` by following the directions on the [Emscripten website for `emsdk`](https://emscripten.org/docs/getting_started/downloads.html).

Then, inside the installed directory, we need to make sure we have the latest toolchain:

```shell
./emsdk install latest
./emsdk activate latest
```

## Create a Parent Builds Directory

We need a directory to contain the different Hermes builds:

```shell
mkdir ~/hermes-builds
cd ~/hermes-builds
```

## Setup Environment Variables

For convenience, we rely on two environment variables throughout this document:
```shell
export Emsdk=<path-to-emsdk-directory>
export HermesSourcePath=<path-to-hermes-source-directory>
```

## Build Hermes for the Host

Hermes uses a two stage build process because parts of Hermes need to be
built using Hermes itself. For the first stage, we need to build Hermes for
the host. We will also use the host build later to compile .js to .c.

```shell
# From within the hermes-builds directory
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -S ${HermesSourcePath?} -B build-host
# Build hermes and shermes for the host
cmake --build build-host --target hermesc --target hermes --target shermes --parallel
```

## Compile the Hermes VM to Wasm

```shell
# From within the hermes-builds directory
cmake -G Ninja \
      -S ${HermesSourcePath?} -B build-wasm \
      -DCMAKE_TOOLCHAIN_FILE=${Emsdk?}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DIMPORT_HOST_COMPILERS=build-host/ImportHostCompilers.cmake \
      -DHAVE_SYS_IOCTL_H=0 \
      -DCMAKE_EXE_LINKER_FLAGS="-sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB"
# Build the VM and libraries
cmake --build build-wasm --target hermes --target shermes --target shermes-dep --parallel
```

Important options:
* `-G Ninja` is optional but recommended. It instructs CMake to use Ninja as a build tool.
* `CMAKE_BUILD_TYPE`: set it to one of CMake's build modes: `Debug`, `Release`,
  `MinSizeRel`, etc.
* `-DHAVE_SYS_IOCTL_H=0` is needed for compatibility with the Emscripten runtime environment.
* `NODERAWFS`: set to `1` if you will be running Hermes directly with Node. It
  enables direct access to the filesystem.
* `ALLOW_MEMORY_GROWTH`: whether to pre-allocate all memory, or let it grow over time

Under Emscripten, Hermes relies on a [small amount of JavaScript](../lib/Platform/Unicode/PlatformUnicodeEmscripten.cpp)
to be executed by the Wasm host (like Node.js or a browser). If you intend to run it under a "pure" Wasm host, consider
using this flag:

* `-DHERMES_UNICODE_LITE=` if set to ON, provides a minimal mostly stubbed-out Unicode implementation.

> Note that running under a "pure" Wasm host is not described here and will likely require more tweaks.

Now that the VM is compiled to Wasm, we can examine it:
```shell
ls -l build-wasm/bin/hermes.*
```

## Execute Some JavaScript with the Wasm Hermes VM

Let's create a small .js file:
```shell
echo 'var x = "hello"; console.log(`${x} world`);' > hello.js
```

Let's run the example with the Wasm VM:
```shell
node ./build-wasm/bin/hermes.js hello.js
```

Let's compile the example to bytecode and then run the bytecode:
```shell
node ./build-wasm/bin/hermes.js hello.js --emit-binary -out hello.hbc
node ./build-wasm/bin/hermes.js hello.hbc
```

Finally, let's compare the performance of the Wasm VM with the native one
by running one of the micro-benchmarks that come with Hermes.
```shell
# Run the micro-benchmark with the host VM.
./build-host/bin/hermes -w ${HermesSourcePath?}/benchmarks/bench-runner/resource/test-suites/micros/interp-dispatch.js
# Run it with the Wasm VM
node ./build-wasm/bin/hermes.js -w ${HermesSourcePath?}/benchmarks/bench-runner/resource/test-suites/micros/interp-dispatch.js
```

## Compile JavaScript to Wasm

Now, let's compile JavaScript to Wasm! This is still not directly supported by
the Hermes CLI tools, so we will need a manual step. But no worries, it is easy.

First, we must make sure we have activated the Emscripten SDK in the current shell.
```shell
emcc --help
```

If that runs fine, the SDK is already active. Otherwise, we need to activate it:
```shell
source ${Emsdk?}/emsdk_env.sh
```
> Note that we didn't need to activate it when we were building with CMake, because
> we used a CMake toolchain file.

Now we are ready to compile the example js file we created earlier directly to Wasm.
`wasm-compile.sh` is a helper script in the Hermes `utils/` directory.
```shell
${HermesSourcePath?}/utils/wasm-compile.sh build-host build-wasm hello.js
```

The compilation generates two files:
- `hello-wasm.wasm`: the compiled Wasm module. Note that this also contains
the entire JS library.
- `hello-wasm.js`: this is the Node.js wrapper to load the Wasm module.

We can run it:
```shell
node ./hello-wasm.js
```

Now let's try compiling the micro-benchmark to Wasm:
```shell
${HermesSourcePath?}/utils/wasm-compile.sh build-host build-wasm \
  ${HermesSourcePath?}/benchmarks/bench-runner/resource/test-suites/micros/interp-dispatch.js
```
