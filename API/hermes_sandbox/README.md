# Hermes Sandboxed Runtime implementation

This directory contains a sandboxed version of Hermes that is produced by compiling Hermes to wasm, and then compiling the resulting wasm to C using wasm2c. The sandboxed code exposes Hermes' C-API, and JSI is re-implemented on top of it.

The generated C is pre-generated and checked in to simplify the build. It will need to be periodically updated to incorporate bug fixes, and especially when there are changes to JSI or the bytecode version.

## Re-generating the sandbox

Install `emsdk` by following the instructions [here](https://github.com/emscripten-core/emsdk). Then [download wasm2c from wabt](https://github.com/WebAssembly/wabt/releases).

Build `hermesc` for the host platform, in order to generate bytecode that is used by the Hermes build. The commands below use `build_host` to refer to this host build.


Generate the debug build with the following command. Note that it is built with O2 and without slow asserts to avoid slowing down execution too dramatically, since the generated C code for the debug build is then typically built without optimisations.
```
cmake -S <path to hermes> -B build_wasm_dbg \
  -DCMAKE_TOOLCHAIN_FILE=<path to emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DIMPORT_HERMESC=build_host/ImportHermesc.cmake \
  -DCMAKE_BUILD_TYPE=Debug -DHERMES_UNICODE_LITE=ON \
  -DCMAKE_CXX_FLAGS=-O2  -DCMAKE_C_FLAGS=-O2 \
  -DCMAKE_EXE_LINKER_FLAGS="-sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB -sGLOBAL_BASE=16384" \
  -DHERMES_ENABLE_DEBUGGER=OFF -DHERMES_SLOW_DEBUG=OFF \
  -DHERMES_IS_MOBILE_BUILD=ON -G Ninja
```

Then generate the C artefact by running the following:
```
cmake --build build_wasm_dbg --target hermesSandboxImpl \
  && wasm2c build_wasm_dbg/API/hermes_sandbox/hermesSandboxImpl.wasm -n hermes --num-outputs 8 \
     -o <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_dbg_compiled.c
```

Likewise for the release build. Note that the release build is still built with `-g2` to preserve function names. Since the generated C is going to be compiled anyway, this doesn't have any effect on the final size of release builds.
```
cmake -S <path to hermes> -B build_wasm_opt \
  -DCMAKE_TOOLCHAIN_FILE=<path to emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DIMPORT_HERMESC=build_host/ImportHermesc.cmake \
  -DCMAKE_BUILD_TYPE=Release -DHERMES_UNICODE_LITE=ON \
  -DCMAKE_EXE_LINKER_FLAGS="-sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB -sGLOBAL_BASE=16384 -g2" \
  -DHERMES_ENABLE_DEBUGGER=OFF -DHERMES_IS_MOBILE_BUILD=ON -G Ninja

cmake --build build_wasm_opt --target hermesSandboxImpl \
  && wasm2c build_wasm_opt/API/hermes_sandbox/hermesSandboxImpl.wasm -n hermes --num-outputs 8 \
     -o <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_opt_compiled.c
```

The currently generated artefact was generated using **emsdk 3.1.39**, and **wasm2c 1.0.33**. Note that using a newer wasm2c will also require updating the checked in wasm-rt files in the `external` directory, which provide the runtime library for the generated code.
