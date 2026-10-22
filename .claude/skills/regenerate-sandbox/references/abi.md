# Changing the C ABI and the host mirror

`HermesSandboxRuntime.cpp` does not include `hermes_abi.h`. It mirrors the
vtable by hand in `SANDBOX_CONTEXT_VTABLE_FUNCTIONS`, with the types each
function has in wasm32:

- Pointers and `size_t` are `u32`, and 64-bit integers are `u64`.
- A struct result wider than 4 bytes, such as `HermesABIValueOrError` or
  `HermesABISizeTOrError`, is returned through a hidden first parameter that
  points to the result, and the function returns `void`.
- A struct of 4 bytes or less, such as `HermesABIBoolOrError` or
  `HermesABIObjectOrError`, is returned as `u32`.

## The mirror binds by position

At startup, `getFunctionPtr` checks the wasm type of each vtable function, and
aborts on a mismatch. It cannot catch:

- two entries with the same wasm type that swapped places;
- a C type that changed but kept its wasm type. For example, a `bool` result
  that becomes `HermesABIBoolOrError` is still `i32`, so a host that was not
  updated reads every error as `true`.

## Rules

- Change `hermes_abi.h`, `hermes_vtable.cpp` (whose vtable initializer must
  follow the order of the header) and `HermesABIRuntimeWrapper.cpp`, the
  native JSI runtime on the same ABI, in one commit. That commit can come
  before the regenerated C, because the sandbox keeps running the ABI that its
  generated C was built from, but not after it.
- Change the mirror entry in `HermesSandboxRuntime.cpp`, and every place that
  decodes its result (the `Sandbox*OrError` types and the `unwrap`
  overloads), in the same commit as the regenerated C.
- Add new entries at the end of the vtable. The host treats its mirror as a
  prefix of the module's vtable, so the module may have more entries than the
  host uses, never fewer.
- `verify_generated.py` checks that the mirror is a prefix of the vtable in
  `hermes_abi.h`. Only running the API tests on the sandbox checks the rest,
  so always run them after an ABI change (see testing.md).

## ABI fixes need a regeneration

The generated C contains `hermes_vtable.cpp` as it was when the C was
generated. The sandbox exercises the ABI more than anything else does, so bugs
in `hermes_vtable.cpp` often show up there first. Fix them in
`hermes_vtable.cpp` and regenerate; the host cannot work around them.
