/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SHA1.h"

#include <cstdio>

namespace hermes {

std::string hashAsString(const SHA1 &hash) {
  char buf[SHA1_NUM_BYTES * 2 + 1];
  for (unsigned i = 0; i < SHA1_NUM_BYTES; i++) {
    // snprintf null-terminates, so overwrite the null terminator in the next
    // iteration of the loop.
    snprintf(buf + (i * 2), 3, "%02x", hash[i]);
  }
  return std::string(buf);
}

} // namespace hermes
