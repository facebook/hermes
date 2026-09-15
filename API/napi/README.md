# Hermes Node-API

This is an implementation of [Node-API](https://nodejs.org/api/n-api.html)
(N-API) v10 for the Hermes JavaScript engine. It allows native addons written
against the Node-API ABI to run on Hermes without modification.

The implementation is built directly on Hermes VM internals (not JSI), covering
all non-experimental APIs through NAPI_VERSION 10. See
[COMPATIBILITY.md](COMPATIBILITY.md) for the full API support matrix.

## Architecture

`napi_value` is a pointer to a `PinnedHermesValue` slot in a handle scope
block pool. Handle scopes, references, and other NAPI-managed state live in the
`napi_env__` struct, which registers a custom GC roots function with the
Hermes Runtime.

Native addon callbacks are bridged through `NativeConstructor` trampolines.
Async work and thread-safe functions use a pluggable host integration interface
(`hermes_napi_host`) that the host application provides.

### Source Files

| File | Purpose |
|------|---------|
| `hermes_napi.h` | Core env struct, handle scopes, refs, macros |
| `hermes_napi_internal.h` | Internal helpers and trampoline declarations |
| `hermes_napi.cpp` | Env lifecycle, GC root marking, module loading |
| `hermes_napi_value.cpp` | Primitives, typeof, coercion, comparison |
| `hermes_napi_string.cpp` | String create/extract (UTF-8, UTF-16, Latin-1) |
| `hermes_napi_object.cpp` | Property operations, define_properties, freeze/seal |
| `hermes_napi_function.cpp` | Functions, call, new_instance, define_class |
| `hermes_napi_error.cpp` | Error creation and throwing |
| `hermes_napi_reference.cpp` | Strong/weak references |
| `hermes_napi_external.cpp` | External values (DecoratedObject) |
| `hermes_napi_wrap.cpp` | Object wrapping and finalizers |
| `hermes_napi_scope.cpp` | Handle scopes, instance data, cleanup hooks |
| `hermes_napi_arraybuffer.cpp` | ArrayBuffer create, external, detach |
| `hermes_napi_typedarray.cpp` | TypedArray (all 11 types) |
| `hermes_napi_dataview.cpp` | DataView |
| `hermes_napi_buffer.cpp` | Buffer (as Uint8Array) |
| `hermes_napi_promise.cpp` | Promise create/resolve/reject |
| `hermes_napi_bigint.cpp` | BigInt create/extract |
| `hermes_napi_typetag.cpp` | 128-bit type tags |
| `hermes_napi_tsfn.cpp` | Thread-safe functions |
| `hermes_napi_async_work.cpp` | Async work |
| `hermes_napi_async_context.cpp` | Async context and callback scopes |

## Building

```bash
# Build the NAPI library
cmake --build <build-dir> --target hermesNapi

# Build and run unit tests
cmake --build <build-dir> --target NapiTests && <build-dir>/unittests/napi/NapiTests

# Run all NAPI test suites (CTS + vendored Node.js tests)
cmake --build <build-dir> --target check-napi
```

## Testing

Tests are organized in three layers:

- **Unit tests** (`unittests/napi/`) — 31 GTest files testing individual APIs
  in isolation.
- **Node.js NAPI tests** (`external/node-api-tests/`) — ~65 test files
  vendored from the Node.js source tree. 33 pass, 31 skipped (due to Node.js
  module dependencies, experimental APIs, or Hermes-specific limitations).
  Zero failures.
- **node-api-cts** (`external/node-api-cts/`) — The official conformance test
  suite (currently a WIP upstream project with minimal coverage).

## Known Limitations

- **External strings** (`node_api_create_external_string_*`) always copy the
  data. Hermes does not support external string storage.
- **`napi_adjust_external_memory`** tracks the counter but does not influence
  GC scheduling.
- **`napi_get_uv_event_loop`** returns the host-provided `uv_loop` if set in
  `hermes_napi_host`, otherwise returns `napi_generic_failure`.
- **Weak reference shutdown ordering** — all weak references use a
  `WeakRefSlot` managed by the GC. Destroying refs during Runtime teardown
  can trigger assertions in debug builds because the ref lifetime outlives
  the Runtime. A few Node.js tests are skipped for this reason.
- **Experimental APIs** (`NAPI_EXPERIMENTAL`) are not included.
