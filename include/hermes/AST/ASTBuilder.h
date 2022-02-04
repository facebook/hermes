/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include "llvh/ADT/Optional.h"
#include "llvh/Support/Allocator.h"

namespace llvh {
class MemoryBuffer;
} // namespace llvh

namespace hermes {
namespace ESTree {

using Allocator = llvh::BumpPtrAllocator;

/// Deserialize the ESTree from the input JSON. Reports an error and fails the
/// Optional result on error.
/// \param context   used to allocate the ESTree nodes
/// \param node      the JSON tree to deserialize from.
/// \param jsSource  an optional buffer containing the input JavaScript source
///     for the ESTree. It is used for correlating source locations.
/// \returns the deserialized data structure.
llvh::Optional<Node *> buildAST(
    Context &context,
    const parser::JSONValue *node,
    const llvh::MemoryBuffer *jsSource = nullptr);

} // namespace ESTree
} // namespace hermes

#endif // HERMES_AST_ASTBUILDER_H
