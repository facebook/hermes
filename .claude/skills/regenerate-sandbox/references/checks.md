# Checking the output

## Before wasm2c

Run `wasm-objdump -x -j Import` and `wasm-objdump -x -j Export` on both
modules.

The imports must be exactly the functions that `HermesSandboxRuntime.cpp`
implements, which are marked with `/* import: ... */` comments near the end
of the file. Today there are 12:

- `env.emscripten_notify_memory_growth`;
- `hermes_import.getentropy`, `hermes_import.test_timeout` and
  `hermes_import.test_and_clear_timeout`;
- `wasi_snapshot_preview1.clock_time_get`, `environ_get`,
  `environ_sizes_get`, `fd_close`, `fd_fdstat_get`, `fd_seek`, `fd_write`
  and `proc_exit`.

The link uses `-sWARN_ON_UNDEFINED_SYMBOLS=0`, so a symbol missing from the
link becomes an import instead of an error. Any other import means something
is wrong; see troubleshooting.md.

The exports must include `memory`, `__indirect_function_table`,
`_initialize`, `get_global_base`, `get_hermes_abi_vtable`, `malloc`, `free`,
`realloc`, `stackSave`, `stackRestore` and `stackAlloc`.

## After wasm2c

Run `python3 .claude/skills/regenerate-sandbox/verify_generated.py <path>`,
where `<path>` is the directory that contains `API/`. For both dbg and opt it
checks:

- that the imports, and the exports the host calls, match what
  `HermesSandboxRuntime.cpp` uses;
- that the bytecode check compares against `BYTECODE_VERSION` in
  `include/hermes/BCGen/HBC/BytecodeVersion.h`;
- the stack size (pass `--stack-size` if `-sSTACK_SIZE` changed);
- that the data has no absolute paths, and the files have no conflict
  markers;
- that no identifier is longer than 2047 characters, so MSVC can compile
  the files;
- that the host mirror is a prefix of the vtable in `hermes_abi.h`.

It also prints the size of each file.

## Stack budget

The VM runs on the wasm stack between `__stack_end` and `__stack_base`. The
host rounds `__stack_end` up to a page boundary and makes the page above it a
guard page (`W2CHermesRAII`). The VM keeps `NativeStackGap` (64 KB) free for
native frames, and throws a RangeError when JS gets closer than that.

So JS gets about STACK_SIZE − (rounding + one page) − 64 KB: about 187 KB with
256 KB on 4 KB pages, and about 167 KB on 16 KB pages.

Do not raise `-sSTACK_SIZE` without a reason. The generated code runs with
wasm2c's stack depth counting off (`WASM_RT_USE_STACK_DEPTH_COUNT=0`), so a
deeper wasm stack also means deeper recursion on the host's C stack. Recursion
that the VM bounds only with a counter, such as the parser and JSON, can still
reach the guard page and crash the process.
