/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_LEB128_H
#define HERMES_SUPPORT_LEB128_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {

// For some reason, the LLVM LEB128 utilities only allow writing signed values
// to raw_ostream, so here's our own.
void appendSignedLEB128(std::vector<uint8_t> &vector, int64_t value);
unsigned readSignedLEB128(
    llvm::ArrayRef<uint8_t> data,
    unsigned offset,
    int64_t *output);

// An implementation of signed 32bit LEB128 encoding with padding (LLVM's
// implementation does not have this.)
void encodeSLEB128(int32_t input, llvm::raw_ostream &os, size_t minBytes = 0);

} // namespace hermes

#endif // HERMES_SUPPORT_LEB128_H
