/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/RuntimeCommonStorage.h"

#include <memory>

#include "hermes/VM/JSLib.h"

namespace hermes {
namespace vm {

std::unique_ptr<RuntimeCommonStorage> createRuntimeCommonStorage() {
  return std::make_unique<RuntimeCommonStorage>();
}

RuntimeCommonStorage::RuntimeCommonStorage() = default;
RuntimeCommonStorage::~RuntimeCommonStorage() = default;

} // namespace vm
} // namespace hermes
