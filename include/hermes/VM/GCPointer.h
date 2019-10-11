/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCPOINTER_H
#define HERMES_VM_GCPOINTER_H

#include "hermes/Support/Compiler.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/PointerBase.h"

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace hermes {
namespace vm {

class GCPointerBase {
 public:
  using StorageType =
#ifdef HERMESVM_COMPRESSED_POINTERS
      BasedPointer;
#else
      void *;
#endif

 protected:
  StorageType ptr_;

  explicit GCPointerBase(std::nullptr_t) : ptr_() {}
  inline GCPointerBase(PointerBase *base, void *ptr);

 public:
  // These classes are used as arguments to GCPointer constructors, to
  // indicate whether write barriers are necessary in initializing the
  // GCPointer.
  class NoBarriers : public std::false_type {};
  class YesBarriers : public std::true_type {};

  void *get(PointerBase *base) const;

  void *getNonNull(PointerBase *base) const;

  /// This must be used to assign a new value to this GCPointer.
  /// \param ptr The memory being pointed to.
  /// \param base The base of ptr.
  /// \param gc Used for write barriers.
  void set(PointerBase *base, void *ptr, GC *gc);

  /// Get the underlying StorageType representation.
  StorageType getStorageType() const;

  /// Get the location of the pointer. Should only be used within the
  /// implementation of garbage collection.
  /// \param gc Used to assert that this is only used within the implementation
  ///   of garbage collection.
  StorageType &getLoc(GC *gc);

  explicit operator bool() const {
    return static_cast<bool>(ptr_);
  }

  bool operator==(const GCPointerBase &other) const {
    return ptr_ == other.ptr_;
  }

  bool operator!=(const GCPointerBase &other) const {
    return !(*this == other);
  }

  static void *storageTypeToPointer(StorageType st, PointerBase *base) {
#ifdef HERMESVM_COMPRESSED_POINTERS
    return base->basedToPointer(st);
#else
    return st;
#endif
  }
};

/// A class to represent "raw" pointers to heap objects.  Disallows assignment,
/// requiring a method that takes a GC* to perform a write barrier.
template <typename T>
class GCPointer : public GCPointerBase {
 public:
  /// Default method stores nullptr.  No barrier necessary.
  GCPointer() : GCPointerBase(nullptr) {}
  /// Explicit construct for the nullptr type.  No barrier necessary.
  GCPointer(std::nullptr_t null) : GCPointerBase(nullptr) {}

  /// Other constructors may need to perform barriers, using the \p gc argument,
  /// as indicated by the \p needsBarrier argument.  (The value of
  /// this argument is unused, but its type's boolean value constant indicates
  /// whether barriers are required.)
  template <typename NeedsBarriers>
  GCPointer(
      PointerBase *base,
      T *ptr,
      GC *gc,
      NeedsBarriers needsBarriersUnused);

  /// Same as the constructor above, with the default for
  /// NeedsBarriers as "YesBarriers".  (We can't use default template
  /// arguments with the idiom used above.)
  inline GCPointer(PointerBase *base, T *ptr, GC *gc)
      : GCPointer<T>(base, ptr, gc, YesBarriers()) {}

  /// We are not allowed to copy-construct or assign GCPointers.
  GCPointer(const GCPointerBase &) = delete;
  GCPointer &operator=(const GCPointerBase &) = delete;
  GCPointer &operator=(const GCPointer<T> &) = delete;

  /// We can assign null without a barrier.  NB: this is true for the
  /// generational collection remembered-set write barrier.  It may not be
  /// true for other possible collectors.  (For example, a
  /// snapshot-at-the-beginning concurrent collector.)
  GCPointer &operator=(std::nullptr_t) {
    ptr_ = StorageType{};
    return *this;
  }

  /// Get the raw pointer value.
  /// \param base The base of the address space that the GCPointer points into.
  T *get(PointerBase *base) const {
    return static_cast<T *>(GCPointerBase::get(base));
  }
  T *getNonNull(PointerBase *base) const {
    return static_cast<T *>(GCPointerBase::getNonNull(base));
  }

  /// Assign a new value to this GCPointer.
  /// \param base The base of ptr.
  /// \param ptr The memory being pointed to.
  /// \param gc Used for write barriers.
  void set(PointerBase *base, T *ptr, GC *gc) {
    GCPointerBase::set(base, ptr, gc);
  }

  /// Convenience overload of GCPointer::set for other GCPointers.
  void set(PointerBase *base, const GCPointer<T> &ptr, GC *gc) {
    set(base, ptr.get(base), gc);
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_H
