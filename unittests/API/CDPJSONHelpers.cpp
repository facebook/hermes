/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/inspector/chrome/tests/TestHelpers.h>

#include <hermes/inspector/chrome/MessageTypesInlines.h>

using namespace facebook::hermes::inspector_modern::chrome::message;
using namespace facebook::hermes::inspector_modern::chrome;
using namespace hermes::parser;

namespace facebook {
namespace hermes {

void ensureErrorResponse(const std::string &message, int id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<message::ErrorResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
}

void ensureOkResponse(const std::string &message, int id) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  auto response =
      mustMake<message::OkResponse>(mustParseStrAsJsonObj(message, factory));
  EXPECT_EQ(response.id, id);
}

template <typename T>
std::unique_ptr<T> getValue(JSONValue *value, std::vector<std::string> paths) {
  int numPaths = paths.size();
  for (int i = 0; i < numPaths; i++) {
    EXPECT_TRUE(JSONObject::classof(value));
    value = static_cast<JSONObject *>(value)->get(paths[i]);

    if (i != numPaths - 1) {
      EXPECT_TRUE(value != nullptr);
    } else {
      std::unique_ptr<T> target = valueFromJson<T>(value);
      return std::move(target);
    }
  }
  // Should never reach here
  EXPECT_TRUE(false);
  return nullptr;
}

void ensureResponse(
    const std::string &message,
    const std::string &expectedMethod,
    int expectedID) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);

  EXPECT_EQ(*getValue<std::string>(obj, {"method"}), expectedMethod);
  EXPECT_EQ(*getValue<double>(obj, {"id"}), expectedID);
}

void ensureNotification(
    const std::string &message,
    const std::string &expectedMethod) {
  JSLexer::Allocator allocator;
  JSONFactory factory(allocator);
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);

  EXPECT_EQ(*getValue<std::string>(obj, {"method"}), expectedMethod);
}

} // namespace hermes
} // namespace facebook
