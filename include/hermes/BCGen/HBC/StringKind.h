/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STRINGKIND_H
#define HERMES_SUPPORT_STRINGKIND_H

#include "llvh/ADT/ArrayRef.h"

#include <cassert>
#include <cstdint>
#include <vector>

namespace hermes {

/// A collection of classes used to represent the kinds of strings in a Hermes
/// bytecode bundle's string table, and sequences thereof.
class StringKind {
  /// The largest run of equal kinds that can be represented by a single
  /// instance of StringKind::Entry.
  static constexpr uint32_t CountBits = 31;
  static constexpr uint32_t MaxCount = (1u << CountBits) - 1;

 public:
  /// The enclosing class is not intended to be constructed.  It is serving
  /// as a namespace with access protection.
  StringKind() = delete;

  enum Kind : uint32_t {
    /// Not been used as an identifer.
    String = 0u << CountBits,

    /// Used as an identifier.
    Identifier = 1u << CountBits,
  };

  struct Accumulator;

  /// Represents a sequence consisting of one kind repeated some number of
  /// times.  Can be combined with others of the same to represent an arbitrary
  /// sequence of kinds as a run-length encoding.
  struct Entry {
    friend struct StringKind::Accumulator;

    /// Construct an entry representing kind \p k repeated \p count times.
    /// \pre 1 <= count <= MaxCount.
    Entry(Kind k, uint32_t count = 1);

    /// \return the kind being repeated.
    inline Kind kind() const;

    /// \return the number of times the kind has been repeated.
    inline uint32_t count() const;

   private:
    /// Increase the length of the count by one. \returns *this.
    /// \pre count < MaxCount.
    Entry &operator++();

    uint32_t datum_;
  };

  /// Gathers string kinds into a run-length encoded sequence.
  struct Accumulator {
    /// Append kind \p k to the end of the sequence.
    void push_back(Kind k);

    /// View the sequence so far.
    inline llvh::ArrayRef<Entry> entries() const &;

    /// Move the sequence out of the accumulator, leaving it empty.
    inline std::vector<Entry> entries() &&;

   private:
    std::vector<Entry> entries_;
  };
};

inline StringKind::Kind StringKind::Entry::kind() const {
  return static_cast<Kind>(datum_ & ~MaxCount);
}

inline uint32_t StringKind::Entry::count() const {
  return datum_ & MaxCount;
}

inline llvh::ArrayRef<StringKind::Entry> StringKind::Accumulator::entries()
    const & {
  return entries_;
}

inline std::vector<StringKind::Entry> StringKind::Accumulator::entries() && {
  return std::move(entries_);
}

} // namespace hermes

#endif // HERMES_SUPPORT_STRINGKINDENTRY_H
