/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_BIGINTTESTHELPERS_H
#define HERMES_SUPPORT_BIGINTTESTHELPERS_H

#include "hermes/Support/BigIntSupport.h"

#include "llvh/ADT/STLExtras.h"

#include <iomanip>
#include <iostream>

namespace hermes {
namespace bigint {

/// Outputs \p v in reverse order -- i.e., from MSB to LSB.
inline std::ostream &outputLeftToRightBytes(
    std::ostream &out,
    const llvh::ArrayRef<uint8_t> &v) {
  auto outFlags = out.flags();
  out << "MSB [";
  bool first = true;
  // use uint32_t instead of uint8_t to for numeric output to out.
  for (uint32_t byte : llvh::reverse(v)) {
    if (!first) {
      out << " ";
    }
    first = false;
    out << std::hex << std::setw(2) << std::setfill('0') << byte;
  }
  out << "] LSB";
  out.flags(outFlags);
  return out;
}

/// Tagged type to make it clear that data is stored left to right -- i.e., LSB
/// is data[0], MSB is data[data.size() - 1]. The idea is to allow tests to be
/// written using the "natural" way to write numbers, i.e., big endian
///
/// 1.234.567.890
/// ^           ^
/// most        least
///
/// even though bigints themselves are little endian. Thus, all the constructors
/// in this class will populate data with the reversed "bytes" payload.
struct LeftToRightVector {
  LeftToRightVector() = default;

  LeftToRightVector(std::initializer_list<uint8_t> bytes)
      : data(std::rbegin(bytes), std::rend(bytes)) {}

  operator llvh::ArrayRef<uint8_t>() const {
    return data;
  }

  std::vector<uint8_t> data;
};

inline std::ostream &operator<<(std::ostream &out, const LeftToRightVector &v) {
  return outputLeftToRightBytes(out, v.data);
}

/// \return a LeftToRightVector that's the result of concatenating \p lhs
/// with \p rhs. The bytes in rhs are less-significant, thus they should appear
/// first in the result.
inline LeftToRightVector operator+(
    const LeftToRightVector &lhs,
    const LeftToRightVector &rhs) {
  LeftToRightVector ret;
  ret.data = rhs.data;
  ret.data.insert(ret.data.end(), lhs.data.begin(), lhs.data.end());
  return ret;
}

/// Helper constructor for creating a LeftToRightVector representing a BigInt
/// digit. Arguments are passed in MSB to LSB, i.e.,
///
/// digit(1,2,3,4) == 0x0000000001020304ull
///
/// which means they need to be added in reverse order to the returned vector.
template <typename... B>
inline LeftToRightVector digit(B &&...bytes) {
  static_assert(sizeof...(bytes) > 0, "empty digit?");
  static_assert(sizeof...(bytes) <= BigIntDigitSizeInBytes, "too many bytes");

  return {static_cast<uint8_t>(bytes)...};
}

/// Helper constructor for creating an empty LeftToRightVector.
inline LeftToRightVector noDigits() {
  return LeftToRightVector{};
}

/// \return \p lhs.data == \p rhs.data.
inline bool operator==(
    const LeftToRightVector &lhs,
    const LeftToRightVector &rhs) {
  return lhs.data == rhs.data;
}

/// \return \p lhs.data == \p rhs.bag.
inline bool operator==(
    const LeftToRightVector &lhs,
    const llvh::ArrayRef<uint8_t> &rhs) {
  return llvh::makeArrayRef(lhs.data) == rhs;
}

} // namespace bigint
} // namespace hermes

#endif // HERMES_SUPPORT_BIGINTTESTHELPERS_H
