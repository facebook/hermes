/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MessageInterfaces.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {
namespace message {

std::string Serializable::toJsonStr() const {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  return jsonValToStr(toJsonVal(factory));
}

} // namespace message
} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
