/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_TIMER_H
#define HERMES_SUPPORT_TIMER_H

#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
// Debug - forward to llvm.
#include "llvm/Support/Timer.h"

namespace hermes {
using Timer = llvm::Timer;
using TimerGroup = llvm::TimerGroup;
using NamedRegionTimer = llvm::NamedRegionTimer;
using TimeRegion = llvm::TimeRegion;
} // namespace hermes

#else
// Release - stub out.
#include "llvm/ADT/StringRef.h"

namespace hermes {
struct TimerGroup {
  TimerGroup(llvm::StringRef a, llvm::StringRef b) {}
};
struct Timer {
  Timer(llvm::StringRef a, llvm::StringRef b) {}
  Timer(const Timer &t) {}
  Timer(llvm::StringRef a, llvm::StringRef b, TimerGroup &tg) {}
};
struct NamedRegionTimer {
  NamedRegionTimer(
      llvm::StringRef a,
      llvm::StringRef b,
      llvm::StringRef c,
      llvm::StringRef d,
      bool Enabled = true) {}
};
struct TimeRegion {
  TimeRegion(Timer *t) {}
};
} // namespace hermes

#endif

#endif
