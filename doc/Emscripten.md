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

We recommend using `latest-fastcomp` to build Hermes, but `latest` works as well.
```
emsdk install latest-fastcomp
emsdk activate latest-fastcomp
```

If you install `emsdk` at `~/emsdk` and activate `latest-fastcomp`,
then you should use this shell variable for the rest of these instructions:
```
$EmscriptenRoot = ~/emsdk/fastcomp/emscripten
```

If you use `latest` instead, replace `fastcomp` in the above instruction with
`upstream`.

## Building Hermes With configure.py

```
# Configure the build. Here the build is output to a
# directory starting with the prefix "embuild".
python3 ${HermesSourcePath?}/utils/build/configure.py \
    --distribute \
    --wasm \
    --emscripten-platform=fastcomp \
    --emscripten-root="${EmscriptenRoot?}" \
    /tmp/embuild

# Build Hermes. The build directory name will depend on the flags passed to
# configure.py.
cmake --build /tmp/embuild_release_wasm_fastcomp --target hermes
# Execute hermes
node /tmp/embuild_release_wasm_fastcomp/bin/hermes.js --help
```

In the commands above, replace `${HermesSourcePath?}` with the path where you
cloned Hermes, and `${EmscriptenRoot?}` with the path to your Emscripten
install.

Make sure that the `--emscripten-platform` option matches the directory given
to `--emscripten-root`, and is also the current activated Emscripten toolchain
via `emsdk activate`.

See `configure.py --help` for more build options.

## Build with CMake directly

The `configure.py` script runs CMake for you with options chosen by the Hermes
project. If you want to customize your build, you can take this command as a
base.

```
mkdir embuild && cd embuild
cmake ${HermesSourcePath?} \
        -DCMAKE_TOOLCHAIN_FILE=${EmscriptenRoot?}/cmake/Modules/Platform/Emscripten.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DEMSCRIPTEN_FASTCOMP=1 \
        -DCMAKE_EXE_LINKER_FLAGS="-s NODERAWFS=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1"
```

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
