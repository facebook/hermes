/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H
#define HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H

#include "hermes/IR/IR.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringRef.h"

#include <functional>

namespace hermes {
namespace hbc {

/// Walk the structure of the bytecode module \p M, calling \p traversal with
/// the contents of every literal string found therein.  Also accepts a
/// predicate to \p shouldVisitFunction, which is queried for each function,
/// to check whether it should be traversed.
void traverseLiteralStrings(
    Module *M,
    bool includeFunctionNames,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvm::StringRef)> traversal);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H
