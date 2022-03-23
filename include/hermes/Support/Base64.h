/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BASE64_H
#define HERMES_BASE64_H

#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"

namespace hermes {

/// Decode a plain old style Base64 \p input (e.g. data URL).
/// \return the decoded value, or None if a value could not be decoded.
llvh::Optional<std::string> base64Decode(llvh::StringRef input);

/// Decode a data URL that is specifically in the base64 encoding and with
/// the media type application/json.
/// \return the decoded value, or None if the \p url could not be decoded.
llvh::Optional<std::string> parseJSONBase64DataURL(llvh::StringRef url);

} // namespace hermes

#endif // HERMES_BASE64_H
