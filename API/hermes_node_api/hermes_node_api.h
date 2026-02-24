/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 */

#ifndef HERMES_NODE_API_H
#define HERMES_NODE_API_H

#include <memory>
#include <string>
#include "hermes/VM/RuntimeModule.h"
#include "node_api/node_api.h"

namespace hermes::node_api {

class NodeApiEnvironment;

// A task to execute by TaskRunner.
class Task {
 public:
  virtual ~Task() = default;
  virtual void invoke() noexcept = 0;
};

// The TaskRunner interface to schedule tasks in JavaScript thread.
class TaskRunner {
 public:
  virtual ~TaskRunner() = default;
  virtual void post(std::unique_ptr<Task> task) noexcept = 0;
};

// Get or create a Node API environment associated with the given Hermes
// runtime. The Node API environment is deleted by the runtime destructor.
vm::CallResult<napi_env> getOrCreateNodeApiEnvironment(
    vm::Runtime &runtime,
    hbc::CompileFlags compileFlags,
    std::shared_ptr<TaskRunner> taskRunner,
    const std::function<void(napi_env, napi_value)> &unhandledErrorCallback,
    int32_t apiVersion) noexcept;

// Initialize new Node API module in a new Node API environment.
napi_status initializeNodeApiModule(
    vm::Runtime &runtime,
    napi_addon_register_func registerModule,
    int32_t apiVersion,
    napi_value *exports) noexcept;

napi_status setNodeApiEnvironmentData(
    napi_env env,
    const napi_type_tag &tag,
    void *data) noexcept;

napi_status getNodeApiEnvironmentData(
    napi_env env,
    const napi_type_tag &tag,
    void **data) noexcept;

// TODO: can we remove it?
napi_status checkNodeApiPreconditions(napi_env env) noexcept;

// TODO: can we remove it?
napi_status setNodeApiValue(
    napi_env env,
    ::hermes::vm::CallResult<::hermes::vm::HermesValue> hvResult,
    napi_value *result);

// TODO: can we remove it?
napi_status checkJSErrorStatus(
    napi_env env,
    vm::ExecutionStatus hermesStatus) noexcept;

// TODO: remove it
napi_status queueMicrotask(napi_env env, napi_value callback) noexcept;

using nodeApiCallback = hermes::vm::CallResult<hermes::vm::HermesValue>(void *);

napi_status runInNodeApiContext(
    napi_env env,
    nodeApiCallback callback,
    void *data,
    napi_value *result) noexcept;

template <typename TCallback>
napi_status runInNodeApiContext(
    napi_env env,
    TCallback &&callback,
    napi_value *result) noexcept {
  return runInNodeApiContext(
      env,
      [](void *data) -> ::hermes::vm::CallResult<hermes::vm::HermesValue> {
        std::remove_reference_t<TCallback> *cb =
            reinterpret_cast<std::remove_reference_t<TCallback> *>(data);
        return (*cb)();
      },
      &callback,
      result);
}

// TODO: can we remove it?
template <class... TArgs>
napi_status setLastNativeError(
    napi_env env,
    napi_status status,
    const char *fileName,
    uint32_t line,
    TArgs &&...args) noexcept {
  std::ostringstream sb;
  (void)(sb << ... << args);
  const std::string message = sb.str();
  return setLastNativeError(env, status, fileName, line, message);
}

// TODO: can we remove it?
template <>
napi_status setLastNativeError(
    napi_env env,
    napi_status status,
    const char *fileName,
    uint32_t line,
    const std::string &message) noexcept;

// TODO: can we remove it?
napi_status clearLastNativeError(napi_env env) noexcept;

// TODO: can we replace it with something else?
napi_status openNodeApiScope(napi_env env, void **scope) noexcept;

// TODO: can we replace it with something else?
napi_status closeNodeApiScope(napi_env env, void *scope) noexcept;

} // namespace hermes::node_api

#endif // HERMES_NODE_API_H
