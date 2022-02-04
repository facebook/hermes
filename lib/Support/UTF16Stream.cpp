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
static constexpr size_t kChunkChars = 1024;

UTF16Stream::UTF16Stream(llvh::ArrayRef<uint8_t> utf8)
    : utf8Begin_(utf8.begin()), utf8End_(utf8.end()), storage_(kChunkChars) {
  // Exhaust the buffer and refill it.
  cur_ = end_ = storage_.end();
}

bool UTF16Stream::refill() {
  assert(cur_ == end_ && "cannot refill when data remains");
  if (utf8Begin_ == utf8End_) {
    // Pass-through mode, or final chunk already converted.
    // Either way, there's nothing (more) to convert.
    return false;
  }
  assert(utf8Begin_ && utf8End_ && "must have input to convert");

  // Reset the conversion buffer.
  cur_ = storage_.begin();
  end_ = storage_.end();
  auto out = storage_.begin();

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
