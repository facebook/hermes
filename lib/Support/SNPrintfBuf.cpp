/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SNPrintfBuf.h"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace hermes {

SNPrintfBuf::SNPrintfBuf(int initSize)
    : curSize_(initSize),
      buf_(new char[initSize]),
      bufPtr_(&buf_[0]),
      remaining_(initSize) {}

void SNPrintfBuf::printf(const char *fmt, ...) {
  assert(buf_.get() && "Buffer has been released");
  va_list vl;
  va_start(vl, fmt);
  // If the first vsnprintf fails, we need a second copy for the post-realloc
  // call.  Unfortunately, we must create it before we know whether we need it.
  va_list dup_vl;
  va_copy(dup_vl, vl);
  int res = vsnprintf(bufPtr_, remaining_, fmt, vl);
  if (res >= remaining_) {
    realloc(res);
    res = vsnprintf(bufPtr_, remaining_, fmt, dup_vl);
    assert(res < remaining_ && "True by construction.");
  }
  remaining_ -= res;
  bufPtr_ += res;
  va_end(vl);
  va_end(dup_vl);
}

void SNPrintfBuf::realloc(int minSize) {
  assert(minSize >= remaining_ && "precondition");
  // We add one to minSize for the terminating null, which is not included.
  int newSize = std::max(minSize + 1, 2 * curSize_);
  std::unique_ptr<char[]> newBuf(new char[newSize]);
  // This will be the number of characters written, not counting terminating
  // null.
  int curUsed = curSize_ - remaining_;
  // Copy the characters, including the terminating null.
  std::memcpy(newBuf.get(), buf_.get(), curUsed + 1);
  // Update the state variables.
  curSize_ = newSize;
  remaining_ = curSize_ - curUsed;
  buf_ = std::move(newBuf);
  bufPtr_ = buf_.get() + curUsed;
}

} // namespace hermes
