/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_PLATFORMDATEFORMAT_H
#define HERMES_PLATFORMUNICODE_PLATFORMDATEFORMAT_H

#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace platform_unicode {

/// Format \p unixtimeMs into \p buf, including the date when \p formatDate is
/// set and the time when \p formatTime is set. The timestamp is interpreted in
/// the host's local timezone.
///
/// The format is fixed and English: "Dec 31, 1969" for the date, "7:00:00 PM"
/// for the time, and the two joined by ", " when both are requested. It does
/// not depend on the host locale, so the output is identical on every machine
/// and in every container.
///
/// This is the fallback for platforms with no locale-aware date formatter of
/// their own. Apple, through CoreFoundation, and Android, through
/// java.text.DateFormat, have one and do not call this.
///
/// \p unixtimeMs must be finite; the behavior is undefined otherwise. The
/// debug assert that checks this is compiled out under NDEBUG, and converting
/// a non-finite double to int64_t is itself undefined behavior. Non-finite
/// timestamps are turned into "Invalid Date" by the caller in Date.cpp and
/// never reach here.
void formatDateTimeFixed(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf);

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_PLATFORMDATEFORMAT_H
