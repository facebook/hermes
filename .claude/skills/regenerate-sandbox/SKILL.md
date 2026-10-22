---
name: regenerate-sandbox
description: >
  Guide for regenerating the Hermes sandbox, the C files in
  API/hermes_sandbox/external that wasm2c produces from a wasm build of the
  VM, and for changing HermesSandboxRuntime.cpp, the JSI runtime on top of it.
  Use when the user asks to regenerate or update the sandbox, re-run wasm2c or
  emscripten for it, or update the sandbox C files; after a bytecode version
  bump, a change to the C ABI in API/hermes_abi, or a VM fix the sandbox
  should get; or when adding or changing JSI methods in the sandbox runtime.
---

# Regenerating the Hermes sandbox

The sandbox runs the Hermes VM inside wasm, translated to C, so that bugs in
the VM cannot reach host memory. Its pieces:

- **The module.** `hermesSandboxImpl` (`API/hermes_sandbox/CMakeLists.txt`)
  links `hermes_sandbox_impl.cpp`, the C ABI (`API/hermes_abi`, mainly
  `hermes_vtable.cpp`) and the VM into a standalone wasm module with
  Emscripten. wasm2c turns it into `external/hermes_sandbox_impl_dbg_compiled*`
  and `external/hermes_sandbox_impl_opt_compiled*`. The `wasm-rt` files in
  `external/` are the wasm2c runtime.
- **The host.** `HermesSandboxRuntime.cpp` implements `jsi::Runtime` by calling
  the ABI vtable that the module exports (`get_hermes_abi_vtable`), through a
  hand-written mirror, `SANDBOX_CONTEXT_VTABLE_FUNCTIONS`. It also implements
  the module's imports.
- **The generated C is checked in.** Nothing rebuilds it, so a change to the
  VM, the ABI or the bytecode format reaches the sandbox only when someone
  regenerates it. The debug build of the host compiles the dbg files and the
  release build the opt files.

If this checkout has a `facebook/` directory, also read
`.claude/skills/facebook/regenerate-sandbox-meta/SKILL.md`, which covers the
internal build, test and submission steps.

## When to regenerate

- The bytecode version changes. The sandbox only accepts bytecode of the
  version it was built with.
- The C ABI changes: `hermes_abi.h` or `hermes_vtable.cpp`.
- A VM fix or feature should reach the sandbox.
- Emscripten or wasm2c is upgraded.

Changes to `HermesSandboxRuntime.cpp` alone do not need a regeneration.

## References

Read the one that matches the step you are on:

- Before changing the ABI or the mirror: [references/abi.md](references/abi.md)
- To set up the tools and build the wasm modules and the C:
  [references/building.md](references/building.md)
- To check the output before committing it:
  [references/checks.md](references/checks.md), which uses
  [verify_generated.py](verify_generated.py)
- To test the result: [references/testing.md](references/testing.md)
- When adding or changing a JSI method in `HermesSandboxRuntime.cpp`:
  [references/jsi.md](references/jsi.md)
- When something fails: [references/troubleshooting.md](references/troubleshooting.md)

## Checklist

1. Make the source changes: the VM, the ABI and the host runtime. The mirror
   changes go in the same commit as the regenerated C (see
   [references/abi.md](references/abi.md)).
2. Build the host compilers and both wasm modules with the commands in
   `API/hermes_sandbox/README.md`, with the pinned tool versions.
3. Check the imports and exports of both modules before running wasm2c.
4. Run wasm2c for dbg and opt, then run `verify_generated.py`.
5. Rebuild from clean build directories and check that the output is the same,
   byte for byte.
6. Run the API tests with the debug and the release build of the host, and
   with UBSan.
7. Update the README if the tools, flags or steps changed.
8. Afterwards, update this skill with anything that went differently.
