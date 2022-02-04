/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BYTECODEDELTAPREPARER_H
#define HERMES_BYTECODEDELTAPREPARER_H

#include <string>
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace hbc {

/// Convert a bytecode file in \p buffer to the target form \p form, in-place.
/// \return true if successful, false if the file could not be intepreted.
/// On a false return, an error is returned in outError.
bool convertBytecodeToForm(
    llvh::MutableArrayRef<uint8_t> buffer,
    BytecodeForm targetForm,
    std::string *outError);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BYTECODEDELTAPREPARER_H
