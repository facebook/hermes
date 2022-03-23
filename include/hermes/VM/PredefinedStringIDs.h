/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PREDEFINEDSTRINGIDS_H
#define HERMES_VM_PREDEFINEDSTRINGIDS_H

#include "hermes/VM/Predefined.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"

namespace hermes {
namespace vm {

/// \return the SymbolID corresponding to string \p str if it is one of the
/// predefined strings.  Returns None otherwise.
llvh::Optional<vm::SymbolID> getPredefinedStringID(llvh::StringRef str);

/// the length of each predefined property.
extern const llvh::ArrayRef<uint8_t> predefPropertyLengths;

/// the length of each predefined string.
extern const llvh::ArrayRef<uint8_t> predefStringLengths;

/// the length of each predefined string.
extern const llvh::ArrayRef<uint8_t> predefSymbolLengths;

/// an array of all concatenated predefined strings immediately
/// followed by all concatenated predefined symbols, all ASCII.
extern const llvh::ArrayRef<char> predefStringAndSymbolChars;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PREDEFINEDSTRINGIDS_H
