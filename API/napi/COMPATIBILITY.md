# Hermes Node-API Compatibility

This document describes the Node-API (N-API) compatibility status for the
Hermes JavaScript engine implementation.

## Supported Version

**NAPI_VERSION 10** — All non-experimental APIs through version 10 are
implemented.

## API Support Matrix

### Legend

| Symbol | Meaning |
|--------|---------|
| Yes | Fully implemented |
| Partial | Implemented with behavioral differences (see notes) |
| Stub | Declared but returns an error (not applicable to Hermes) |

### js-native-api (Engine-Only APIs)

#### Environment & Versioning

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_get_last_error_info` | Yes | |
| `napi_get_version` | Yes | Returns 10 |

#### Primitives & Values

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_get_undefined` | Yes | |
| `napi_get_null` | Yes | |
| `napi_get_global` | Yes | |
| `napi_get_boolean` | Yes | |
| `napi_create_object` | Yes | |
| `napi_create_array` | Yes | |
| `napi_create_array_with_length` | Yes | |
| `napi_create_double` | Yes | |
| `napi_create_int32` | Yes | |
| `napi_create_uint32` | Yes | |
| `napi_create_int64` | Yes | |
| `napi_typeof` | Yes | |
| `napi_is_array` | Yes | |
| `napi_get_array_length` | Yes | |

#### Number Extraction

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_get_value_double` | Yes | |
| `napi_get_value_int32` | Yes | |
| `napi_get_value_uint32` | Yes | |
| `napi_get_value_int64` | Yes | |
| `napi_get_value_bool` | Yes | |

#### Strings

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_string_utf8` | Yes | |
| `napi_create_string_utf16` | Yes | |
| `napi_create_string_latin1` | Yes | |
| `napi_get_value_string_utf8` | Yes | |
| `napi_get_value_string_utf16` | Yes | |
| `napi_get_value_string_latin1` | Yes | |
| `node_api_create_external_string_latin1` | Partial | Always copies; `copied` set to true (1) |
| `node_api_create_external_string_utf16` | Partial | Always copies; `copied` set to true (2) |
| `node_api_create_property_key_latin1` | Yes | |
| `node_api_create_property_key_utf8` | Yes | |
| `node_api_create_property_key_utf16` | Yes | |

#### Symbols

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_symbol` | Yes | |
| `node_api_symbol_for` | Yes | |

#### Functions & Constructors

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_function` | Yes | |
| `napi_call_function` | Yes | |
| `napi_new_instance` | Yes | |
| `napi_instanceof` | Yes | |
| `napi_get_cb_info` | Yes | |
| `napi_get_new_target` | Yes | |
| `napi_define_class` | Yes | |

#### Error Handling

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_throw` | Yes | |
| `napi_throw_error` | Yes | |
| `napi_throw_type_error` | Yes | |
| `napi_throw_range_error` | Yes | |
| `node_api_throw_syntax_error` | Yes | |
| `napi_create_error` | Yes | |
| `napi_create_type_error` | Yes | |
| `napi_create_range_error` | Yes | |
| `node_api_create_syntax_error` | Yes | |
| `napi_is_error` | Yes | |
| `napi_is_exception_pending` | Yes | |
| `napi_get_and_clear_last_exception` | Yes | |

#### Object Operations

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_set_property` | Yes | |
| `napi_get_property` | Yes | |
| `napi_has_property` | Yes | |
| `napi_delete_property` | Yes | |
| `napi_has_own_property` | Yes | |
| `napi_set_named_property` | Yes | |
| `napi_get_named_property` | Yes | |
| `napi_has_named_property` | Yes | |
| `napi_set_element` | Yes | |
| `napi_get_element` | Yes | |
| `napi_has_element` | Yes | |
| `napi_delete_element` | Yes | |
| `napi_define_properties` | Yes | |
| `napi_get_all_property_names` | Yes | |
| `napi_get_property_names` | Yes | |
| `napi_get_prototype` | Yes | |
| `napi_object_freeze` | Yes | |
| `napi_object_seal` | Yes | |

#### Type Coercion & Comparison

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_coerce_to_bool` | Yes | |
| `napi_coerce_to_number` | Yes | |
| `napi_coerce_to_object` | Yes | |
| `napi_coerce_to_string` | Yes | |
| `napi_strict_equals` | Yes | |

#### Object Wrapping & Externals

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_wrap` | Yes | (3) |
| `napi_unwrap` | Yes | |
| `napi_remove_wrap` | Yes | |
| `napi_create_external` | Yes | |
| `napi_get_value_external` | Yes | |
| `napi_add_finalizer` | Yes | |
| `napi_type_tag_object` | Yes | |
| `napi_check_object_type_tag` | Yes | |

#### References

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_reference` | Yes | |
| `napi_delete_reference` | Yes | |
| `napi_reference_ref` | Yes | |
| `napi_reference_unref` | Yes | |
| `napi_get_reference_value` | Yes | |

#### Handle Scopes

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_open_handle_scope` | Yes | |
| `napi_close_handle_scope` | Yes | |
| `napi_open_escapable_handle_scope` | Yes | |
| `napi_close_escapable_handle_scope` | Yes | |
| `napi_escape_handle` | Yes | |

#### ArrayBuffer & TypedArray

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_arraybuffer` | Yes | |
| `napi_create_external_arraybuffer` | Yes | |
| `napi_get_arraybuffer_info` | Yes | |
| `napi_is_arraybuffer` | Yes | |
| `napi_detach_arraybuffer` | Yes | |
| `napi_is_detached_arraybuffer` | Yes | (4) |
| `napi_create_typedarray` | Yes | |
| `napi_get_typedarray_info` | Yes | |
| `napi_is_typedarray` | Yes | |
| `napi_create_dataview` | Yes | |
| `napi_get_dataview_info` | Yes | |
| `napi_is_dataview` | Yes | |

#### Promise

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_promise` | Yes | (5) |
| `napi_resolve_deferred` | Yes | |
| `napi_reject_deferred` | Yes | |
| `napi_is_promise` | Yes | |

#### Script Execution

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_run_script` | Yes | |

#### Date

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_date` | Yes | |
| `napi_is_date` | Yes | |
| `napi_get_date_value` | Yes | |

#### BigInt

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_bigint_int64` | Yes | |
| `napi_create_bigint_uint64` | Yes | |
| `napi_create_bigint_words` | Yes | |
| `napi_get_value_bigint_int64` | Yes | |
| `napi_get_value_bigint_uint64` | Yes | |
| `napi_get_value_bigint_words` | Yes | |

#### Instance Data

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_set_instance_data` | Yes | |
| `napi_get_instance_data` | Yes | |

#### Memory

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_adjust_external_memory` | Partial | Tracks counter only; does not influence GC (6) |

### node-api (Node.js Runtime APIs)

#### Versioning

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_get_node_version` | Yes | Reports Hermes version |
| `node_api_get_module_file_name` | Yes | |

#### Buffer

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_buffer` | Yes | Backed by ArrayBuffer (7) |
| `napi_create_buffer_copy` | Yes | Backed by ArrayBuffer |
| `napi_create_external_buffer` | Yes | Backed by external ArrayBuffer |
| `node_api_create_buffer_from_arraybuffer` | Yes | |
| `napi_is_buffer` | Yes | |
| `napi_get_buffer_info` | Yes | |

#### Async Work

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_async_work` | Yes | Requires event loop (8) |
| `napi_delete_async_work` | Yes | |
| `napi_queue_async_work` | Yes | Requires event loop |
| `napi_cancel_async_work` | Yes | |

#### Async Context & Callbacks

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_async_init` | Yes | No-op context (9) |
| `napi_async_destroy` | Yes | |
| `napi_make_callback` | Yes | |
| `napi_open_callback_scope` | Yes | |
| `napi_close_callback_scope` | Yes | |

#### Error Handling

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_fatal_error` | Yes | Calls `abort()` |
| `napi_fatal_exception` | Yes | Delegates to host callback if provided, otherwise calls `abort()` |

#### Cleanup Hooks

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_add_env_cleanup_hook` | Yes | |
| `napi_remove_env_cleanup_hook` | Yes | |
| `napi_add_async_cleanup_hook` | Yes | Called synchronously (10) |
| `napi_remove_async_cleanup_hook` | Yes | |

#### Thread-Safe Functions

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_create_threadsafe_function` | Yes | Requires event loop (8) |
| `napi_get_threadsafe_function_context` | Yes | |
| `napi_call_threadsafe_function` | Yes | |
| `napi_acquire_threadsafe_function` | Yes | |
| `napi_release_threadsafe_function` | Yes | |
| `napi_ref_threadsafe_function` | Yes | |
| `napi_unref_threadsafe_function` | Yes | |

#### libuv

| Function | Supported | Notes |
|----------|-----------|-------|
| `napi_get_uv_event_loop` | Yes | Returns host-provided `uv_loop` if set in `hermes_napi_host`, otherwise returns `napi_generic_failure` (11) |

## Notes

1. **External strings (Latin-1)**: Hermes does not support external string
   storage. The string data is always copied into the Hermes heap. The
   `copied` parameter is set to `true` and the `finalize_cb` is called
   immediately to release the original data.

2. **External strings (UTF-16)**: Same behavior as Latin-1 external strings.

3. **Weak reference shutdown ordering**: All weak references (including
   those created by `napi_wrap`) use a `WeakRefSlot` managed by the GC.
   These slots are not fully cleaned up when the Runtime is destroyed,
   which can trigger a GC destructor assertion in debug builds. The issue
   is architectural: the module env lifetime outlives the Runtime.

4. **Detached ArrayBuffer detection**: A zero-length external ArrayBuffer
   created with `napi_create_external_arraybuffer(env, NULL, 0, ...)` is
   considered "attached" (valid but empty) by Hermes, whereas Node.js
   considers it "detached".

5. **Promise microtask queue**: Promise resolution callbacks (`.then()`,
   `.catch()`) require the microtask queue to be enabled via
   `RuntimeConfig::Builder().withMicrotaskQueue(true)`. The host must
   call `runtime.drainJobs()` to process queued microtasks.

6. **External memory**: `napi_adjust_external_memory` maintains a counter
   for reporting but does not influence Hermes GC scheduling. Hermes GC
   external memory APIs require a specific `GCCell`, which is not
   available from this global API.

7. **Buffer implementation**: Hermes does not have a native `Buffer` type.
   Buffers are implemented as `Uint8Array` views backed by `ArrayBuffer`,
   matching the Node.js `Buffer` semantics for binary data operations.

8. **Event loop requirement**: Async work and thread-safe functions require
   the host to provide callbacks via `hermes_napi_host` when
   creating the NAPI environment. Without the relevant callbacks,
   `napi_queue_async_work` and `napi_create_threadsafe_function` return
   `napi_generic_failure`.

9. **Async context**: Hermes does not implement async hooks or the
   `AsyncLocalStorage` API. `napi_async_init` / `napi_async_destroy` are
   no-ops. `napi_make_callback` functions as a regular function call.

10. **Async cleanup hooks**: Since Hermes has no event loop of its own,
    async cleanup hooks are invoked synchronously during environment
    teardown, unlike Node.js where they run asynchronously.

11. **libuv**: Hermes does not use libuv. `napi_get_uv_event_loop` returns
    the host-provided `uv_loop` if set in `hermes_napi_host`, otherwise
    returns `napi_generic_failure`.

## Excluded Experimental APIs

The following `NAPI_EXPERIMENTAL` APIs are not included in the vendored
headers and are not implemented:

- `napi_create_object_with_properties` / `napi_create_object_with_named_properties`
- `node_api_is_sharedarraybuffer`
- `node_api_post_finalizer`

## Test Results

Test suite results as of the latest comprehensive test pass:

| Suite | Pass | Fail | Skip | Total |
|-------|------|------|------|-------|
| Node.js NAPI tests | 33 | 0 | 31 | 64 |
| node-api-cts | 1 | 0 | 0 | 1 |

See `external/node-api-tests/hermes/hermes-skip-list.txt` for the
categorized list of skipped tests and their reasons.
