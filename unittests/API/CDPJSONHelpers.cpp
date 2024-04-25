/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/inspector/chrome/tests/TestHelpers.h>

using namespace hermes::parser;
using namespace facebook::hermes::inspector_modern::chrome;

namespace hermes {

void ensureErrorResponse(int id, const std::string &json) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<message::ErrorResponse>(mustParseStrAsJsonObj(json, factory));
  EXPECT_EQ(response.id, id);
}

} // namespace hermes
