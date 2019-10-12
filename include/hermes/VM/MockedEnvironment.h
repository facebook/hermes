/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_PUBLIC_MOCKEDENVIRONMENT_H
#define HERMES_PUBLIC_MOCKEDENVIRONMENT_H

#include <cstdint>
#include <deque>
#include <random>
#include <string>

namespace hermes {
namespace vm {

/// A MockedEnvironment is a group of results of calls to functions in JS that
/// have non-deterministic behavior. It also includes the seed that was used for
/// Math.random. This can be printed out to have a record of what occurred, or
/// passed into the Runtime so it returns the same sequence of values for the
/// specified calls.
struct MockedEnvironment final {
  std::minstd_rand::result_type mathRandomSeed{0};
  std::deque<uint64_t> callsToDateNow;
  std::deque<uint64_t> callsToNewDate;
  std::deque<std::string> callsToDateAsFunction;

  MockedEnvironment() = default;
  explicit MockedEnvironment(
      std::minstd_rand::result_type mathRandomSeed,
      const std::deque<uint64_t> &callsToDateNow,
      const std::deque<uint64_t> &callsToNewDate,
      const std::deque<std::string> &callsToDateAsFunction)
      : mathRandomSeed(mathRandomSeed),
        callsToDateNow(callsToDateNow),
        callsToNewDate(callsToNewDate),
        callsToDateAsFunction(callsToDateAsFunction) {}
};

} // namespace vm
} // namespace hermes

#endif
