/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SHA1_H
#define HERMES_SUPPORT_SHA1_H

#include <array>
#include <string>

namespace hermes {

constexpr size_t SHA1_NUM_BYTES = 20;
using SHA1 = std::array<uint8_t, SHA1_NUM_BYTES>;

std::string hashAsString(const SHA1 &hash);

} // namespace hermes

#endif
