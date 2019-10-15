/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_REGEXPSERIALIZATION_H
#define HERMES_SUPPORT_REGEXPSERIALIZATION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"

#include <deque>
#include <memory>
#include <string>

namespace llvm {
class raw_ostream;
}

/// Support for statically compiling regexps.
namespace hermes {

/// RegExpBytecode is the bytecode form of a CompiledRegExp, and simply a list
/// of bytes.
using RegExpBytecode = std::vector<unsigned char>;

/// A RegExpTableEntry is simply an (offset, length) pair for bytecode.
struct RegExpTableEntry {
  uint32_t offset;
  uint32_t length;
};

/// Dump a regexp bytecode stream to a raw_ostream \p OS.
void dumpRegexBytecode(llvm::ArrayRef<uint8_t> bytes, llvm::raw_ostream &OS);

/// CompiledRegExp represents a syntax-validated intermediate form of regular
/// expressions.
class CompiledRegExp {
 public:
  ~CompiledRegExp();
  CompiledRegExp(CompiledRegExp &&);
  CompiledRegExp &operator=(CompiledRegExp &&);

  /// Validate a RegExp given by the pattern \p pattern and flags \p flags.
  /// \return a CompiledRegExp if the regexp syntax is valid, llvm::None if it
  /// is invalid, in which case we return an error message by reference in \p
  /// outError (if not null).
  static llvm::Optional<CompiledRegExp> tryCompile(
      llvm::StringRef pattern,
      llvm::StringRef flags,
      llvm::StringRef *outError = nullptr);

  llvm::StringRef getPattern() const {
    return pattern_;
  }

  llvm::StringRef getFlags() const {
    return flags_;
  }

  /// \return regexp-specific bytecode for the receiver.
  llvm::ArrayRef<uint8_t> getBytecode() const;

 private:
  std::vector<uint8_t> bytecode_;
  std::string pattern_;
  std::string flags_;
  CompiledRegExp(
      std::vector<uint8_t> bytecode,
      std::string pattern,
      std::string flags);
  CompiledRegExp(const CompiledRegExp &) = delete;
  void operator=(const CompiledRegExp &) = delete;
};

/// A class responsible for assigning IDs to regexps.
class UniquingRegExpTable {
  /// List of compiled regexps. Use deque because it does not move elements when
  /// appending, which might invalidate the StringRefs.
  std::deque<CompiledRegExp> regexps_;

  /// RegExps are uniqued according to their pattern and flags. Note that a
  /// regexp pattern is logically UCS-2 (or UTF-16 with the 'u' flag). We match
  /// StringStorage in that we represent this as UTF-8, except that UTF-16
  /// surrogate code points are explicitly encoded via the UTF-8 format.
  using KeyType = std::pair<llvm::StringRef, llvm::StringRef>;

  /// Map from compiled regexp (pattern, flags) pairs to its index. The
  /// StringRefs reference data owned by the regexps_ field.
  llvm::DenseMap<KeyType, uint32_t> keysToIndex_;

  /// A UniquingRegExpTable may not be copied.
  UniquingRegExpTable(const UniquingRegExpTable &) = delete;
  void operator=(const UniquingRegExpTable &) = delete;

 public:
  UniquingRegExpTable() = default;

  /// Adds a regexp to the table if not already present.
  /// \return the ID of the regexp.
  uint32_t addRegExp(CompiledRegExp regexp) {
    auto iter = keysToIndex_.find(keyFor(regexp));
    if (iter != keysToIndex_.end())
      return iter->second;

    uint32_t index = regexps_.size();
    regexps_.push_back(std::move(regexp));
    keysToIndex_[keyFor(regexps_.back())] = index;
    return index;
  }

  /// \return whether the table is empty.
  bool empty() const {
    return regexps_.empty();
  }

  /// Return the regexp entry list.
  std::vector<RegExpTableEntry> getEntryList() const;

  /// Return the combined bytecode buffer.
  RegExpBytecode getBytecodeBuffer() const;

  /// Disassemble the regexp bytecode, printing the result to the output stream.
  void disassemble(llvm::raw_ostream &OS) const;

 private:
  KeyType keyFor(const CompiledRegExp &re) const {
    return {re.getPattern(), re.getFlags()};
  }
};

} // namespace hermes

#endif
