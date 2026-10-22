# Building the wasm modules and the C

## Tools

Use **emsdk 3.1.39** and **wabt 1.0.33**, which provides `wasm2c` and
`wasm-objdump`. Do not upgrade them casually:

- Newer Emscripten no longer exports `stackSave`, `stackRestore` and
  `stackAlloc`, which the host calls, and its newer C library fails to compile
  the VM (`lseek64`).
- Newer wabt changes the `wasm-rt` runtime files in `external/`. The
  checked-in ones carry local changes, such as the randomized address hint for
  the linear memory in `wasm-rt-impl.c`, which an upgrade has to keep.

Build wabt from its `1.0.33` tag. Install emsdk with
`./emsdk install 3.1.39 && ./emsdk activate 3.1.39`, and pass its
`upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake` as the CMake
toolchain file.

Build from a copy of the sources on a local disk, so the builds are fast.

## setjmp and longjmp

Code compiled by shermes, and the JIT, use setjmp and longjmp for exceptions.
Emscripten implements them with JS imports (`env.invoke_*`,
`env._emscripten_throw_longjmp`), which a standalone module cannot have.

Configure both wasm builds with `-DHERMES_SANDBOX_WASM=ON`. It compiles them
out: `_sh_setjmp` returns 0 and `_sh_longjmp` traps
(`include/hermes/VM/static_h.h`), which is fine because neither shermes code
nor the JIT can run in the sandbox. As safeguards, `hermes_sandbox_impl.cpp`
does not compile without the option, and the link passes
`-sSUPPORT_LONGJMP=0 -fno-exceptions`, so anything that still needs them fails
to link instead of becoming an import.

## Builds

Follow `API/hermes_sandbox/README.md`: a host build of `hermesc` and
`shermes`, then the dbg and opt wasm builds with `-DIMPORT_HOST_COMPILERS`,
then `wasm2c -n hermes --num-outputs 8` for each, followed by
`API/hermes_sandbox/shorten_names.py` on its output. wasm2c turns C++ function
names into C identifiers, some of them several KB long, and MSVC rejects
identifiers that long. The script shortens every identifier longer than 2047
characters, deterministically. The flags that affect the output:

- `-DHERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32`: 32-bit heap values, as on 32-bit
  devices.
- `-sSTACK_SIZE=256KB`: see the stack budget in checks.md.
- `-sGLOBAL_BASE=16384`: keeps the lowest page unused, so the host can make it
  a guard page against null pointers.
- `-ffile-prefix-map=<path to hermes>/=xplat/static_h/`: keeps absolute paths
  out of the output.
- dbg is built with `-O2` and without the slow debug checks, because debug
  builds usually compile the generated C without optimizations. opt keeps
  `-g2`, so the C has readable function names.

`--num-outputs 8` keeps each file at about 12 MB. Raise it if the files grow
much larger.

## Reproducibility

Build both modules again in new, empty build directories with the same
commands, run wasm2c again, and compare every generated file with the first
run. They must be identical. The dbg wasm files themselves differ in their
debug information, which records the build directory, but that does not reach
the C.
