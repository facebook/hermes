/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_FLOWPARSER_FLOWPARSER_FLOWPARSER_H
#define HERMES_FLOWPARSER_FLOWPARSER_FLOWPARSER_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"

#ifdef HERMES_USE_FLOWPARSER

namespace hermes {
namespace parser {

llvh::Optional<ESTree::ProgramNode *> parseFlowParser(
    Context &context,
    uint32_t bufferId);

} // namespace parser
} // namespace hermes

#endif // HERMES_USE_FLOWPARSER

#endif // HERMES_FLOWPARSER_FLOWPARSER_FLOWPARSER_H
