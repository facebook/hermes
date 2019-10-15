/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

/// An instance of this object is passed to native functions to enable them
/// to access their arguments.
class NativeArgs final {
  /// Point to the first argument in the stack, which is 'this'.
  const PinnedHermesValue *const thisArg_;
  /// The number of JavaScript arguments excluding 'this'.
  unsigned const argCount_;
  /// The \c new.target value of the call.
  const PinnedHermesValue *const newTarget_;

  /// Return a pointer to argument with index \p index, starting from 0 and
  /// excluding 'thisArg'.
  /// NOTE: this function deliberately doesn't perform a range check (even in
  /// debug builds). It is always used only in the context of an existing range
  /// check, or to obtain an iterator (which should always be compared against
  /// the 'end' iterator before using. More importantly, if we had a range check
  /// here, we wouldn't be able to obtain an iterator to an empty argument list.
  const PinnedHermesValue *argPtr(unsigned index) const {
    return thisArg_ + index + 1;
  }

  /// \param points to "this", "arg0", ... "argN").
  /// \param argCount number of JavaScript arguments excluding 'this'
  /// \param newTarget points to the value of \c new.target in the stack.
  NativeArgs(
      const PinnedHermesValue *thisArg,
      unsigned argCount,
      const PinnedHermesValue *newTarget)
      : thisArg_(thisArg), argCount_(argCount), newTarget_(newTarget) {}

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
    return *thisArg_;
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
    return index < argCount_ ? static_cast<HermesValue>(*argPtr(index))
                             : HermesValue::encodeUndefinedValue();
  }

  /// Efficiently wrap the specified argument by index starting from 0 (and
  /// excluding 'this') into a Handle<>. If there is no such argument,
  /// 'undefined' is wrapped.
  Handle<> getArgHandle(unsigned index) const {
    return index < argCount_ ? Handle<>(argPtr(index))
                             : HandleRootOwner::getUndefinedValue();
  }

  /// If argument with index \p index (starting from 0 and excluding `thisArg`)
  /// is present is and is a pointer to type T, return it as a Handle<T>,
  /// otherwise return a Handle<T>(nullptr).
  /// This is a very efficient operation - it never allocates a new handle.
  template <class T>
  Handle<T> dyncastArg(unsigned index) const {
    return index < argCount_ && vmisa<T>(*argPtr(index))
        ? Handle<T>::vmcast(argPtr(index))
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
  class handle_iterator
      : public std::iterator<std::random_access_iterator_tag, Handle<>> {
    friend class NativeArgs;

    const PinnedHermesValue *arg_;

    explicit handle_iterator(PinnedHermesValue const *arg) : arg_(arg) {}

   public:
    handle_iterator(const handle_iterator &) = default;
    handle_iterator &operator=(const handle_iterator &) = default;

    bool operator!=(const handle_iterator &o) const {
      return arg_ != o.arg_;
    }
    bool operator==(const handle_iterator &o) const {
      return arg_ == o.arg_;
    }

    Handle<> operator*() const {
      return Handle<>(arg_);
    }
    Handle<> operator[](difference_type o) const {
      return Handle<>(arg_ + o);
    }
    const HermesValue *operator->() const {
      return arg_;
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
  llvm::iterator_range<handle_iterator> handles() const {
    return llvm::make_range(handle_iterator(begin()), handle_iterator(end()));
  };

  using const_iterator = const PinnedHermesValue *;

  const_iterator begin() const {
    return argPtr(0);
  }
  const_iterator end() const {
    return argPtr(0) + argCount_;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_NATIVEARGS_H
