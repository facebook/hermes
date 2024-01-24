/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IRUTILS_H
#define HERMES_IR_IRUTILS_H

namespace hermes {

class BasicBlock;
class Function;

/// Delete all unreachable basic blocks from \p F.
/// \return true if anything was deleted, false otherwise.
bool deleteUnreachableBasicBlocks(Function *F);

} // namespace hermes

#endif
