/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/MD5.h"

namespace hermes {

llvh::MD5::MD5Result doMD5Checksum(
    uint32_t funcId,
    llvh::ArrayRef<uint8_t> bytecode) {
  llvh::MD5 md5;
  llvh::MD5::MD5Result checksum;
  md5.update(bytecode);
  md5.update(llvh::ArrayRef<uint8_t>(
      reinterpret_cast<uint8_t *>(&funcId), sizeof(uint32_t)));
  md5.final(checksum);
  return checksum;
}

} // namespace hermes
