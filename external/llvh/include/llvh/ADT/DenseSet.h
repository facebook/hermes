//===- llvm/ADT/DenseSet.h - Dense probed hash table ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DenseSet and SmallDenseSet classes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_DENSESET_H
#define LLVM_ADT_DENSESET_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseMapInfo.h"
#include "llvh/Support/type_traits.h"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace llvh {

namespace detail {

struct DenseSetEmpty {};

// Use the empty base class trick so we can create a DenseMap where the buckets
// contain only a single item.
template <typename KeyT> class DenseSetPair : public DenseSetEmpty {
  KeyT key;

public:
  KeyT &getFirst() { return key; }
  const KeyT &getFirst() const { return key; }
  DenseSetEmpty &getSecond() { return *this; }
  const DenseSetEmpty &getSecond() const { return *this; }
};

/// Base class for DenseSet and DenseSmallSet.
///
/// MapTy should be either
///
///   DenseMap<ValueT, detail::DenseSetEmpty, ValueInfoT,
///            detail::DenseSetPair<ValueT>>
///
/// or the equivalent SmallDenseMap type.  ValueInfoT must implement the
/// DenseMapInfo "concept".
template <typename ValueT, typename MapTy, typename ValueInfoT>
class DenseSetImpl {
  static_assert(sizeof(typename MapTy::value_type) == sizeof(ValueT),
                "DenseMap buckets unexpectedly large!");
  MapTy TheMap;

  template <typename T>
  using const_arg_type_t = typename const_pointer_or_const_ref<T>::type;

public:
  using key_type = ValueT;
  using value_type = ValueT;
  using size_type = unsigned;

  explicit DenseSetImpl(unsigned InitialReserve = 0) : TheMap(InitialReserve) {}

  DenseSetImpl(std::initializer_list<ValueT> Elems)
      : DenseSetImpl(Elems.size()) {
    insert(Elems.begin(), Elems.end());
  }

  bool empty() const { return TheMap.empty(); }
  size_type size() const { return TheMap.size(); }
  size_t getMemorySize() const { return TheMap.getMemorySize(); }

  /// Grow the DenseSet so that it has at least Size buckets. Will not shrink
  /// the Size of the set.
  void resize(size_t Size) { TheMap.resize(Size); }

  /// Grow the DenseSet so that it can contain at least \p NumEntries items
  /// before resizing again.
  void reserve(size_t Size) { TheMap.reserve(Size); }

  void clear() {
    TheMap.clear();
  }

  /// Return 1 if the specified key is in the set, 0 otherwise.
  size_type count(const_arg_type_t<ValueT> V) const {
    return TheMap.count(V);
  }

  bool erase(const ValueT &V) {
    return TheMap.erase(V);
  }

  void swap(DenseSetImpl &RHS) { TheMap.swap(RHS.TheMap); }

  // Iterators.

  class ConstIterator;

  class Iterator {
    typename MapTy::iterator I;
    friend class DenseSetImpl;
    friend class ConstIterator;

  public:
    using difference_type = typename MapTy::iterator::difference_type;
    using value_type = ValueT;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::forward_iterator_tag;

    Iterator() = default;
    Iterator(const typename MapTy::iterator &i) : I(i) {}

    ValueT &operator*() { return I->getFirst(); }
    const ValueT &operator*() const { return I->getFirst(); }
    ValueT *operator->() { return &I->getFirst(); }
    const ValueT *operator->() const { return &I->getFirst(); }

    Iterator& operator++() { ++I; return *this; }
    Iterator operator++(int) { auto T = *this; ++I; return T; }
    bool operator==(const ConstIterator& X) const { return I == X.I; }
    bool operator!=(const ConstIterator& X) const { return I != X.I; }
  };

  class ConstIterator {
    typename MapTy::const_iterator I;
    friend class DenseSet;
    friend class Iterator;

  public:
    using difference_type = typename MapTy::const_iterator::difference_type;
    using value_type = ValueT;
    using pointer = const value_type *;
    using reference = const value_type &;
    using iterator_category = std::forward_iterator_tag;

    ConstIterator() = default;
    ConstIterator(const Iterator &B) : I(B.I) {}
    ConstIterator(const typename MapTy::const_iterator &i) : I(i) {}

    const ValueT &operator*() const { return I->getFirst(); }
    const ValueT *operator->() const { return &I->getFirst(); }

    ConstIterator& operator++() { ++I; return *this; }
    ConstIterator operator++(int) { auto T = *this; ++I; return T; }
    bool operator==(const ConstIterator& X) const { return I == X.I; }
    bool operator!=(const ConstIterator& X) const { return I != X.I; }
  };

  using iterator = Iterator;
  using const_iterator = ConstIterator;

  iterator begin() { return Iterator(TheMap.begin()); }
  iterator end() { return Iterator(TheMap.end()); }

  const_iterator begin() const { return ConstIterator(TheMap.begin()); }
  const_iterator end() const { return ConstIterator(TheMap.end()); }

  iterator find(const_arg_type_t<ValueT> V) { return Iterator(TheMap.find(V)); }
  const_iterator find(const_arg_type_t<ValueT> V) const {
    return ConstIterator(TheMap.find(V));
  }

  /// Alternative version of find() which allows a different, and possibly less
  /// expensive, key type.
  /// The DenseMapInfo is responsible for supplying methods
  /// getHashValue(LookupKeyT) and isEqual(LookupKeyT, KeyT) for each key type
  /// used.
  template <class LookupKeyT>
  iterator find_as(const LookupKeyT &Val) {
    return Iterator(TheMap.find_as(Val));
  }
  template <class LookupKeyT>
  const_iterator find_as(const LookupKeyT &Val) const {
    return ConstIterator(TheMap.find_as(Val));
  }

  void erase(Iterator I) { return TheMap.erase(I.I); }
  void erase(ConstIterator CI) { return TheMap.erase(CI.I); }

  std::pair<iterator, bool> insert(const ValueT &V) {
    detail::DenseSetEmpty Empty;
    return TheMap.try_emplace(V, Empty);
  }

  std::pair<iterator, bool> insert(ValueT &&V) {
    detail::DenseSetEmpty Empty;
    return TheMap.try_emplace(std::move(V), Empty);
  }

  /// Alternative version of insert that uses a different (and possibly less
  /// expensive) key type.
  template <typename LookupKeyT>
  std::pair<iterator, bool> insert_as(const ValueT &V,
                                      const LookupKeyT &LookupKey) {
    return TheMap.insert_as({V, detail::DenseSetEmpty()}, LookupKey);
  }
  template <typename LookupKeyT>
  std::pair<iterator, bool> insert_as(ValueT &&V, const LookupKeyT &LookupKey) {
    return TheMap.insert_as({std::move(V), detail::DenseSetEmpty()}, LookupKey);
  }

  // Range insertion of values.
  template<typename InputIt>
  void insert(InputIt I, InputIt E) {
    for (; I != E; ++I)
      insert(*I);
  }
};

} // end namespace detail

/// Implements a dense probed hash-table based set.
template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT>>
class DenseSet : public detail::DenseSetImpl<
                     ValueT, DenseMap<ValueT, detail::DenseSetEmpty, ValueInfoT,
                                      detail::DenseSetPair<ValueT>>,
                     ValueInfoT> {
  using BaseT =
      detail::DenseSetImpl<ValueT,
                           DenseMap<ValueT, detail::DenseSetEmpty, ValueInfoT,
                                    detail::DenseSetPair<ValueT>>,
                           ValueInfoT>;

public:
  using BaseT::BaseT;
};

/// Implements a dense probed hash-table based set with some number of buckets
/// stored inline.
template <typename ValueT, unsigned InlineBuckets = 4,
          typename ValueInfoT = DenseMapInfo<ValueT>>
class SmallDenseSet
    : public detail::DenseSetImpl<
          ValueT, SmallDenseMap<ValueT, detail::DenseSetEmpty, InlineBuckets,
                                ValueInfoT, detail::DenseSetPair<ValueT>>,
          ValueInfoT> {
  using BaseT = detail::DenseSetImpl<
      ValueT, SmallDenseMap<ValueT, detail::DenseSetEmpty, InlineBuckets,
                            ValueInfoT, detail::DenseSetPair<ValueT>>,
      ValueInfoT>;

public:
  using BaseT::BaseT;
};

} // end namespace llvh

#endif // LLVM_ADT_DENSESET_H
