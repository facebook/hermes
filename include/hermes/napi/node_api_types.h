/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Vendored from Node.js v24 src/node_api_types.h with modifications:
// - Include guard renamed to avoid conflicts with Node.js headers

#ifndef HERMES_NAPI_NODE_API_TYPES_H_
#define HERMES_NAPI_NODE_API_TYPES_H_

#include "js_native_api_types.h"

typedef napi_value(NAPI_CDECL* napi_addon_register_func)(napi_env env,
                                                         napi_value exports);
typedef int32_t(NAPI_CDECL* node_api_addon_get_api_version_func)(void);

typedef struct napi_callback_scope__* napi_callback_scope;
typedef struct napi_async_context__* napi_async_context;
typedef struct napi_async_work__* napi_async_work;

#if NAPI_VERSION >= 3
typedef void(NAPI_CDECL* napi_cleanup_hook)(void* arg);
#endif  // NAPI_VERSION >= 3

#if NAPI_VERSION >= 4
typedef struct napi_threadsafe_function__* napi_threadsafe_function;
#endif  // NAPI_VERSION >= 4

#if NAPI_VERSION >= 4
typedef enum {
  napi_tsfn_release,
  napi_tsfn_abort
} napi_threadsafe_function_release_mode;

typedef enum {
  napi_tsfn_nonblocking,
  napi_tsfn_blocking
} napi_threadsafe_function_call_mode;
#endif  // NAPI_VERSION >= 4

typedef void(NAPI_CDECL* napi_async_execute_callback)(napi_env env,
                                                      void* data);
typedef void(NAPI_CDECL* napi_async_complete_callback)(napi_env env,
                                                       napi_status status,
                                                       void* data);
#if NAPI_VERSION >= 4
typedef void(NAPI_CDECL* napi_threadsafe_function_call_js)(
    napi_env env, napi_value js_callback, void* context, void* data);
#endif  // NAPI_VERSION >= 4

typedef struct {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
  const char* release;
} napi_node_version;

#if NAPI_VERSION >= 8
typedef struct napi_async_cleanup_hook_handle__*
    napi_async_cleanup_hook_handle;
typedef void(NAPI_CDECL* napi_async_cleanup_hook)(
    napi_async_cleanup_hook_handle handle, void* data);
#endif  // NAPI_VERSION >= 8

#endif  // HERMES_NAPI_NODE_API_TYPES_H_
