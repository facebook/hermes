# Hermes Sandboxed Runtime implementation

This directory contains a sandboxed version of Hermes that is produced by compiling Hermes to wasm, and then compiling the resulting wasm to C using wasm2c. The sandboxed code exposes Hermes' C-API, and JSI is re-implemented on top of it.

The generated C is pre-generated and checked in to simplify the build. It will need to be periodically updated to incorporate bug fixes, and especially when there are changes to JSI or the bytecode version.

## JSI support

`HermesSandboxRuntime.cpp` implements `jsi::Runtime` on top of the ABI. The JS running in the sandbox is untrusted, so the runtime's implementations of `jsi::Runtime` methods do not look up JS globals, and they check the type of every value they get from the sandbox. Many default implementations in `jsi.cpp` do neither, so the runtime overrides them. When the ABI has no entry for an operation, the runtime calls a built-in that it saved when it was created, before any JS ran: `eval` (only to create strings with unpaired surrogates), `JSON.parse`, `queueMicrotask`, `Object.create`, `Object.getPrototypeOf`, `Object.setPrototypeOf`, `Reflect.deleteProperty`, `Reflect.ownKeys`, the `Uint8Array` and error constructors, and the getters of `%TypedArray%.prototype` and `ArrayBuffer.prototype.detached`. Runtime data is stored in the runtime, and released before the VM.

JSI helpers that are not `jsi::Runtime` methods run on the host, and this runtime cannot override them. `jsi::Value::toString`, and building the message of a `jsi::JSError`, call the global `String`. Both check that it returns a string, but the JS in the sandbox can replace `String`, so do not trust the text they produce for values from the sandbox.

When JSI gains a method, check whether its default implementation looks up globals, or uses the unchecked `Value` accessors such as `getString` on what JS returns. If it does, override it here. Code that is not a `jsi::Runtime` method cannot be overridden, so fix it in JSI, for example by using the checked accessors such as `asString`.

These JSI features are not supported:
- `createArrayBuffer` and `ArrayBuffer::data` throw, since the sandbox cannot share memory with the host.
- `utf16`, `getStringData` and `getPropNameIdData` convert through UTF-8, so unpaired surrogates come back as U+FFFD.
- `instrumentation` is the JSI default, so heap information is empty and requests for garbage collection do nothing.
- `isInspectable` returns false, and `castInterface` returns null, so the Hermes interfaces such as `IHermes` and `ISerialization` are not available.

## Re-generating the sandbox

The `regenerate-sandbox` skill in `.claude/skills` has a checklist for this, the checks to run on the output, and solutions to problems seen in the past.

Install `emsdk` by following the instructions [here](https://github.com/emscripten-core/emsdk). Then [download wasm2c from wabt](https://github.com/WebAssembly/wabt/releases). The versions to use are listed at the end of this file.

Build `hermesc` for the host platform, in order to generate bytecode that is used by the Hermes build. The commands below use `build_host` to refer to this host build. Use a fresh directory, since the build appends to `ImportHostCompilers.cmake`.
```
cmake -S <path to hermes> -B build_host -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build_host --target hermesc shermes
```

The wasm builds must be configured with `-DHERMES_SANDBOX_WASM=ON`. Static Hermes uses setjmp/longjmp for exceptions in shermes-compiled code and the JIT, and Emscripten would implement them with JS imports that the sandbox runtime cannot provide. Since neither can run in the sandbox, this option compiles them out. The sandbox uses 32-bit heap values with boxed doubles (`HERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32`), which is how Hermes is configured on 32-bit devices. `-ffile-prefix-map=<path to hermes>/=xplat/static_h/` keeps absolute source paths out of the generated code, so the output does not depend on where the checkout lives.

Generate the debug build with the following command. Note that it is built with O2 and without slow asserts to avoid slowing down execution too dramatically, since the generated C code for the debug build is then typically built without optimisations.
```
cmake -S <path to hermes> -B build_wasm_dbg \
  -DCMAKE_TOOLCHAIN_FILE=<path to emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DIMPORT_HOST_COMPILERS=<absolute path to build_host>/ImportHostCompilers.cmake \
  -DHERMES_SANDBOX_WASM=ON -DHERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32 \
  -DCMAKE_BUILD_TYPE=Debug -DHERMES_UNICODE_LITE=ON \
  -DCMAKE_CXX_FLAGS="-O2 -ffile-prefix-map=<path to hermes>/=xplat/static_h/" \
  -DCMAKE_C_FLAGS="-O2 -ffile-prefix-map=<path to hermes>/=xplat/static_h/" \
  -DCMAKE_EXE_LINKER_FLAGS="-sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB -sGLOBAL_BASE=16384" \
  -DHERMES_ENABLE_DEBUGGER=OFF -DHERMES_SLOW_DEBUG=OFF \
  -DHERMES_IS_MOBILE_BUILD=ON -DHERMES_ENABLE_NAPI=OFF \
  -DHERMES_ENABLE_TEST_SUITE=OFF -G Ninja
```

Then build the wasm module, check its imports, and generate the C artefact:
```
cmake --build build_wasm_dbg --target hermesSandboxImpl
wasm-objdump -x -j Import build_wasm_dbg/API/hermes_sandbox/hermesSandboxImpl.wasm
wasm2c build_wasm_dbg/API/hermes_sandbox/hermesSandboxImpl.wasm -n hermes --num-outputs 8 \
  -o <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_dbg_compiled.c
python3 <path to hermes>/API/hermes_sandbox/shorten_names.py \
  <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_dbg_compiled*
```

The module must only import the functions that `HermesSandboxRuntime.cpp` implements: `env.emscripten_notify_memory_growth`, `hermes_import.getentropy`, `hermes_import.test_timeout`, `hermes_import.test_and_clear_timeout`, and the `wasi_snapshot_preview1` functions `clock_time_get`, `environ_get`, `environ_sizes_get`, `fd_close`, `fd_fdstat_get`, `fd_seek`, `fd_write` and `proc_exit`. Any other import usually means that a library is missing from the link, or that the VM picked up a dependency that the sandbox cannot provide.

wasm2c turns the C++ function names into C identifiers, and some template instantiations become identifiers several KB long, which MSVC cannot compile (fatal error C1064). `shorten_names.py` replaces every identifier longer than 2047 characters with a shorter one derived from a hash of the name, so the output is still reproducible.

Likewise for the release build. Note that the release build is still built with `-g2` to preserve function names. Since the generated C is going to be compiled anyway, this doesn't have any effect on the final size of release builds.
```
cmake -S <path to hermes> -B build_wasm_opt \
  -DCMAKE_TOOLCHAIN_FILE=<path to emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DIMPORT_HOST_COMPILERS=<absolute path to build_host>/ImportHostCompilers.cmake \
  -DHERMES_SANDBOX_WASM=ON -DHERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32 \
  -DCMAKE_BUILD_TYPE=Release -DHERMES_UNICODE_LITE=ON \
  -DCMAKE_CXX_FLAGS="-ffile-prefix-map=<path to hermes>/=xplat/static_h/" \
  -DCMAKE_C_FLAGS="-ffile-prefix-map=<path to hermes>/=xplat/static_h/" \
  -DCMAKE_EXE_LINKER_FLAGS="-sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB -sGLOBAL_BASE=16384 -g2" \
  -DHERMES_ENABLE_DEBUGGER=OFF -DHERMES_IS_MOBILE_BUILD=ON \
  -DHERMES_ENABLE_NAPI=OFF -DHERMES_ENABLE_TEST_SUITE=OFF -G Ninja

cmake --build build_wasm_opt --target hermesSandboxImpl
wasm-objdump -x -j Import build_wasm_opt/API/hermes_sandbox/hermesSandboxImpl.wasm
wasm2c build_wasm_opt/API/hermes_sandbox/hermesSandboxImpl.wasm -n hermes --num-outputs 8 \
  -o <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_opt_compiled.c
python3 <path to hermes>/API/hermes_sandbox/shorten_names.py \
  <path to hermes>/API/hermes_sandbox/external/hermes_sandbox_impl_opt_compiled*
```

The currently generated artefact was generated from the Static Hermes sources using **emsdk 3.1.39**, and **wasm2c 1.0.33**. Note that using a newer wasm2c will also require updating the checked in wasm-rt files in the `external` directory, which provide the runtime library for the generated code. Newer versions of emscripten also stop exporting `stackSave`, `stackRestore` and `stackAlloc`, which `HermesSandboxRuntime.cpp` relies on.
