/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPTESTHELPERS_H
#define HERMES_UNITTESTS_API_CDPTESTHELPERS_H

#include <gtest/gtest.h>

#include <hermes/Parser/JSONParser.h>
#include <hermes/cdp/JSONValueInterfaces.h>
#include <hermes/cdp/MessageTypes.h>
#include <hermes/cdp/MessageTypesInlines.h>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace ::hermes::parser;

inline JSONValue *mustParseStr(const std::string &str, JSONFactory &factory) {
  std::optional<JSONValue *> v = parseStr(str, factory);
  EXPECT_TRUE(v.has_value());
  return v.value();
}

inline JSONObject *mustParseStrAsJsonObj(
    const std::string &str,
    JSONFactory &factory) {
  std::optional<JSONObject *> obj = parseStrAsJsonObj(str, factory);
  EXPECT_TRUE(obj.has_value());
  return obj.value();
}

template <typename T>
T mustMake(const JSONObject *obj) {
  std::unique_ptr<T> instance = T::tryMake(obj);
  EXPECT_TRUE(instance != nullptr);
  return std::move(*instance);
}

template <typename T>
T clone(const T &t, JSONFactory &factory) {
  return mustMake<T>(mustParseStrAsJsonObj(t.toJsonStr(), factory));
}

namespace message {

inline std::unique_ptr<Request> mustGetRequestFromJson(const std::string &str) {
  std::unique_ptr<Request> req = Request::fromJson(str);
  EXPECT_TRUE(req != nullptr);
  return req;
}

} // namespace message

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_UNITTESTS_API_CDPTESTHELPERS_H
