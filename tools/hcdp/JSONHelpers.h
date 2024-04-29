/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HCDP_JSONHELPERS_H
#define HERMES_TOOLS_HCDP_JSONHELPERS_H

#include <optional>
#include <string>

namespace hermes {

/// Parses a JSON string and extract the message ID out from it.
/// \param message JSON string
std::optional<long long> getResponseId(const std::string &message);

} // namespace hermes

#endif // HERMES_TOOLS_HCDP_JSONHELPERS_H
