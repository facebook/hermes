/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SNPRINTFBUF_H
#define HERMES_SUPPORT_SNPRINTFBUF_H

#include "hermes/Support/Compiler.h"

#include <cassert>
#include <memory>

namespace hermes {

/// Allocates a char array into which the printf method adds characters, via
/// snprintf.  The char array has an initial size, but will reallocate
/// as necessary to contain the written text.  The SNPrintfBuf has a
/// current index, initially at the start of the buffer.
class SNPrintfBuf {
 public:
  /// Allocate a new SNPrintfBuf, of the given \p initSize.  (This
  /// will default if not specified, but we allow specification to
  /// avoid reallocation costs.)
  SNPrintfBuf(int initSize = kInitSize);

  /// Adds the given \p fmt string, with the given arguments, to the
  /// buffer at the current index.  Advances the index
  /// by the size of the written characters.
  void printf(const char *fmt, ...) HERMES_ATTRIBUTE_FORMAT(printf, 2, 3);

  /// Yields the current buffer.  The lifetime of the returned pointer must
  /// not exceed the lifetime of \p this.
  const char *c_str() const {
    return buf_.get();
  }

 private:
  static const unsigned kInitSize = 100;

  /// Reallocate the buffer to a larger size, able to contain a string
  /// at least as large as minSize (which does not include the
  /// string's terminating null), copying the contents over to the new buffer.
  void realloc(int minSize);

  int curSize_;
  std::unique_ptr<char[]> buf_;
  char *bufPtr_;
  int remaining_;
};

} // namespace hermes

#endif
