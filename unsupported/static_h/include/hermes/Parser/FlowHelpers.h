/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_FLOWHELPERS_H
#define HERMES_PARSER_FLOWHELPERS_H

#include "hermes/Parser/JSLexer.h"

#include <cstdint>
#include <string>
#include <vector>

namespace hermes {
class Context;
}

namespace hermes {
namespace parser {

/// \return the comments in the \p bufferId file in the context.
/// Each StoredComment contains pointers directly into the buffer.
std::vector<StoredComment> getCommentsInDocBlock(
    Context &context,
    uint32_t bufferId);

/// \return true if one of the comments contains "@flow" followed by word
/// boundary.
bool hasFlowPragma(llvh::ArrayRef<StoredComment> comments);

/// Concatenate \p comments into a docblock.
/// \return a string of all the comments with newlines between them.
std::string getDocBlock(llvh::ArrayRef<StoredComment> comments);

} // namespace parser
} // namespace hermes

#endif
