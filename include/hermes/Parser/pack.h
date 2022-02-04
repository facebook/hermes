/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_PACK_H
#define HERMES_PARSER_PACK_H

namespace hermes {
namespace parser {

/// This contraption allows us to allocate an object and an array of values
/// together and provides access to the values from the object pointer.
/// We have to jump through significant hoops to avoid the "non-standard layout
/// offsetof()" warning.
template <class Obj, typename Value>
struct Pack {
  struct Pack1 {
    // Here are forced to work around bogus compiler warnings about offsetof().
    // Even though the layout of Obj not be known (in theory), the layout of
    // this struct is!
    char obj[sizeof(Obj)];
    Value values[1];
  };

  // This second struct needed for correct alignment determination.
  struct Pack2 {
    Obj obj;
    Value values[1];
  };

  static_assert(
      sizeof(Pack1) == sizeof(Pack2),
      "Pack1 and Pack2 must have the same size");

  static inline Value *values(Obj *obj) {
    return reinterpret_cast<Value *>(
        reinterpret_cast<char *>(obj) + offsetof(Pack1, values));
  }
  static inline Value const *values(const Obj *obj) {
    return reinterpret_cast<Value const *>(
        reinterpret_cast<const char *>(obj) + offsetof(Pack1, values));
  }
  template <class Allocator>
  static inline void *allocate(Allocator &alloc, size_t count) {
    return alloc.Allocate(
        offsetof(Pack1, values) + sizeof(Value) * count, alignof(Pack2));
  }
};

} // namespace parser
} // namespace hermes

#endif // HERMES_PARSER_PACK_H
