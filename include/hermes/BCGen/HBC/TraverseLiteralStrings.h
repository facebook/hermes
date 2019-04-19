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

/// These functions are used during bytecode generation to exhaustively
/// traverse *every* occurrence of a string in the IR that needs to appear in
/// the final bytecode output.  If a string is not visited by one of these
/// functions, it will not be available in the resulting bytecode's string
/// table.  The compiler will assert at any attempt to request the string table
/// ID for a string that has not been traversed in this way (if assertions have
/// been enabled).

/// Walk the structure of the bytecode module \p M, calling \p traversal with
/// the contents of every literal string found therein.  Also accepts a
/// predicate to \p shouldVisitFunction, which is queried for each function,
/// to check whether it should be traversed.
void traverseLiteralStrings(
    Module *M,
    bool includeFunctionNames,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvm::StringRef, bool)> traversal);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H
