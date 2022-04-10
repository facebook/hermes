/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_NATIVEARGS_H
#define HERMES_VM_NATIVEARGS_H

#include "hermes/VM/Handle.h"
#include "hermes/VM/HandleRootOwner.h"

namespace hermes {
namespace vm {

template <bool isConst>
using ArgIteratorT = std::reverse_iterator<typename std::conditional<
    isConst,
    const PinnedHermesValue *,
    PinnedHermesValue *>::type>;
using ArgIterator = ArgIteratorT<false>;
using ConstArgIterator = ArgIteratorT<true>;

/// An instance of this object is passed to native functions to enable them
/// to access their arguments.
class NativeArgs final {
  /// Point to the first explicit argument in the stack. 'this' can be accessed
  /// at firstArg_[-1].
  const ConstArgIterator firstArg_;
  /// The number of JavaScript arguments excluding 'this'.
  unsigned const argCount_;
  /// The \c new.target value of the call.
  const PinnedHermesValue *const newTarget_;

  /// \param firstArg is an iterator to ("arg0", ... "argN").
  /// \param argCount number of JavaScript arguments excluding 'this'
  /// \param newTarget points to the value of \c new.target in the stack.
  NativeArgs(
      ConstArgIterator firstArg,
      unsigned argCount,
      const PinnedHermesValue *newTarget)
      : firstArg_(firstArg), argCount_(argCount), newTarget_(newTarget) {}

  template <bool isConst>
  friend class StackFramePtrT;

 public:
  /// \return the value of \c new.target. It is \c undefined if this is a
  ///   regular function call, of the callable of the constructor invoked by
  ///   \c new otherwise.
  const PinnedHermesValue &getNewTarget() const {
    return *newTarget_;
  }

  /// \return the value of \c new.target. It is \c undefined if this is a
  ///   regular function call, of the callable of the constructor invoked by
  ///   \c new otherwise.
  Handle<> getNewTargetHandle() const {
    return Handle<>(newTarget_);
  }

  /// \return true if this is a function call.
  bool isFunctionCall() const {
    return newTarget_->isUndefined();
  }
  /// \return true if this is a constructor invocation.
  bool isConstructorCall() const {
    return !isFunctionCall();
  }

  /// \return the number of arguments passed to the function, excluding 'this'.
  unsigned getArgCount() const {
    return argCount_;
  }

  /// \return the 'this' argument passed to the function.
  const PinnedHermesValue &getThisArg() const {
    return begin()[-1];
  }

  /// Efficiently wrap the 'this' argument into a Handle<>
  Handle<> getThisHandle() const {
    return Handle<>(&getThisArg());
  }

  /// If 'thisArg' is an instance of T, wrap it into a Handle<T> and return
  /// it, otherwise return Handle<>(nullptr).
  template <class T>
  Handle<T> dyncastThis() const {
    return vmisa<T>(getThisArg()) ? Handle<T>::vmcast(&getThisArg())
                                  : HandleRootOwner::makeNullHandle<T>();
  }

  // Assert that 'thisArg' is an instance of T and return it as a Handle.
  template <class T>
  Handle<T> vmcastThis() const {
    return Handle<T>::vmcast(&getThisArg());
  }

  /// \return the specified argument by index starting from 0 (and excluding
  ///   'this'). If there is no such argument, return 'undefined'.
  HermesValue getArg(unsigned index) const {
    return index < argCount_ ? static_cast<HermesValue>(begin()[index])
                             : HermesValue::encodeUndefinedValue();
  }

  /// Efficiently wrap the specified argument by index starting from 0 (and
  /// excluding 'this') into a Handle<>. If there is no such argument,
  /// 'undefined' is wrapped.
  Handle<> getArgHandle(unsigned index) const {
    return index < argCount_ ? Handle<>(&begin()[index])
                             : HandleRootOwner::getUndefinedValue();
  }

  /// If argument with index \p index (starting from 0 and excluding `thisArg`)
  /// is present is and is a pointer to type T, return it as a Handle<T>,
  /// otherwise return a Handle<T>(nullptr).
  /// This is a very efficient operation - it never allocates a new handle.
  template <class T>
  Handle<T> dyncastArg(unsigned index) const {
    return index < argCount_ && vmisa<T>(begin()[index])
        ? Handle<T>::vmcast(&begin()[index])
        : HandleRootOwner::makeNullHandle<T>();
  }

  /// An iterator returning Handle<>() for each argument. Since arguments
  /// are pinned on the stack, it can be done efficiently. This is a random
  /// access iterator, so it can even be used to access arguments directly by
  /// index, but only if we are sure that it is a valid argument, for example
  /// like this:
  /// \code
  ///   if (args.getArgCount() < 2)
  ///     return error...;
  ///   auto argsHandles = args.handles().begin();
  ///   auto arg0 = argsHandles[0];
  ///   auto arg1 = argsHandles[1];
  /// \endcode
  class handle_iterator {
    friend class NativeArgs;

    ConstArgIterator arg_;

    explicit handle_iterator(ConstArgIterator arg) : arg_(arg) {}

   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Handle<>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    handle_iterator(const handle_iterator &) = default;
    handle_iterator &operator=(const handle_iterator &) = default;

    bool operator!=(const handle_iterator &o) const {
      return arg_ != o.arg_;
    }
    bool operator==(const handle_iterator &o) const {
      return arg_ == o.arg_;
    }

    Handle<> operator*() const {
      return Handle<>(&*arg_);
    }
    Handle<> operator[](difference_type o) const {
      return Handle<>(&*(arg_ + o));
    }
    const HermesValue *operator->() const {
      return &*arg_;
    }

    handle_iterator &operator++() {
      ++arg_;
      return *this;
    }
    handle_iterator &operator--() {
      --arg_;
      return *this;
    }
    handle_iterator operator++(int) {
      auto res = *this;
      ++arg_;
      return res;
    }
    handle_iterator operator--(int) {
      auto res = *this;
      --arg_;
      return res;
    }
    handle_iterator &operator+=(difference_type o) {
      arg_ += o;
      return *this;
    }
    handle_iterator &operator-=(difference_type o) {
      arg_ -= o;
      return *this;
    }

    handle_iterator operator+(difference_type o) const {
      return handle_iterator(arg_ + o);
    }
    handle_iterator operator-(difference_type o) const {
      return handle_iterator(arg_ - o);
    }
    difference_type operator-(const handle_iterator x) const {
      return arg_ - x.arg_;
    }
  };

  /// Return a range for iterator over Handle<> instead of PinnedHermesValue.
  /// The usual usage pattern is:
  /// \code
  ///   for (Handle<> arg : args.handles()) {
  ///     ...;
  ///   }
  /// \endcode
  llvh::iterator_range<handle_iterator> handles() const {
    return llvh::make_range(handle_iterator(begin()), handle_iterator(end()));
  };

  ConstArgIterator begin() const {
    return firstArg_;
  }
  ConstArgIterator end() const {
    return begin() + argCount_;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_NATIVEARGS_H
