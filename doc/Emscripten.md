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


## Building Hermes With configure.py

```
# Configure the build. Here the build is output to a
# directory starting with the prefix "embuild".
python3 ${HERMES_WS_DIR}/hermes/utils/build/configure.py \
    --cmake-flags " -DIMPORT_HERMESC:PATH=${HERMES_WS_DIR}/build_host_hermesc/ImportHermesc.cmake " \
    --distribute \
    --wasm \
    --emscripten-platform=upstream \
    --emscripten-root="${EmscriptenRoot?}" \
    /tmp/embuild

# Build Hermes. The build directory name will depend on the flags passed to
# configure.py.
cmake --build /tmp/embuild --target hermes
# Execute hermes
node /tmp/embuild/bin/hermes.js --help
```

Make sure that the `--emscripten-platform` option matches the directory given
to `--emscripten-root`, and is also the current activated Emscripten toolchain
via `emsdk activate`.

See `configure.py --help` for more build options.

## Build with CMake directly

The `configure.py` script runs CMake for you with options chosen by the Hermes
project. If you want to customize your build, you can take this command as a
base.

```
cmake ${HERMES_WS_DIR}/hermes \
        -B embuild \
        -DCMAKE_TOOLCHAIN_FILE=${EmscriptenRoot?}/cmake/Modules/Platform/Emscripten.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-s NODERAWFS=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1" \
        -DIMPORT_HERMESC:PATH="${HERMES_WS_DIR}/build_host_hermesc/ImportHermesc.cmake"
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
