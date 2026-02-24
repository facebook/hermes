/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SH_LINEDIREMITTER_H
#define HERMES_BCGEN_SH_LINEDIREMITTER_H

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/SourceErrorManager.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes::sh {

/// Wrapper around raw_ostream for emitting C line directives. These directives
/// will be inserted before any character that is directly following a newline.
/// The contents of the line directive are populated via a given SMLoc.
class LineDirectiveEmitter : public llvh::raw_ostream {
  /// Type for line number, filename
  using LineDirectiveInfo = std::pair<uint32_t, llvh::StringRef>;
  /// The actual stream that will be doing the real work outputting the data
  /// somewhere.
  llvh::raw_ostream &os_;
  /// The current source information for generating a #line directive.
  LineDirectiveInfo info_{};
  /// Set if line directives are enabled. This is expected to be toggled
  /// throughout the lifetime of the emitter, as some sections of C should not
  /// be annotated with a directive.
  bool enabled_ = false;
  /// Set if the last call to write_impl ended on a newline.
  bool endedOnNewline_ = false;

  /// This is called when the buffer is flushed.
  void write_impl(const char *ptr, size_t size) override;

  /// Emit a line directive to os_. This assumes a newline has just been
  /// printed.
  void writeLineDirective();

  /// We must override this, but this function is unused in our use case.
  uint64_t current_pos() const override {
    hermes_fatal("unimplemented");
  }

 public:
  explicit LineDirectiveEmitter(llvh::raw_ostream &o) : os_(o) {}
  ~LineDirectiveEmitter() override {
    flush();
  };

  /// Update the current location information for the line directive statement.
  /// If \p loc is invalid, or the same as the previous location, no update
  /// occurs.
  void setDirectiveInfo(SMLoc loc, SourceErrorManager &srcMgr);

  /// Enable emitting line directives
  void enableLineDirectives();

  /// Disable emitting line directives
  void disableLineDirectives();
};

} // namespace hermes::sh

#endif
