/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_BASE64VLQ_H
#define HERMES_SUPPORT_BASE64VLQ_H

#include "hermes/Support/OptValue.h"

#include "llvh/Support/raw_ostream.h"

#include <vector>

namespace hermes {

/// The vlq namespace contains support for SourceMap-style Base-64
/// variable-length quantities.
namespace base64vlq {

/// Encode \p value into \p OS as a Base64 variable-length quantity.
/// \return OS.
llvh::raw_ostream &encode(llvh::raw_ostream &OS, int32_t value);

/// Decode a Base64 variable-length quantity from the range starting at \p begin
/// and ending at \p end (whose length is end - begin).
/// \return the decoded value, or None if a value could not be decoded.
/// If a value could be decoded, \p begin is updated to point after the end of
/// the string.
OptValue<int32_t> decode(const char *&begin, const char *end);

} // namespace base64vlq

} // namespace hermes

#endif
