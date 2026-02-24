/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODESTREAM_H
#define HERMES_BCGEN_HBC_BYTECODESTREAM_H

#include "hermes/Support/SHA1.h"
#include "hermes/Utils/Options.h"

#include "llvh/Support/SHA1.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace hbc {

class BytecodeModule;

/// Serializes the provided BytecodeModule to an output stream.
/// \param BM the BytecodeModule to serialize.
/// \param sourceHash the hash of the source being serialized.
/// \param os the output stream to serialize to.
/// \param options the generation options.
void serializeBytecodeModule(
    BytecodeModule &BM,
    const SHA1 &sourceHash,
    llvh::raw_ostream &os,
    BytecodeGenerationOptions options = BytecodeGenerationOptions::defaults());

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODESTREAM_H
