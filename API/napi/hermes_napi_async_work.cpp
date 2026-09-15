/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_impl.h"

//===========================================================================
// napi_async_work__ definition
//===========================================================================

/// An async work item. Created by napi_create_async_work, queued via
/// napi_queue_async_work, and deleted by napi_delete_async_work.
///
/// The execute callback runs on a worker thread and must not call any
/// NAPI functions that access JavaScript values. The complete callback
/// runs on the main thread and can safely access JS.
///
/// The host integration (hermes_napi_host) is responsible for
/// scheduling the actual threading. This struct just holds the
/// callbacks and data needed to bridge the NAPI API to that interface.
struct napi_async_work__ {
  napi_env env;
  napi_async_execute_callback execute;
  napi_async_complete_callback complete;
  void *data;
};

//===========================================================================
// Trampolines for hermes_napi_host
//===========================================================================

/// Called by the event loop on a worker thread.
static void asyncWorkExecuteTrampoline(void *work_data) {
  auto *work = static_cast<napi_async_work__ *>(work_data);
  if (work->execute) {
    work->execute(work->env, work->data);
  }
}

/// Called by the event loop on the main thread after execute completes
/// (or after cancellation).
static void asyncWorkCompleteTrampoline(void *work_data, napi_status status) {
  auto *work = static_cast<napi_async_work__ *>(work_data);
  if (work->complete) {
    napi_env env = work->env;

    // Open a handle scope for the complete callback, matching the
    // V8 reference implementation's HandleScope in AfterThreadPoolWork.
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);

    work->complete(env, status, work->data);

    napi_close_handle_scope(env, scope);
  }
}

//===========================================================================
// NAPI async work API
//===========================================================================

napi_status NAPI_CDECL napi_create_async_work(
    napi_env env,
    napi_value async_resource,
    napi_value async_resource_name,
    napi_async_execute_callback execute,
    napi_async_complete_callback complete,
    void *data,
    napi_async_work *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, execute);
  CHECK_ARG(env, result);

  // async_resource and async_resource_name are for Node.js async hooks
  // tracing. Hermes does not have async hooks, so we accept and ignore
  // these parameters.
  (void)async_resource;
  (void)async_resource_name;

  auto *work = new napi_async_work__{env, execute, complete, data};
  *result = work;
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL
napi_delete_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  delete work;
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL
napi_queue_async_work(node_api_basic_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  // A host must be provided for async work to function.
  RETURN_STATUS_IF_FALSE(env, env->host_ != nullptr, napi_generic_failure);

  env->host_->post_work(
      env->host_->data,
      work,
      asyncWorkExecuteTrampoline,
      asyncWorkCompleteTrampoline);
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL
napi_cancel_async_work(node_api_basic_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  // A host must be provided for async work to function.
  RETURN_STATUS_IF_FALSE(env, env->host_ != nullptr, napi_generic_failure);

  bool cancelled = env->host_->cancel_work(env->host_->data, work);
  // If cancellation could not be initiated (work already executing or
  // completed), return napi_generic_failure following Node.js behavior.
  RETURN_STATUS_IF_FALSE(env, cancelled, napi_generic_failure);
  return napi_clear_last_error(env);
}
