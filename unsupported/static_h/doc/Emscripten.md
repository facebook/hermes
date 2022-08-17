---
id: emscripten
title: Building with Emscripten
---

## Setting up Emscripten

To setup Emscripten for building Hermes, we recommend using `emsdk`, which is
the same way Emscripten recommends for most circumstances.
Follow the directions on the
[Emscripten website for `emsdk`](https://emscripten.org/docs/getting_started/downloads.html)
to download the SDK.

```
emsdk install latest
emsdk activate latest
source ./emsdk_env.sh
```

If you install `emsdk` at `~/emsdk` and activate `latest`,
then you should use this shell variable for the rest of these instructions:

```
$EmscriptenRoot = ~/emsdk/upstream/emscripten
```

If you are using the old `fastcomp` instead, replace `upstream` in the above instruction with `fastcomp`.

WARNING: The old `fastcomp` backend was [removed in emscripten `2.0.0` (August 2020)](https://emscripten.org/docs/compiling/WebAssembly.html?highlight=fastcomp#backends)


## Setting up Workspace and Host Hermesc

Hermes now requires a two stage build proecess because the VM now contains
Hermes bytecode which needs to be compiled by Hermes.

Please follow the [Cross Compilation](./CrossCompilation.md) to set up a workplace
and build a host hermesc at `$HERMES_WS_DIR/build_host_hermesc`.


# Building Hermes With Emscripten and CMake

    cmake -S ${HermesSourcePath?} -B build \
          -DCMAKE_TOOLCHAIN_FILE=${EmscriptenRoot?}/emscripten/cmake/Modules/Platform/Emscripten.cmake \
          -DCMAKE_BUILD_TYPE=MinSizeRel \
          -DEMSCRIPTEN_FASTCOMP=1 \
          -DCMAKE_EXE_LINKER_FLAGS="-s NODERAWFS=1 -s WASM=0 -s ALLOW_MEMORY_GROWTH=1"
    # Build Hermes
    cmake --build ./build --target hermes --parallel
    # Execute hermes
    node bin/hermes.js --help

In the commands above, replace `${HermesSourcePath?}` with the path where you
cloned Hermes, and `${EmscriptenRoot?}` with the path to your Emscripten
install.


Each option is explained below:
* `CMAKE_BUILD_TYPE`: set it to one of CMake's build modes: `Debug`, `Release`,
  `MinSizeRel`, etc.
* `EMSCRIPTEN_FASTCOMP`: set to `1` if using fastcomp, or `0` if using upstream
  (LLVM)
* `WASM`: whether to use asm.js (`0`), WebAssembly (`1`), or both (`2`)
* `NODERAWFS`: set to `1` if you will be running Hermes directly with Node. It
  enables direct access to the filesystem.
* `ALLOW_MEMORY_GROWTH`: whether to pre-allocate all memory, or let it grow over
  time

You can customize the build generator by passing the `-G` option to CMake, for
example `-G Ninja`.
