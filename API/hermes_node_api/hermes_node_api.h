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

#include "hermes/VM/Runtime.h"
#include "js_native_api.h"

namespace hermes::node_api {

// Get or create a Node API environment associated with the given Hermes
// runtime. The Node API environment is deleted by the runtime destructor.
vm::CallResult<napi_env> getOrCreateRuntimeNodeApiEnvironment(
    vm::Runtime &runtime,
    int32_t apiVersion) noexcept;

// Create new Node API environment for the given Hermes runtime.
// This environment is to be used for modules.
// The Node API environment is deleted by the runtime destructor.
vm::CallResult<napi_env> createModuleNodeApiEnvironment(
    vm::Runtime &runtime,
    int32_t apiVersion) noexcept;

} // namespace hermes::node_api

#endif // HERMES_NODE_API_H
