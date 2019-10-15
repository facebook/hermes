/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ASTBUILDER_H
#define HERMES_AST_ASTBUILDER_H

#include "Context.h"
#include "ESTree.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/SourceErrorManager.h"

#include "llvm/ADT/Optional.h"
#include "llvm/Support/Allocator.h"

namespace llvm {
class MemoryBuffer;
} // namespace llvm

namespace hermes {
namespace ESTree {

using Allocator = llvm::BumpPtrAllocator;

/// Deserialize the ESTree from the input JSON. Reports an error and fails the
/// Optional result on error.
/// \param allocator used to allocate the ESTree nodes
/// \param sm        used to report errors
/// \param node      the JSON tree to deserialize from.
/// \param jsSource  an optional buffer containing the input JavaScript source
///     for the ESTree. It is used for correlating source locations.
/// \returns the deserialized data structure.
llvm::Optional<Node *> buildAST(
    Context &context,
    const parser::JSONValue *node,
    const llvm::MemoryBuffer *jsSource = nullptr);

} // namespace ESTree
} // namespace hermes

#endif // HERMES_AST_ASTBUILDER_H
