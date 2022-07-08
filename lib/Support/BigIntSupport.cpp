/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/BigIntSupport.h"

#include "hermes/Support/OptValue.h"

#include "llvh/ADT/APInt.h"
#include "llvh/Support/Endian.h"

#include <cmath>
#include <string>

namespace hermes {
namespace bigint {
llvh::ArrayRef<uint8_t> dropExtraSignBits(llvh::ArrayRef<uint8_t> src) {
  if (src.empty()) {
    // return an empty array ref.
    return src;
  }

  const uint8_t drop = getSignExtValue<uint8_t>(src.back());

  // Iterate over all bytes in src, in reverse order, and drop everything that
  // can be inferred with a sign-extension from the previous byte. For example,
  //
  // src = { 0x00, 0x00, 0x00, 0xff }
  //
  // results in { 0x00, 0xff } so that sign extension results in the original
  // sequence being reconstructed.

  auto previousSrc = src;
  while (!src.empty() && src.back() == drop) {
    previousSrc = src;
    src = src.drop_back();
  }

  // Invariants:
  //
  //  * previousSrc.size() > 0
  //  * previousSrc == src -> no bytes dropped from src
  //  * previousSrc != src -> previousSrc.back() == drop
  //  * src.empty() -> original src = {drop, drop, drop, ..., drop, drop} and
  //                   previousSrc[0] == drop
  //
  // The return value should be
  //  * {} iff src.empty and drop == 0x00; or
  //  * {0xff} iff src.empty and drop == 0xff; or
  //  * src iff getSignExtValue(src.back()) == drop; and
  //  * previousSrc otherwise
  //
  // which can be expressed as
  //  * src iff src.empty and drop == 0x00; or
  //  * previousSrc iff src.empty and drop == 0xff; or
  //  * src iff getSignExtValue(src.back()) == drop; and
  //  * previousSrc otherwise
  //
  // By defining
  //   lastChar = src.empty ? 0 : src.back,
  //
  // the return value can be expressed as
  //  * src iff getSignExtValue(lastChar) == drop; and
  //  * previousSrc otherwise
  const uint8_t lastChar = src.empty() ? 0u : src.back();

  return getSignExtValue<uint8_t>(lastChar) == drop ? src : previousSrc;
}

namespace {
/// Trims any digits in \p dst that can be inferred by a sign extension.
void ensureCanonicalResult(MutableBigIntRef &dst) {
  auto ptr = reinterpret_cast<uint8_t *>(dst.digits);
  const uint32_t sizeInBytes = dst.numDigits * BigIntDigitSizeInBytes;

  llvh::ArrayRef<uint8_t> compactView =
      dropExtraSignBits(llvh::makeArrayRef(ptr, sizeInBytes));
  dst.numDigits = numDigitsForSizeInBytes(compactView.size());
}
} // namespace

// Ensure there's a compile-time failure if/when hermes is compiled for
// big-endian machines. This is needed for correct serialization and
// deserialization (the hermes bytecode format expects bigint bytes in
// little-endian format).
static_assert(
    llvh::support::endian::system_endianness() == llvh::support::little,
    "BigIntSupport expects little-endian host");

OperationStatus initWithBytes(
    MutableBigIntRef dst,
    llvh::ArrayRef<uint8_t> data) {
  const uint32_t dstSizeInBytes = dst.numDigits * BigIntDigitSizeInBytes;

  assert(dst.digits != nullptr && "buffer can't be nullptr");

  if (dstSizeInBytes < data.size()) {
    // clear numDigits in the response (i.e., sanitizing the output).
    dst.numDigits = 0;
    return OperationStatus::DEST_TOO_SMALL;
  }

  const size_t dataSizeInBytes = data.size();

  if (dataSizeInBytes == 0) {
    // data is empty, so don't bother copying it to dst; simply return 0n.
    dst.numDigits = 0;
    return OperationStatus::RETURNED;
  }

  // Get a uint8_t* to dst so we can do pointer arithmetic.
  auto *ptr = reinterpret_cast<uint8_t *>(dst.digits);

  // Copy bytes first; dataSizeInBytes may not be a multiple of
  // BigIntDigitSizeInBytes.
  memcpy(ptr, data.data(), dataSizeInBytes);

  // Now sign-extend to a length that's multiple of DigitType size. Note that
  // dataSizeInBytes is not zero (otherwise the function would have returned)
  // by now.
  const uint32_t numBytesToSet = dstSizeInBytes - dataSizeInBytes;
  const uint8_t signExtValue =
      getSignExtValue<uint8_t>(ptr[dataSizeInBytes - 1]);

  memset(ptr + dataSizeInBytes, signExtValue, numBytesToSet);

  ensureCanonicalResult(dst);
  return OperationStatus::RETURNED;
}

} // namespace bigint
} // namespace hermes
