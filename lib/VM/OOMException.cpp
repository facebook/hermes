/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/OOMException.h"

namespace {
const char *kOOMWhat = "Hermes: Out of Memory";
} // namespace

namespace hermes {
namespace vm {

const char *OOMException::what() const noexcept {
  return kOOMWhat;
}

} // namespace vm
} // namespace hermes
