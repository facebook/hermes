/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_FAST_STR_TO_DOUBLE
#define HERMES_SUPPORT_FAST_STR_TO_DOUBLE

#include "hermes/Support/OptValue.h"
#include "llvh/ADT/ArrayRef.h"

namespace hermes {

/// Parse a string into a double.
/// \param numView the string contents to parse.
/// \return None if the parse was not successful.
OptValue<double> fastStrToDouble(llvh::ArrayRef<char> numView);

/// Overload for char16_t
OptValue<double> fastStrToDouble(llvh::ArrayRef<char16_t> numView);

} // namespace hermes

#endif // HERMES_SUPPORT_FAST_STR_TO_DOUBLE
