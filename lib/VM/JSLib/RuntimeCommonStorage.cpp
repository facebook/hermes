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

std::shared_ptr<RuntimeCommonStorage> createRuntimeCommonStorage(
    bool shouldTrace) {
  return std::make_shared<RuntimeCommonStorage>(shouldTrace);
}

RuntimeCommonStorage::RuntimeCommonStorage(bool shouldTrace)
    : shouldTrace(shouldTrace) {}
RuntimeCommonStorage::~RuntimeCommonStorage() {}

} // namespace vm
} // namespace hermes
