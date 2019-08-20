/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_PREDEFINEDSTRINGIDS_H
#define HERMES_BCGEN_HBC_PREDEFINEDSTRINGIDS_H

#include "hermes/VM/Predefined.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"

namespace hermes {
namespace hbc {

/// \return the SymbolID corresponding to string \p str if it is one of the
/// predefined strings.  Returns None otherwise.
llvm::Optional<vm::SymbolID> getPredefinedStringID(llvm::StringRef str);

/// the length of each predefined string.
extern const llvm::ArrayRef<uint8_t> predefStringLengths;

/// the length of each predefined string.
extern const llvm::ArrayRef<uint8_t> predefSymbolLengths;

/// an array of all concatenated predefined strings immediately
/// followed by all concatenated predefined symbols, all ASCII.
extern const llvm::ArrayRef<char> predefStringAndSymbolChars;

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_PREDEFINEDSTRINGIDS_H
