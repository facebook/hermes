/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_HALFPAIRITERATOR_H
#define HERMES_ADT_HALFPAIRITERATOR_H

#include <iterator>

namespace hermes {

/// Declare iterators over a single field in a tuple.
#define HERMES_DECLARE_PARTIAL_ITERATOR(Field, FIELD)                          \
  template <class IT>                                                          \
  class Pair##Field##Iterator {                                                \
    IT it_;                                                                    \
                                                                               \
   public:                                                                     \
    using iterator_category = std::input_iterator_tag;                         \
    using value_type = decltype(std::iterator_traits<IT>::value_type::FIELD);  \
    using difference_type = std::ptrdiff_t;                                    \
    using pointer = value_type *;                                              \
    using reference = value_type &;                                            \
                                                                               \
    Pair##Field##Iterator(const IT &it) : it_(it) {}                           \
    Pair##Field##Iterator(const Pair##Field##Iterator &) = default;            \
    Pair##Field##Iterator &operator=(const Pair##Field##Iterator &) = default; \
                                                                               \
    Pair##Field##Iterator &operator++() {                                      \
      ++it_;                                                                   \
      return *this;                                                            \
    }                                                                          \
    bool operator==(const Pair##Field##Iterator &a) const {                    \
      return it_ == a.it_;                                                     \
    }                                                                          \
    bool operator!=(const Pair##Field##Iterator &a) const {                    \
      return it_ != a.it_;                                                     \
    }                                                                          \
                                                                               \
    decltype((*it_).FIELD) operator*() {                                       \
      return (*it_).FIELD;                                                     \
    }                                                                          \
                                                                               \
    ptrdiff_t operator-(const Pair##Field##Iterator &it) const {               \
      return std::distance(it_, it.it_);                                       \
    }                                                                          \
  };                                                                           \
  template <class IT>                                                          \
  Pair##Field##Iterator<IT> makePair##Field##Iterator(const IT &it) {          \
    return Pair##Field##Iterator<IT>(it);                                      \
  }

/// PairFirstIterator: iterate over pair::first.
HERMES_DECLARE_PARTIAL_ITERATOR(First, first);
/// PairSecondIterator: iterate over pair::second.
HERMES_DECLARE_PARTIAL_ITERATOR(Second, second);

} // namespace hermes

#endif // HERMES_ADT_HALFPAIRITERATOR_H
