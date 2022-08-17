/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H
#define HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H

#include "hermes/IR/IR.h"

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/StringRef.h"

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

/// Walk every function in the bytecode module \p M that passes the predicate \p
/// shouldVisitFunction, calling \p traversal for their source code
/// representations and, if not \p stripFunctionNames, names.
void traverseFunctions(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef)> traversal,
    bool stripFunctionNames);

/// Calls \p traversal with the name of the CommonJS module of every function
/// in bytecode module \p M that passes the predicate \p shouldVisitFunction.
void traverseCJSModuleNames(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef)> traversal);

/// Walk the structure of the bytecode module \p M, calling \p traversal, for
/// each string encountered, with the string's character representation and a
/// boolean that is true if the string is being used as an identifier at that
/// position in the structure.  The predicate \p shouldVisitFunction, is queried
/// for each function, to check whether it should be traversed.  Only usages of
/// strings in functions for which the predicate returns true are traversed.
void traverseLiteralStrings(
    Module *M,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvh::StringRef, bool)> traversal);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_TRAVERSELITERALSTRINGS_H
