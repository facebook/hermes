/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/UTF16Stream.h"

#include <cstdlib>

#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/ConvertUTF.h"
#include "llvh/Support/MathExtras.h"

namespace hermes {

/// Number of char16_t in the internal conversion buffer (if UTF8 input).
static constexpr size_t kDefaultBufferSize = 1024;

UTF16Stream::UTF16Stream(llvh::ArrayRef<uint8_t> utf8)
    : utf8Begin_(utf8.begin()),
      utf8End_(utf8.end()),
      beginCapture_(nullptr),
      storage_(kDefaultBufferSize) {
  // Exhaust the buffer and refill it.
  cur_ = end_ = &storage_.back();
}

void UTF16Stream::beginCapture() {
  assert(!beginCapture_ && "there is already an active capture");
  beginCapture_ = cur_;
}

llvh::ArrayRef<char16_t> UTF16Stream::endCapture() {
  assert(beginCapture_ && "no active capture");
  const char16_t *b = beginCapture_;
  beginCapture_ = nullptr;
  return {b, cur_};
}

void UTF16Stream::cancelCapture() {
  beginCapture_ = nullptr;
}

void UTF16Stream::makeRoomForCapture() {
  assert(beginCapture_ && "no active capture");
  assert(cur_ == end_ && "there is still room left for the capture");
  size_t beginCaptureOffset = beginCapture_ - &storage_.front();
  // We only grow the conversion buffer if the the beginning of the capture is
  // in the first half of the buffer. Otherwise, we just copy the current
  // capture into the beginning of the storage.
  if (beginCaptureOffset < storage_.size() / 2) {
    // The buffer may get moved, in which case our current pointers into it will
    // be invalidated. In order to reset the pointers, we need to calculate
    // their offset into the buffer, then apply that same offset to the new
    // buffer.
    size_t curOffset = cur_ - &storage_.front();
    storage_.resize(storage_.size() * 2);
    end_ = &storage_.back();
    beginCapture_ = &storage_.front() + beginCaptureOffset;
    cur_ = &storage_.front() + curOffset;
  } else {
    size_t captureLen = end_ - beginCapture_;
    std::copy(beginCapture_, end_, storage_.begin());
    beginCapture_ = &storage_.front();
    cur_ = &storage_.front() + captureLen;
  }
}

bool UTF16Stream::refill() {
  assert(cur_ == end_ && "cannot refill when data remains");
  if (utf8Begin_ == utf8End_) {
    // Pass-through mode, or final chunk already converted.
    // Either way, there's nothing (more) to convert.
    return false;
  }
  assert(utf8Begin_ && utf8End_ && "must have input to convert");

  if (beginCapture_) {
    // There is an active capture happening, so we need to provide more room for
    // the capture to continue growing.
    makeRoomForCapture();
  } else {
    // Check to see if we grew the conversion buffer extremely long from a past
    // capture. If it is too long, shorten it back to the default length.
    if (storage_.size() > (kDefaultBufferSize * 2)) {
      storage_.resize(kDefaultBufferSize);
    }
    cur_ = &storage_.front();
  }
  char16_t *out = const_cast<char16_t *>(cur_);
  end_ = &storage_.back();

  // Fast case for any ASCII prefix...
  {
    int len = std::min(end_ - cur_, utf8End_ - utf8Begin_);
    int index = 0;
    while (index < len && utf8Begin_[index] < 128) {
      out[index] = utf8Begin_[index];
      ++index;
    }
    utf8Begin_ += index;
    out += index;
  }

  // ...and call the library for any non-ASCII remainder. Conversion always
  // stops at a code point boundary.
  llvh::ConversionResult cRes = ConvertUTF8toUTF16(
      &utf8Begin_,
      utf8End_,
      (llvh::UTF16 **)&out,
      (llvh::UTF16 *)const_cast<char16_t *>(end_),
      llvh::lenientConversion);

  if (cRes != llvh::ConversionResult::targetExhausted) {
    // Indicate that we have converted the final chunk.
    utf8Begin_ = utf8End_;
  }
  end_ = out;

  // Did we actually convert anything?
  return cur_ != end_;
}

} // namespace hermes
