/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STATISTIC_H
#define HERMES_SUPPORT_STATISTIC_H

namespace hermes {
struct DummyCounter {
  DummyCounter &operator++(int) {
    return *this;
  }
  DummyCounter &operator++() {
    return *this;
  }
  DummyCounter &operator+=(int) {
    return *this;
  }
  DummyCounter &operator=(int) {
    return *this;
  }
};
} // namespace hermes

#if !defined(NDEBUG) || LLVM_FORCE_ENABLE_STATS
// Debug - forward to llvm.
#include "llvh/ADT/Statistic.h"

namespace hermes {
inline bool AreStatisticsEnabled() {
  return llvh::AreStatisticsEnabled();
}
inline void EnableStatistics() {
  llvh::EnableStatistics();
}
} // namespace hermes

#else
// Release - stub out.
#define STATISTIC(Name, Desc) static hermes::DummyCounter Name;
namespace hermes {
inline bool AreStatisticsEnabled() {
  return false;
}
inline void EnableStatistics() {}
} // namespace hermes
#endif

// HERMES_SLOW_STATISTIC are Statistics which are only enabled if
// HERMES_SLOW_DEBUG is defined or HERMES_FORCE_SLOW_STATS is defined.
#if defined(HERMES_SLOW_DEBUG) || defined(HERMES_FORCE_SLOW_STATS)
#define HERMES_SLOW_STATISTIC(Name, Desc) STATISTIC(Name, Desc)
#else
#define HERMES_SLOW_STATISTIC(Name, Desc) static hermes::DummyCounter Name;
#endif

#endif
