/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROCESSSTATS_H
#define HERMES_VM_PROCESSSTATS_H

#include "hermes/VM/instrumentation/ApproxIntegral.h"

#include "llvh/Support/raw_ostream.h"

#include <chrono>

namespace hermes {
namespace vm {

/// Keeps track of process statistics, starting from when the instance was
/// constructed.  The graphs of these statistics are translated so that their
/// initial values are zero.  This ensures that statistics captured during the
/// same extent of different runs are comparable.  For each statistic, we
/// approximate the area under its graph over time in milliseconds.
class ProcessStats {
 public:
  using Clock = std::chrono::steady_clock;

  /// The information tracked by instances of this class.
  struct Info {
    /// The Resident-Set Size (Physical memory currently in-use by the process),
    /// in kilobytes.
    int64_t RSSkB;

    /// The Virtual Address Usage (Virtual memory reserved by the process), in
    /// kilobytes.
    int64_t VAkB;

    /// Output a JSON representation of the statistics to \p os.
    void printJSON(llvh::raw_ostream &os);
  };

  /// Create a new instance of this class.  Measurements are taken relative to
  /// the time of construction and values of metrics at that time.
  ProcessStats();

  /// \return the time we started collecting stats from.
  inline Clock::time_point initTime() const;

  /// Update the approximate integrals of the statistics by taking a snapshot of
  /// the statistics, and associating that with the timestamp, \p now.
  void sample(Clock::time_point now);

  /// \return the integrals of the statistics collected so far.
  Info getIntegratedInfo() const;

 private:
  /// The time at which the instance was created.
  const Clock::time_point initTime_;

  /// The snapshot of the statistics at the time the instance was created.
  const Info initInfo_;

  /// Accumulator for the approximate integral of Resident-Set Size (in
  /// kilobytes) over time in milliseconds.
  ApproxIntegral iRSSkBms_;

  /// Accumulator for the approximate integral of Virtual Address Usage (in
  /// kilobytes) over time in milliseconds.
  ApproxIntegral iVAkBms_;
};

ProcessStats::Clock::time_point ProcessStats::initTime() const {
  return initTime_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROCESSSTATS_H
