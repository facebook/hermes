/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_RUNTIME_H
#define HERMES_ABI_HERMES_ABI_RUNTIME_H

#include <hermes/Public/HermesExport.h>
#include <hermes/Public/RuntimeConfig.h>
#include <jsi/jsi.h>

namespace facebook::hermes {
/// Create a runtime based on Hermes' C ABI.
HERMES_EXPORT std::unique_ptr<facebook::jsi::Runtime> makeHermesABIRuntime(
    const ::hermes::vm::RuntimeConfig &runtimeConfig =
        ::hermes::vm::RuntimeConfig());
} // namespace facebook::hermes

#endif // HERMES_ABI_HERMES_ABI_RUNTIME_H
