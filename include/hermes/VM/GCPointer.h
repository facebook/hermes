#ifndef HERMES_VM_GCPOINTER_H
#define HERMES_VM_GCPOINTER_H

#include "hermes/VM/GCDecl.h"

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace hermes {
namespace vm {

class GCPointerBase {
 protected:
  void *ptr_;

  GCPointerBase(void *ptr) : ptr_(ptr) {}

 public:
  // These classes are used as arguments to GCPointer constructors, to
  // indicate whether write barriers are necessary in initializing the
  // GCPointer.
  class NoBarriers : public std::false_type {};
  class YesBarriers : public std::true_type {};

  void *get() const {
    return ptr_;
  }
  /// This must be used to assign a new value to this GCPointer.
  /// The \p gc argument is used to perform barriers.
  inline void set(void *ptr, GC *gc);
  /// Get the location of the pointer.  Should only be used
  /// within the implementation of garbage collection; the \p gc
  /// argument is used to assert this requirement.
  inline void *&getLoc(GC *gc);
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
  inline GCPointer(T *ptr, GC *gc, NeedsBarriers needsBarriersUnused);

  /// Same as the constructor above, with the default for
  /// NeedsBarriers as "YesBarriers".  (We can't use default template
  /// arguments with the idiom used above.)
  inline GCPointer(T *ptr, GC *gc) : GCPointer<T>(ptr, gc, YesBarriers()) {}

  /// We are not allowed to copy-construct or assign GCPointers.
  GCPointer(const GCPointerBase &) = delete;
  GCPointer &operator=(const GCPointerBase &) = delete;

  /// Standard smart pointer operations.
  explicit operator bool() const {
    return ptr_;
  }
  T *operator->() const {
    return get();
  }
  T &operator*() const {
    return *get();
  }
  operator T *() const {
    return get();
  }
  T *get() const {
    return reinterpret_cast<T *>(ptr_);
  }

  /// We can assign null without a barrier.  NB: this is true for the
  /// generational collection remembered-set write barrier.  It may not be
  /// true for other possible collectors.  (For example, a
  /// snapshot-at-the-beginning concurrent collector.)
  GCPointer &operator=(std::nullptr_t null) {
    ptr_ = nullptr;
    return *this;
  }

  bool operator==(const GCPointer &other) const {
    return ptr_ == other.ptr_;
  }
  bool operator!=(const GCPointer &other) const {
    return !(*this == other);
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_H
