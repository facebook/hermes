/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <climits>

#include "hermes/Support/LEB128.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/raw_ostream.h"

namespace {

/**
 * bits<T>::width
 *
 * The size, in bits, of the type `T`.
 */
template <typename T>
struct bits {
  static constexpr size_t width = sizeof(T) * CHAR_BIT;
};

template <size_t B, typename T>
using bigger_signed_int = typename std::enable_if<
    std::is_integral<T>::value && std::is_signed<T>::value &&
        B <= bits<T>::width,
    T>::type;

/**
 * signExtend<B, T>
 *
 * Take a `B`-bit signed twos-complement integer stored in an integral type `T`
 * wider than `B` bits and sign-extend it to the full width of `T`.
 */
template <size_t B, typename T>
inline bigger_signed_int<B, T> signExtend(const T allBits) {
  struct {
    T lowBits : B;
  } s;
  s.lowBits = allBits;
  return s.lowBits;
}

} // anonymous namespace

namespace hermes {

void appendSignedLEB128(std::vector<uint8_t> &vector, int64_t value) {
  llvm::SmallVector<char, 16> data;
  llvm::raw_svector_ostream OS(data);
  llvm::encodeSLEB128(value, OS);

  // "Convert" from char to uint8_t.
  for (int i = 0, e = data.size(); i < e; i++) {
    vector.push_back((uint8_t)data[i]);
  }
}

unsigned readSignedLEB128(
    llvm::ArrayRef<uint8_t> data,
    unsigned offset,
    int64_t *output) {
  unsigned size;
  *output = llvm::decodeSLEB128(&data[offset], &size);
  return size;
}

void encodeSLEB128(int32_t input, llvm::raw_ostream &os, size_t minBytes) {
  constexpr size_t CAP = bits<int32_t>::width / 7 + 1;
  assert(minBytes <= CAP);

  uint8_t buf[CAP]{};

  for (size_t i = 0; i < CAP; ++i) {
    int8_t byte = signExtend<7>(input);

    if (byte != input || i + 1 < minBytes) {
      buf[i] = byte | 0x80;
      input = signExtend<bits<int32_t>::width - 7>(input >> 7);
    } else {
      buf[i] = byte & ~0x80;
      os.write(reinterpret_cast<char *>(buf), i + 1);
      return;
    }
  }

  llvm_unreachable("Input too wide.");
}

} // namespace hermes
