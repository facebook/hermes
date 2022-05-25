/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_FLOWHELPERS_H
#define HERMES_PARSER_FLOWHELPERS_H

#include <cstdint>

namespace hermes {
class Context;
}

namespace hermes {
namespace parser {

bool hasFlowPragma(Context &context, uint32_t bufferId);

} // namespace parser
} // namespace hermes

#endif
