/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STRINGTABLE_H
#define HERMES_SUPPORT_STRINGTABLE_H

#include "hermes/Support/Allocator.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/StringRef.h"

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {

/// Allocate a StringRef with a '\0' following after the end.
template <class Allocator>
llvh::StringRef zeroTerminate(Allocator &allocator, llvh::StringRef str) {
  // Allocate a copy of the name, adding a trailing \0 for convenience.
  auto *s = allocator.template Allocate<char>(str.size() + 1);
  auto end = std::copy(str.begin(), str.end(), s);
  *end = 0; // Zero terminate string.

  // NOTE: returning the original size.
  return llvh::StringRef(s, str.size());
}

class UniqueString {
  const llvh::StringRef str_;

  UniqueString(const UniqueString &) = delete;
  UniqueString &operator=(const UniqueString &) = delete;

 public:
  explicit UniqueString(llvh::StringRef str) : str_(str){};

  const llvh::StringRef &str() const {
    return str_;
  }
  const char *c_str() const {
    return str_.begin();
  }

  explicit operator llvh::StringRef() const {
    return str_;
  }
};

/// This is an instance of a uniqued identifier created by StringTable. It is
/// just a wrapper around a UniqueString pointer. Identifier is passed by value
/// and must be kept small.
class Identifier {
 public:
  using PtrType = UniqueString *;

  explicit Identifier() = default;

 private:
  PtrType ptr_{nullptr};

  explicit Identifier(PtrType ptr) : ptr_(ptr) {}

 public:
  bool isValid() const {
    return ptr_ != nullptr;
  }

  /// \returns the pointer value that the context uses to index this string. We
  /// Also use this value to hash the identifier.
  PtrType getUnderlyingPointer() const {
    return ptr_;
  }

  static Identifier getFromPointer(UniqueString *ptr) {
    return Identifier(ptr);
  }

  bool operator==(Identifier RHS) const {
    return ptr_ == RHS.ptr_;
  }
  bool operator!=(Identifier RHS) const {
    return !(*this == RHS);
  }

  const llvh::StringRef &str() const {
    return ptr_->str();
  }
  const char *c_str() const {
    return ptr_->c_str();
  }
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, Identifier id);

/// Encapsulates a table of unique zero-terminated strings. Unlike
/// llvh::StringMap it gives us access to the string itself and also provides
/// convenient zero termination.
class StringTable {
  using Allocator = hermes::BumpPtrAllocator;
  Allocator &allocator_;

  llvh::DenseMap<llvh::StringRef, UniqueString *> strMap_{};

  StringTable(const StringTable &) = delete;
  StringTable &operator=(const StringTable &_) = delete;

 public:
  explicit StringTable(Allocator &allocator) : allocator_(allocator){};

  /// Return a unique zero-terminated copy of the supplied string \p name.
  UniqueString *getString(llvh::StringRef name) {
    // Already in the map?
    auto it = strMap_.find(name);
    if (it != strMap_.end())
      return it->second;

    // Allocate a zero-terminated copy of the string
    auto *str = new (allocator_.Allocate<UniqueString>())
        UniqueString(zeroTerminate(allocator_, name));
    strMap_.insert({str->str(), str});
    return str;
  }

  /// A wrapper arond getString() returning an Identifier.
  Identifier getIdentifier(llvh::StringRef name) {
    return Identifier::getFromPointer(getString(name));
  }
};

} // namespace hermes

// Enable using Identifier in DenseMap.
namespace llvh {

template <>
struct DenseMapInfo<hermes::Identifier> {
  static inline hermes::Identifier getEmptyKey() {
    return hermes::Identifier::getFromPointer(
        DenseMapInfo<hermes::UniqueString *>::getEmptyKey());
  }
  static inline hermes::Identifier getTombstoneKey() {
    return hermes::Identifier::getFromPointer(
        DenseMapInfo<hermes::UniqueString *>::getTombstoneKey());
  }
  static inline unsigned getHashValue(hermes::Identifier id) {
    return DenseMapInfo<hermes::UniqueString *>::getHashValue(
        id.getUnderlyingPointer());
  }
  static inline bool isEqual(hermes::Identifier a, hermes::Identifier b) {
    return a == b;
  }
};

} // namespace llvh

#endif // HERMES_SUPPORT_STRINGTABLE_H
