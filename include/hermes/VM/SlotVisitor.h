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

/// SlotVisitor is a way to call a function, called the acceptor, on each
/// special GC value inside an object that resides in the heap.
///
/// It finds the special values by using the object's type metadata which
/// describes where they will be located within an object.
namespace SlotVisitor {
namespace detail {
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
    if constexpr (WithNames) {
      char name[16];
      ::snprintf(name, sizeof(name), "%u", i);
      acceptor.accept(*reinterpret_cast<ElementType *>(start), name);
    } else {
      acceptor.accept(*reinterpret_cast<ElementType *>(start));
    }
    start += stride;
  }
}

/// Dispatches array information to be used by \c visitArrayObject
template <typename Acceptor, bool WithNames>
void visitArray(
    Acceptor &acceptor,
    char *base,
    const Metadata::ArrayData &array) {
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

/// Visit each individual slot within the object that starts at \p base.
/// \p offsets A list of offsets at which fields of type T can be found.
template <typename Acceptor, typename T>
void visitSlot(Acceptor &acceptor, char *const slot) {
  assert(
      reinterpret_cast<uintptr_t>(slot) % alignof(T) == 0 &&
      "Should be aligned to the same alignment as T");
  acceptor.accept(*reinterpret_cast<T *>(slot));
}

/// Same as \c visitArrayObject, except it only accepts fields between
/// [begin, end).
template <typename Acceptor, typename ElementType>
void visitArrayObjectWithinRange(
    Acceptor &acceptor,
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
    acceptor.accept(*reinterpret_cast<ElementType *>(start));
    start += stride;
  }
}

/// Same as \c visitArray, but forwards \p begin and \p end.
template <typename Acceptor>
void visitArrayWithinRange(
    Acceptor &acceptor,
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
#define SLOT_TYPE(type)                               \
  case ArrayType::type:                               \
    visitArrayObjectWithinRange<Acceptor, type>(      \
        acceptor, start, length, stride, begin, end); \
    break;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  }
}

template <typename Acceptor, typename T>
void visitSlotWithNames(Acceptor &acceptor, char *slot, const char *name) {
  assert(
      reinterpret_cast<uintptr_t>(slot) % alignof(T) == 0 &&
      "Should be aligned to the same alignment as T");
  acceptor.accept(*reinterpret_cast<T *>(slot), name);
}

} // namespace detail

/// Visit all the fields in an object and calls accept on them.
/// It knows where the acceptable fields are by reading from \p offsets.
/// It also visits all of the fields in variable sized objects like arrays.
/// \p acceptor The acceptor to use
/// \p cell The cell to be marked, any object in the GC heap.
/// \p offsets The metadata about the cell.
template <typename Acceptor>
void visit(
    Acceptor &acceptor,
    GCCell *cell,
    const Metadata::SlotOffsets &offsets) {
  auto *base = reinterpret_cast<char *>(cell);

  size_t i = 0;
#define SLOT_TYPE(type)              \
  for (; i < offsets.end##type; ++i) \
    detail::visitSlot<Acceptor, type>(acceptor, base + offsets.fields[i]);
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

  if (offsets.array)
    detail::visitArray<Acceptor, /* WithNames */ false>(
        acceptor, base, *offsets.array);
}

/// Like \c visit, but only invokes the acceptor on slots that begin within
/// [begin, end)
template <typename Acceptor>
void visitWithinRange(
    Acceptor &acceptor,
    GCCell *cell,
    const Metadata::SlotOffsets &offsets,
    const char *begin,
    const char *end) {
  auto *base = reinterpret_cast<char *>(cell);

  size_t i = 0;
#define SLOT_TYPE(type)                                \
  for (; i < offsets.end##type; ++i) {                 \
    char *slot = base + offsets.fields[i];             \
    if (slot < begin)                                  \
      continue;                                        \
    if (slot >= end) {                                 \
      i = offsets.end##type;                           \
      break;                                           \
    }                                                  \
    detail::visitSlot<Acceptor, type>(acceptor, slot); \
  }
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

  if (offsets.array)
    detail::visitArrayWithinRange(acceptor, base, *offsets.array, begin, end);
}

/// Like \c visit, but invokes the acceptor with names.
template <typename Acceptor>
void visitWithNames(
    Acceptor &acceptor,
    GCCell *cell,
    const Metadata::SlotOffsets &offsets,
    const Metadata::SlotNames &names) {
  auto *base = reinterpret_cast<char *>(cell);

  // Ignore sizes for special fields, since these are known types with known
  // sizes.
  size_t i = 0;
#define SLOT_TYPE(type)                         \
  for (; i < offsets.end##type; ++i)            \
    detail::visitSlotWithNames<Acceptor, type>( \
        acceptor, base + offsets.fields[i], names[i]);
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

  if (offsets.array)
    detail::visitArray<Acceptor, /* WithNames */ true>(
        acceptor, base, *offsets.array);
}

} // namespace SlotVisitor
} // namespace vm
} // namespace hermes

#endif
