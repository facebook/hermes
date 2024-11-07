/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "OptionHelpers.h"

#include <cmath>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

vm::CallResult<std::optional<std::u16string>> getStringOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    llvh::ArrayRef<const char16_t *> validValues,
    std::optional<std::u16string_view> defaultValue) {
  // 1. Let value be ? Get(options, property).
  auto iter = options.find(property);
  // 2. If value is undefined, then
  //   a. If default is required, throw a RangeError exception.
  //   b. Return default.
  if (iter == options.end()) {
    return std::optional<std::u16string>(defaultValue);
  }
  // 5. Else,
  //   a. Assert: type is string.
  //   b. Set value to ? ToString(value).
  const auto &optionValue = iter->second.getString();
  // 6. If values is not empty and values does not contain value,
  // throw a RangeError exception.
  if (!validValues.empty() &&
      llvh::find(validValues, optionValue) == validValues.end()) {
    return runtime.raiseRangeError(
        vm::TwineChar16(property.c_str()) +
        vm::TwineChar16(" value is invalid."));
  }
  // 7. Return value.
  return std::optional<std::u16string>(optionValue);
}

std::optional<bool> getBoolOption(
    const Options &options,
    const std::u16string &property,
    std::optional<bool> defaultValue) {
  // 1. Let value be ? Get(options, property).
  auto iter = options.find(property);
  // 2. If value is undefined, then
  //   a. If default is required, throw a RangeError exception.
  //   b. Return default.
  if (iter == options.end()) {
    return defaultValue;
  }
  // 3. If type is boolean, then
  //   a. Set value to ToBoolean(value).
  // 7. Return value.
  return iter->second.getBool();
}

// https://tc39.es/ecma402/#sec-getnumberoption
vm::CallResult<std::optional<double>> getNumberOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    double minimum,
    double maximum,
    std::optional<double> defaultValue) {
  auto iter = options.find(property);
  // https://402.ecma-international.org/8.0/#sec-defaultnumberoption
  // 1. If value is undefined, return fallback.
  if (iter == options.end()) {
    return defaultValue;
  }
  // 2. Set value to ? ToNumber(value).
  double optionValue = iter->second.getNumber();
  // 3. If value is NaN or less than minimum or greater than maximum,
  // throw a RangeError exception.
  if (std::isnan(optionValue) || optionValue < minimum ||
      optionValue > maximum) {
    return runtime.raiseRangeError(
        vm::TwineChar16(property.c_str()) +
        vm::TwineChar16(" value is invalid."));
  }
  // 4. Return floor(value).
  return std::optional<double>(std::floor(optionValue));
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
