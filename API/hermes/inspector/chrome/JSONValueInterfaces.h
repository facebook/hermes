/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <hermes/Parser/JSONParser.h>
#include <jsi/jsi.h>

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {
using namespace ::hermes::parser;

/// Convert a string to a JSONValue. Will throw if the parse is not completed.
JSONValue *parseStr(const std::string &str, JSONFactory &factory);

/// Convert a string to a JSON object. Will throw if the parse is not completed,
/// or the resulting JSON value is not an object.
JSONObject *parseStrAsJsonObj(const std::string &str, JSONFactory &factory);

/// Convert a JSONValue to a string.
std::string jsonValToStr(const JSONValue *v);

/// get(obj, key) is a wrapper for:
///   obj[key]
/// This function will throw if key is not found.
JSONValue *get(const JSONObject *obj, const std::string &key);

/// safeGet(obj, key) is a wrapper for:
///   obj[key]
/// This function will NOT throw if key is not found.
/// It will just return nullptr.
JSONValue *safeGet(const JSONObject *obj, const std::string &key);

/// Check if two JSONValues are equal.
bool jsonValsEQ(const JSONValue *A, const JSONValue *B);

}; // namespace chrome
}; // namespace inspector
}; // namespace hermes
}; // namespace facebook
