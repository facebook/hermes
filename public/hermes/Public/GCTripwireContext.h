/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PUBLIC_GCTRIPWIRECONTEXT_H
#define HERMES_PUBLIC_GCTRIPWIRECONTEXT_H

#include <string>

namespace hermes {
namespace vm {

/// Interface passed to the GC tripwire callback when it fires.
class GCTripwireContext {
 public:
  virtual ~GCTripwireContext() = default;

  /// Captures the heap to a file
  ///
  /// \param path to save the heap capture
  ///
  /// \return true iff the heap capture succeeded
  virtual bool createSnapshotToFile(const std::string &path) = 0;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_PUBLIC_GCTRIPWIRECONTEXT_H
