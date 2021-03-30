/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
      case ArrayType::Pointer:
        visitArrayObject<Acceptor, GCPointerBase, WithNames>(
            acceptor, start, length, stride);
        break;
      case ArrayType::HermesValue:
        visitArrayObject<Acceptor, GCHermesValue, WithNames>(
            acceptor, start, length, stride);
        break;
      case ArrayType::SmallHermesValue:
        visitArrayObject<Acceptor, GCSmallHermesValue, WithNames>(
            acceptor, start, length, stride);
        break;
      case ArrayType::Symbol:
        visitArrayObject<Acceptor, GCSymbolID, WithNames>(
            acceptor, start, length, stride);
        break;
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
      auto name = oscompat::to_string(num);
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
  void visit(GCCell *cell, const Metadata &meta) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFields(ptr, meta);
    if (meta.array_) {
      visitArray<Acceptor, /*WithNames*/ false>(acceptor_, ptr, *meta.array_);
    }
  }

  /// Like \c visit, but only invokes the acceptor on slots that begin within
  /// [begin, end)
  void visitWithinRange(
      GCCell *cell,
      const Metadata &meta,
      const char *begin,
      const char *end) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFieldsWithinRange(ptr, meta, begin, end);
    if (meta.array_) {
      visitArrayWithinRange(ptr, *meta.array_, begin, end);
    }
  }

 private:
  /// Visit each individual slot within the object that starts at \p base.
  /// \p offsets A list of offsets at which fields of type T can be found.
  template <typename T>
  void visitSlots(
      char *const base,
      llvh::ArrayRef<Metadata::offset_t> offsets) {
    for (const auto offset : offsets) {
      assert(
          reinterpret_cast<uintptr_t>(base + offset) % alignof(T) == 0 &&
          "Should be aligned to the same alignment as T");
      acceptor_.accept(*reinterpret_cast<T *>(base + offset));
    }
  }

  /// Same as \c visitSlots, except it only visits slots that are between
  /// [begin, end).
  template <typename T>
  void visitSlotsWithinRange(
      char *base,
      llvh::ArrayRef<Metadata::offset_t> offsets,
      const char *begin,
      const char *end) {
    for (const auto offset : offsets) {
      char *curr = base + offset;
      assert(
          reinterpret_cast<uintptr_t>(curr) % alignof(T) == 0 &&
          "Should be aligned to the same alignment as T");
      if (curr >= begin && curr < end) {
        acceptor_.accept(*reinterpret_cast<T *>(curr));
      }
    }
  }

  /// Visits the fields of an object that starts at \p base, using \p meta to
  /// find the fields, and calls \c acceptor_.accept() on them.
  void visitFields(char *base, const Metadata &meta) {
    visitSlots<GCPointerBase>(base, meta.pointers_.offsets);
    visitSlots<GCHermesValue>(base, meta.values_.offsets);
    visitSlots<GCSmallHermesValue>(base, meta.smallValues_.offsets);
    visitSlots<GCSymbolID>(base, meta.symbols_.offsets);
  }

  /// Same as \c visitFields, except any pointer field inside \p base that is
  /// not between \p begin and \p end is skipped.
  void visitFieldsWithinRange(
      char *base,
      const Metadata &meta,
      const char *begin,
      const char *end) {
    visitSlotsWithinRange<GCPointerBase>(
        base, meta.pointers_.offsets, begin, end);
    visitSlotsWithinRange<GCHermesValue>(
        base, meta.values_.offsets, begin, end);
    visitSlotsWithinRange<GCSmallHermesValue>(
        base, meta.smallValues_.offsets, begin, end);
    visitSlotsWithinRange<GCSymbolID>(base, meta.symbols_.offsets, begin, end);
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
      case ArrayType::Pointer:
        visitArrayObjectWithinRange<GCPointerBase>(
            start, length, stride, begin, end);
        break;
      case ArrayType::HermesValue:
        visitArrayObjectWithinRange<GCHermesValue>(
            start, length, stride, begin, end);
        break;
      case ArrayType::SmallHermesValue:
        visitArrayObjectWithinRange<GCSmallHermesValue>(
            start, length, stride, begin, end);
        break;
      case ArrayType::Symbol:
        visitArrayObjectWithinRange<GCSymbolID>(
            start, length, stride, begin, end);
        break;
    }
  }
};

template <typename Acceptor>
struct SlotVisitorWithNames final : BaseVisitor {
  Acceptor &acceptor_;

  SlotVisitorWithNames(Acceptor &acceptor) : acceptor_(acceptor) {}

  void visit(GCCell *cell, const Metadata &meta) {
    auto *ptr = reinterpret_cast<char *>(cell);
    visitFields(ptr, meta);
    if (meta.array_) {
      visitArray<Acceptor, /*WithNames*/ true>(acceptor_, ptr, *meta.array_);
    }
  }

 private:
  /// Visits the fields of an object that starts at \p base, using \p meta to
  /// find the fields, and calls \c acceptor_.accept() on them.
  void visitFields(char *base, const Metadata &meta) {
    // Ignore sizes for special fields, since these are known types with known
    // sizes.
    visitSlots<GCPointerBase>(
        base, meta.pointers_.offsets, meta.pointers_.names);
    visitSlots<GCHermesValue>(base, meta.values_.offsets, meta.values_.names);
    visitSlots<GCSmallHermesValue>(
        base, meta.smallValues_.offsets, meta.smallValues_.names);
    visitSlots<GCSymbolID>(base, meta.symbols_.offsets, meta.symbols_.names);
  }

  template <typename T>
  void visitSlots(
      char *base,
      llvh::ArrayRef<Metadata::offset_t> offsets,
      llvh::ArrayRef<const char *> names) {
    for (decltype(offsets.size()) i = 0; i < offsets.size(); ++i) {
      char *curr = base + offsets[i];
      assert(
          reinterpret_cast<uintptr_t>(curr) % alignof(T) == 0 &&
          "Should be aligned to the same alignment as T");
      acceptor_.accept(*reinterpret_cast<T *>(curr), names[i]);
    }
  }
};

} // namespace vm
} // namespace hermes

#endif
