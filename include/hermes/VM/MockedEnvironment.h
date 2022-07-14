/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PUBLIC_MOCKEDENVIRONMENT_H
#define HERMES_PUBLIC_MOCKEDENVIRONMENT_H

#include <llvh/ADT/StringMap.h>

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
  /// The tagged union for the values in a StatsTable (below).
  class StatsTableValue {
   public:
    // Need a default.
    StatsTableValue() : isNum_(true), numVal_(0.0) {}
    StatsTableValue(double val) : isNum_(true), numVal_(val) {}
    StatsTableValue(const std::string &val) : isNum_(false), strVal_(val) {}

    bool isNum() const {
      return isNum_;
    }

    double num() const {
      assert(isNum_);
      return numVal_;
    }

    std::string str() const {
      assert(!isNum_);
      return strVal_;
    }

   private:
    // Conceptually a union; can't actually be one because needs copy ctor.
    bool isNum_;
    double numVal_;
    std::string strVal_;
  };
  using StatsTable = llvh::StringMap<StatsTableValue>;

  std::deque<StatsTable> callsToHermesInternalGetInstrumentedStats;

  /// True if we should try to execute the same number of CPU instructions
  /// across repeated invocations of the same JS.
  bool stabilizeInstructionCount{false};

  MockedEnvironment() = default;
  explicit MockedEnvironment(
      const std::deque<StatsTable> &callsToHermesInternalGetInstrumentedStats)
      : callsToHermesInternalGetInstrumentedStats(
            callsToHermesInternalGetInstrumentedStats) {}
};

} // namespace vm
} // namespace hermes

#endif
