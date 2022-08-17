/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STRINGSETVECTOR_H
#define HERMES_SUPPORT_STRINGSETVECTOR_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/StringRef.h"

#include <deque>
#include <limits>
#include <string>

namespace hermes {

/// Class storing a uniqued set of strings, preserving the order of initial
/// insertion.  This is similar to llvm's SetVector, but more efficient
/// because it does not duplicate the strings in the index.
struct StringSetVector final {
  using iterator = std::deque<std::string>::iterator;
  using const_iterator = std::deque<std::string>::const_iterator;
  using size_type = std::deque<std::string>::size_type;

  StringSetVector() = default;

  // Type is movable.
  StringSetVector(StringSetVector &&) = default;
  StringSetVector &operator=(StringSetVector &&) = default;

  // Type is not copyable.
  StringSetVector(const StringSetVector &) = delete;
  StringSetVector &operator=(const StringSetVector &) = delete;

  /// Adds a string, \p str to the vector if it is not already present.
  /// \return the index of the string in the vector.
  inline size_type insert(llvh::StringRef str);

  /// Return an iterator \c it such that *it == str if such an iterator exists
  /// or end() otherwise.
  inline iterator find(llvh::StringRef str);
  inline const_iterator find(llvh::StringRef str) const;

  /// Return a reference to the \p i'th string inserted into this set vector.
  /// Assumes \pre i < size().
  inline const std::string &operator[](size_type i) const;

  inline size_type size() const;
  inline bool empty() const;

  inline iterator begin();
  inline const_iterator begin() const;

  inline iterator end();
  inline const_iterator end() const;

 private:
  /// The object that owns storage for the strings, in initial insertion order.
  /// We use a deque so that insertions do not move previous strings.  We seek
  /// to preserve them because small strings' characters may be stored in-line
  /// so a StringRef of a small string would point into the string and would be
  /// invalidated if it were moved.
  std::deque<std::string> stringsStorage_;

  /// A map from strings to their index in the storage.  The strings referred
  /// to in the keys of this mapping are owned by \c stringsToStorage_.
  using index_type = uint32_t;
  llvh::DenseMap<llvh::StringRef, index_type> stringsToIndex_;
};

inline StringSetVector::size_type StringSetVector::insert(llvh::StringRef str) {
  assert(stringsToIndex_.size() == stringsStorage_.size());
  auto it = stringsToIndex_.find(str);
  if (it != stringsToIndex_.end()) {
    return it->second;
  }

  size_type storageSize = stringsStorage_.size();
  assert(
      storageSize < std::numeric_limits<index_type>::max() &&
      "StringSetVector cannot store that many elements");
  auto newIdx = static_cast<index_type>(storageSize);
  assert(storageSize == newIdx && "casted value unexpectedly different");
  stringsStorage_.emplace_back(str.begin(), str.end());
  auto inserted = stringsToIndex_.insert({stringsStorage_.back(), newIdx});
  assert(inserted.second && "String already exists as key in mapping.");
  (void)inserted;

  return storageSize;
}

inline StringSetVector::iterator StringSetVector::find(llvh::StringRef str) {
  auto it = stringsToIndex_.find(str);
  if (it == stringsToIndex_.end()) {
    return end();
  }

  return stringsStorage_.begin() + it->second;
}

inline StringSetVector::const_iterator StringSetVector::find(
    llvh::StringRef str) const {
  return const_cast<StringSetVector *>(this)->find(str);
}

inline const std::string &StringSetVector::operator[](size_t i) const {
  return stringsStorage_[i];
}

inline size_t StringSetVector::size() const {
  return stringsStorage_.size();
}

inline bool StringSetVector::empty() const {
  return size() == 0;
}

inline StringSetVector::iterator StringSetVector::begin() {
  return stringsStorage_.begin();
}

inline StringSetVector::const_iterator StringSetVector::begin() const {
  return stringsStorage_.begin();
}

inline StringSetVector::iterator StringSetVector::end() {
  return stringsStorage_.end();
}

inline StringSetVector::const_iterator StringSetVector::end() const {
  return stringsStorage_.end();
}

} // namespace hermes

#endif // HERMES_SUPPORT_STRINGSETVECTOR_H
