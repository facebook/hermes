# Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| The module imports `env.invoke_*` or `env._emscripten_throw_longjmp`, or exports `setThrew` | The wasm build was configured without `-DHERMES_SANDBOX_WASM=ON` | Configure with it; see building.md |
| The wasm link fails on setjmp, longjmp or exceptions | New code in the sandbox needs them, and the `-sSUPPORT_LONGJMP=0 -fno-exceptions` safeguard caught it | Keep that code out of the sandbox build, or compile it out under `HERMES_SANDBOX_WASM` |
| The module imports other `env.*` functions, named after VM or support library functions | A library is missing from the link, and `-sWARN_ON_UNDEFINED_SYMBOLS=0` turned its symbols into imports | Link it into `hermesabi`, which links `hermesvm_a` |
| Compile errors in the C library headers, such as `lseek64` | A newer Emscripten than 3.1.39 | Use the pinned version |
| MSVC fails on the generated C with "fatal error C1064: compiler limit: token overflowed internal buffer" | wasm2c turned a long C++ template name into an identifier of several KB | Run `API/hermes_sandbox/shorten_names.py` on the generated files, as the README does after wasm2c |
| The host fails to link because `w2c_hermes_stackSave`, `stackRestore` or `stackAlloc` is missing | A newer Emscripten stopped exporting them | Use the pinned version |
| The process aborts while creating the runtime, in `getFunctionPtr` | The host mirror and the module's vtable disagree on a function type | See abi.md |
| Wrong results without an abort after an ABI change | Entries changed places, or a type changed without changing its wasm type | See abi.md |
| Two clean rebuilds produce different C | Absolute paths reached the output, or the tool versions differ | Check `-ffile-prefix-map` and the versions; `verify_generated.py` reports absolute paths |
| A check that the setjmp safeguard fails the link passes anyway | `wasm-ld` dropped the unreferenced code before resolving it | Export the function that uses setjmp from the test module, so it is kept |
| UBSan reports "call to function ... through pointer to incorrect function type" in the generated C | Clang 17 and later check indirect calls in C too | Build the generated files with `-fno-sanitize=function` |
| An assertion in `hermes_vtable.cpp` that only the sandbox hits | A bug in the ABI, which the sandbox exercises most | Fix `hermes_vtable.cpp` and regenerate |
| The process crashes on deeply nested source code or JSON | The wasm stack reached the guard page | A known limitation; see the stack budget in checks.md |
