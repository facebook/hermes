/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SLOTVISITOR_H
#define HERMES_VM_SLOTVISITOR_H

#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/Metadata.h"

namespace hermes {
namespace vm {

/// BaseVisitor contains any common functionality between multiple visitors.
class BaseVisitor {
 protected:
  /// Dispatches array information to be used by \c visitArrayObject
  template <typename Acceptor, bool WithNames>
  void
  visitArray(Acceptor &acceptor, char *base, const Metadata::ArrayData &array) {
    using ArrayType = Metadata::ArrayData::ArrayType;
    char *start = base + array.startOffset;
    // Load the length, making sure all dependent writes have completed with
    // memory_order_acquire.
    const auto length = reinterpret_cast<AtomicIfConcurrentGC<std::uint32_t> *>(
                            base + array.lengthOffset)
                            ->load(std::memory_order_acquire);
    const auto stride = array.stride;
    switch (array.type) {
#define SLOT_TYPE(type)                          \
  case ArrayType::type:                          \
    visitArrayObject<Acceptor, type, WithNames>( \
        acceptor, start, length, stride);        \
    break;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
    }
  }

 private:
  /// Partially-specializable function wrapper for deciding between supplying
  /// the name or not.
  template <typename Acceptor, typename ElementType, bool WithName>
  struct ArrayElementAccept final {
    static void impl(Acceptor &acceptor, ElementType &elem, uint32_t num);
  };

  template <typename Acceptor, typename ElementType>
  struct ArrayElementAccept<Acceptor, ElementType, true> final {
    static void impl(Acceptor &acceptor, ElementType &elem, uint32_t num) {
      auto name = std::to_string(num);
      // It's fine to use the raw string since it is not stored, only copied.
      acceptor.accept(elem, name.c_str());
    }
  };

  template <typename Acceptor, typename ElementType>
  struct ArrayElementAccept<Acceptor, ElementType, false> final {
    static void impl(Acceptor &acceptor, ElementType &elem, uint32_t) {
      acceptor.accept(elem);
    }
  };

  /// Visits the array fields in an object.
  ///
  /// ElementType is the type of element that the array contains.
  /// WithNames is true if the caller wants to pass the array index as a name
  ///   to the acceptor.
  /// \p start The start of the array field of the object.
  /// \p length The length of the array to be marked.
  /// \p stride The width of each element of the array.
  template <typename Acceptor, typename ElementType, bool WithNames>
  void visitArrayObject(
      Acceptor &acceptor,
      char *start,
      std::uint32_t length,
      std::size_t stride) {
    for (std::uint32_t i = 0; i < length; ++i) {
      ArrayElementAccept<Acceptor, ElementType, WithNames>::impl(
          acceptor, *reinterpret_cast<ElementType *>(start), i);
      start += stride;
    }
  }
};

/// SlotVisitor is a way to call a function, called the acceptor, on each
/// special GC value inside an object that resides in the heap.
///
/// It finds the special values by using the object's type metadata which
/// describes where they will be located within an object.
/// NOTE: for best performance, the SlotVisitor's visit methods should be called
///   in the same compilation unit in which the the SlotVisitor is constructed.
///   This will increase the likelihood of inlining.
template <typename Acceptor>
struct SlotVisitor final : BaseVisitor {
  Acceptor &acceptor_;

  /// Creates a new SlotVisitor, which can iterate over the fields of an object
  /// and calls \c acceptor.accept on them.
  /// \p acceptor A function object which can be called with the various types
  ///   of acceptable fields within an object. Should adhere to the interface
  ///   given by \c SlotAcceptor.
  SlotVisitor(Acceptor &acceptor) : acceptor_(acceptor) {}

  /// Visit all the fields in an object and calls accept on them.
  /// It knows where the acceptable fields are by reading from \p meta.
  /// It also visits all of the fields in variable sized objects like arrays.
  /// \p cell The cell to be marked, any object in the GC heap.
  /// \p meta The metadata about the cell.
  void visit(GCCell *cell, const Metadata::SlotOffsets &offsets) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFields(ptr, offsets);
    if (offsets.array) {
      visitArray<Acceptor, /*WithNames*/ false>(acceptor_, ptr, *offsets.array);
    }
  }

  /// Like \c visit, but only invokes the acceptor on slots that begin within
  /// [begin, end)
  void visitWithinRange(
      GCCell *cell,
      const Metadata::SlotOffsets &offsets,
      const char *begin,
      const char *end) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFieldsWithinRange(ptr, offsets, begin, end);
    if (offsets.array) {
      visitArrayWithinRange(ptr, *offsets.array, begin, end);
    }
  }

 private:
  /// Visit each individual slot within the object that starts at \p base.
  /// \p offsets A list of offsets at which fields of type T can be found.
  template <typename T>
  void visitSlot(char *const slot) {
    assert(
        reinterpret_cast<uintptr_t>(slot) % alignof(T) == 0 &&
        "Should be aligned to the same alignment as T");
    acceptor_.accept(*reinterpret_cast<T *>(slot));
  }

  /// Visits the fields of an object that starts at \p base, using \p meta to
  /// find the fields, and calls \c acceptor_.accept() on them.
  void visitFields(char *base, const Metadata::SlotOffsets &offsets) {
    size_t i = 0;
#define SLOT_TYPE(type)              \
  for (; i < offsets.end##type; ++i) \
    visitSlot<type>(base + offsets.fields[i]);
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  }

  /// Same as \c visitFields, except any pointer field inside \p base that is
  /// not between \p begin and \p end is skipped.
  void visitFieldsWithinRange(
      char *base,
      const Metadata::SlotOffsets &offsets,
      const char *begin,
      const char *end) {
    size_t i = 0;
#define SLOT_TYPE(type)                    \
  for (; i < offsets.end##type; ++i) {     \
    char *slot = base + offsets.fields[i]; \
    if (slot < begin)                      \
      continue;                            \
    if (slot >= end) {                     \
      i = offsets.end##type;               \
      break;                               \
    }                                      \
    visitSlot<type>(slot);                 \
  }
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  }

  /// Same as \c visitArrayObject, except it does not accept fields between
  /// [begin, end).
  template <typename ElementType>
  void visitArrayObjectWithinRange(
      char *start,
      std::size_t length,
      std::size_t stride,
      const char *begin,
      const char *end) {
    // start is the beginning of the array, begin is where the dirty card
    // begins, end is where the dirty card ends.
    const char *const endOfObject = start + length * stride;
    char *const alignedStartOfCard =
        const_cast<char *>(begin - (begin - start) % stride);
    start = std::max(start, alignedStartOfCard);
    end = std::min(endOfObject, end);
    while (start < end) {
      // Selects which type to cast to based on the type of array given.
      acceptor_.accept(*reinterpret_cast<ElementType *>(start));
      start += stride;
    }
  }

  /// Same as \c visitArray, but forwards \p begin and \p end.
  void visitArrayWithinRange(
      char *base,
      const Metadata::ArrayData &array,
      const char *begin,
      const char *end) {
    using ArrayType = Metadata::ArrayData::ArrayType;
    char *start = base + array.startOffset;
    const auto length =
        *reinterpret_cast<std::uint32_t *>(base + array.lengthOffset);
    const auto stride = array.stride;
    switch (array.type) {
#define SLOT_TYPE(type)                                                   \
  case ArrayType::type:                                                   \
    visitArrayObjectWithinRange<type>(start, length, stride, begin, end); \
    break;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
    }
  }
};

template <typename Acceptor>
struct SlotVisitorWithNames final : BaseVisitor {
  Acceptor &acceptor_;

  SlotVisitorWithNames(Acceptor &acceptor) : acceptor_(acceptor) {}

  void visit(
      GCCell *cell,
      const Metadata::SlotOffsets &offsets,
      const Metadata::SlotNames &names) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFields(ptr, offsets, names);
    if (offsets.array) {
      visitArray<Acceptor, /*WithNames*/ true>(acceptor_, ptr, *offsets.array);
    }
  }

 private:
  /// Visits the fields of an object that starts at \p base, using \p meta to
  /// find the fields, and calls \c acceptor_.accept() on them.
  void visitFields(
      char *base,
      const Metadata::SlotOffsets &offsets,
      const Metadata::SlotNames &names) {
    // Ignore sizes for special fields, since these are known types with known
    // sizes.
    size_t i = 0;
#define SLOT_TYPE(type)              \
  for (; i < offsets.end##type; ++i) \
    visitSlot<type>(base + offsets.fields[i], names[i]);
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  }

  template <typename T>
  void visitSlot(char *slot, const char *name) {
    assert(
        reinterpret_cast<uintptr_t>(slot) % alignof(T) == 0 &&
        "Should be aligned to the same alignment as T");
    acceptor_.accept(*reinterpret_cast<T *>(slot), name);
  }
};

} // namespace vm
} // namespace hermes

#endif
