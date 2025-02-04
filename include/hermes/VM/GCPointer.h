/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCPOINTER_H
#define HERMES_VM_GCPOINTER_H

#include "hermes/Support/Compiler.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCDecl.h"

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace hermes {
namespace vm {

class GCPointerBase : public CompressedPointer {
 protected:
  explicit GCPointerBase(std::nullptr_t) : CompressedPointer(nullptr) {}

  template <typename NeedsBarriers>
  inline GCPointerBase(PointerBase &base, GCCell *ptr, GC &gc, NeedsBarriers);

  inline GCPointerBase(
      PointerBase &base,
      GCCell *ptr,
      GC &gc,
      const GCCell *owningObj);

 public:
  // These classes are used as arguments to GCPointer constructors, to
  // indicate whether write barriers are necessary in initializing the
  // GCPointer.
  class NoBarriers : public std::false_type {};
  class YesBarriers : public std::true_type {};

  /// This must be used to assign a new value to this GCPointer. This must not
  /// be used if it lives in an object that supports large allocation.
  /// \param ptr The memory being pointed to.
  /// \param base The base of ptr.
  /// \param gc Used for write barriers.
  inline void set(PointerBase &base, GCCell *ptr, GC &gc);
  inline void set(PointerBase &base, CompressedPointer ptr, GC &gc);
  inline void setNonNull(PointerBase &base, GCCell *ptr, GC &gc);

  /// This must be used to assign a new value to this GCPointer, which lives in
  /// an object of kind that supports large allocation.
  /// \param ptr The memory being pointed to.
  /// \param base The base of ptr.
  /// \param gc Used for write barriers.
  /// \param owningObj The object that contains this GCPointer, used by the
  /// writer barriers.
  inline void setInLargeObj(
      PointerBase &base,
      GCCell *ptr,
      GC &gc,
      const GCCell *owningObj);
  inline void setInLargeObj(
      PointerBase &base,
      CompressedPointer ptr,
      GC &gc,
      const GCCell *owningObj);
  inline void setNonNullInLargeObj(
      PointerBase &base,
      GCCell *ptr,
      GC &gc,
      const GCCell *owningObj);

  /// Set this pointer to null. This needs a write barrier in some types of
  /// garbage collectors.
  inline void setNull(GC &gc);
};

/// A class to represent "raw" pointers to heap objects.  Disallows assignment,
/// requiring a method that takes a GC* to perform a write barrier.
template <typename T>
class GCPointer : public GCPointerBase {
 public:
  /// Default method stores nullptr.  No barrier necessary.
  GCPointer() : GCPointerBase(nullptr) {}
  /// Explicit construct for the nullptr type.  No barrier necessary.
  GCPointer(std::nullptr_t) : GCPointerBase(nullptr) {}

  /// Other constructors may need to perform barriers, using the \p gc argument,
  /// as indicated by the \p needsBarriers argument.  (The value of
  /// this argument is unused, but its type's boolean value constant indicates
  /// whether barriers are required.)
  template <typename NeedsBarriers>
  GCPointer(PointerBase &base, T *ptr, GC &gc, NeedsBarriers needsBarriers)
      : GCPointerBase(base, ptr, gc, needsBarriers) {}
  /// Pass the owning object pointer to perform barriers when the object
  /// supports large allocation.
  template <typename NeedsBarriers>
  GCPointer(
      PointerBase &base,
      T *ptr,
      GC &gc,
      const GCCell *owningObj,
      NeedsBarriers needsBarriers)
      : GCPointerBase(base, ptr, gc, owningObj, needsBarriers) {}

  /// Same as the constructor above, with the default for
  /// NeedsBarriers as "YesBarriers".  (We can't use default template
  /// arguments with the idiom used above.)
  GCPointer(PointerBase &base, T *ptr, GC &gc)
      : GCPointer<T>(base, ptr, gc, YesBarriers()) {}
  /// Pass the owning object pointer to perform barriers when the object
  /// supports large allocation.
  GCPointer(PointerBase &base, T *ptr, GC &gc, const GCCell *owningObj)
      : GCPointer<T>(base, ptr, gc, owningObj, YesBarriers()) {}

  /// We are not allowed to copy-construct or assign GCPointers.
  GCPointer(const GCPointerBase &) = delete;
  GCPointer &operator=(const GCPointerBase &) = delete;
  GCPointer(const GCPointer<T> &) = delete;
  GCPointer &operator=(const GCPointer<T> &) = delete;

  /// Get the raw pointer value.
  /// \param base The base of the address space that the GCPointer points into.
  T *get(PointerBase &base) const {
    return vmcast_or_null<T>(GCPointerBase::get(base));
  }
  T *getNonNull(PointerBase &base) const {
    return vmcast<T>(GCPointerBase::getNonNull(base));
  }

  /// Assign a new value to this GCPointer. This must not be used if it lives in
  /// an object that supports large allocation.
  /// \param base The base of ptr.
  /// \param ptr The memory being pointed to.
  /// \param gc Used for write barriers.
  void set(PointerBase &base, T *ptr, GC &gc) {
    GCPointerBase::set(base, ptr, gc);
  }
  void setNonNull(PointerBase &base, T *ptr, GC &gc) {
    GCPointerBase::setNonNull(base, ptr, gc);
  }

  /// Assign a new value to this GCPointer, which lives in an object of kind
  /// that supports large allocation.
  /// \param base The base of ptr.
  /// \param ptr The memory being pointed to.
  /// \param gc Used for write barriers.
  /// \param owningObj The object that contains this GCPointer, used by the
  /// writer barriers.
  void
  setInLargeObj(PointerBase &base, T *ptr, GC &gc, const GCCell *owningObj) {
    GCPointerBase::set(base, ptr, gc, owningObj);
  }
  void setNonNullInLargeObj(
      PointerBase &base,
      T *ptr,
      GC &gc,
      const GCCell *owningObj) {
    GCPointerBase::setNonNull(base, ptr, gc, owningObj);
  }

  /// Convenience overload of GCPointer::set for other GCPointers. This must not
  /// be used if it lives in an object that supports large allocation.
  void set(PointerBase &base, const GCPointer<T> &ptr, GC &gc) {
    GCPointerBase::set(base, ptr, gc);
  }

  /// Convenience overload of GCPointer::set for other GCPointers. \p owningObj
  /// is used by the writer barriers.
  void setInLargeObj(
      PointerBase &base,
      const GCPointer<T> &ptr,
      GC &gc,
      const GCCell *owningObj) {
    GCPointerBase::set(base, ptr, gc, owningObj);
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_H
