---
id: Emscripten
title: Building Hermes with Emscripten
---

# Building Hermes With Emscripten and CMake

    # Create a new build directory
    mkdir embuild && cd embuild
    # Configure the build
    cmake ${HermesSourcePath?} \
          -DCMAKE_TOOLCHAIN_FILE=${EmscriptenRoot?}/emscripten/cmake/Modules/Platform/Emscripten.cmake \
          -DCMAKE_BUILD_TYPE=MinSizeRel \
          -DEMSCRIPTEN_FASTCOMP=1 \
          -DCMAKE_EXE_LINKER_FLAGS="-s NODERAWFS=1 -s WASM=0 -s ALLOW_MEMORY_GROWTH=1"
    # Build Hermes
    make -j hermes
    # Execute hermes
    node bin/hermes.js --help
    
In the commands above, replace `${HermesSourcePath?}` with the path where you
cloned Hermes, and `${EmscriptenRoot?}` with the path to your Emscripten
install.

You can further customize the following settings:
- `CMAKE_BUILD_TYPE`: set it to one of CMake's build modes: `Debug`, `Release`, 
`MinSizeRel`, etc.
- `EMSCRIPTEN_FASTCOMP`: Set it to 1 or 0 depending on whether you are using
Emscripten with the *fastcomp* backend, or with the new LLVM backend.
- `-s NODERAWFS`: add this if you will be running Hermes directly with Node. It
enables direct access to the filesystem.
- `-s WASM=`: set this to 0 for *Asm.js* output, 1 for *Wasm*, and 2 for both.

You can customize the build generator by passing the `-G` option to CMake, for 
example `-G Ninja`.

