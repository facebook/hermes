/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INSTRUMENTATION_PERFEVENTS_H
#define HERMES_VM_INSTRUMENTATION_PERFEVENTS_H

#include <cstdint>
#include <string>

namespace hermes {
namespace vm {
namespace instrumentation {

/// Activates and logs various Linux performance counters.
/// Methods are NOT thread-safe. Counters are process-wide.
struct PerfEvents {
  /// Reset counters. Returns false on failure.
  static bool begin();
  /// Read the counters. jsonStats must contain a serialized JSON object that
  /// has a field named "totalTime". The counters are added before that field.
  /// If the counters can't be read, leave jsonStats unchanged and return false.
  static bool endAndInsertStats(std::string &jsonStats);
};

} // namespace instrumentation
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_PERFEVENTS_H
