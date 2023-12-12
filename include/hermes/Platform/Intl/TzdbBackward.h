/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TZDB_BACKWARD_H
#define HERMES_TZDB_BACKWARD_H

#include <string>
#include <unordered_map>

namespace hermes {
namespace platform_intl {
namespace Tzdb {
namespace Backward {
std::unordered_map<std::u16string, std::u16string> parse();
} // Backward
} // Tzdb
} // namespace platform_intl
} // namespace hermes

#endif
