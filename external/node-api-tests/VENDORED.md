Vendored Node.js NAPI test sources for testing the Hermes NAPI implementation.

Source: Node.js v24.13.0 (commit def0bdf8)
Date vendored: 2026-02-10

## Contents

### js-native-api/
Engine-independent Node-API test suites (~35 suites). These test the core
js_native_api.h functions and should work with any compliant engine.

### node-api/
Selected Node.js-specific test suites that are relevant to non-Node
engines. Suites that depend on libuv, workers, or Node.js-specific
infrastructure were excluded.

#### Included node-api tests:
- 1_hello_world
- test_buffer
- test_exception
- test_general
- test_init_order
- test_instance_data
- test_null_init
- test_reference_by_node_api_version

#### Excluded node-api tests (Node.js-specific):
- test_async (libuv async work)
- test_async_cleanup_hook (Node.js cleanup hooks)
- test_async_context (Node.js async context)
- test_callback_scope (Node.js callback scope)
- test_cleanup_hook (Node.js cleanup hooks)
- test_env_teardown_gc (Node.js env teardown)
- test_fatal (Node.js fatal handling / crash tests)
- test_fatal_exception (Node.js fatal exception)
- test_make_callback (Node.js make_callback)
- test_make_callback_recurse (Node.js make_callback)
- test_threadsafe_function (libuv-based TSFN)
- test_uv_loop (libuv specific)
- test_uv_threadpool_size (libuv specific)
- test_worker_buffer_callback (Node.js workers)
- test_worker_terminate (Node.js workers)
- test_worker_terminate_finalization (Node.js workers)

## License

These files are from the Node.js project and are licensed under the
Node.js license (MIT). See LICENSE in this directory.
