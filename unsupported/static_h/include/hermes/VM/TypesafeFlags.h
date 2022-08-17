/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**===----------------------------------------------------------------------===
\file
This header defines helper macros for creating type-safe flags classes.
Instead of OR-ing integer constants, it declares a class with getters,
setters and a "fluent" builder API.

Users are expected to define a macro listing the names of all flags, like
in this example:

\code
#define LIST_MY_FLAGS(FLAG) \
   FLAG(Flag1) \
   FLAG(Flag2) \
   FLAG(Flag3)
\endcode

Declaring the actual class works like this:

\code
HERMES_VM__DECLARE_FLAGS_CLASS(MyFlags, LIST_MY_FLAGS);
\endcode

The macro above declares a class that initializes all flags to false on
construction and offers the following methods for every flag:

\code
 bool getFLAGNAME() const;
 ClassName setFLAGNAME(bool);
 ClassName plusFLAGNAME();
 ClassName minusFLAGNAME();
\endcode

The modifier methods implement a "fluent" API where they return a new
instance of the class with the corresponding flag set or cleared. This
pattern is especially convenient when constructing combinations of flags to
pass to functions.

Given the declarations for MyFlags above, this is some example usage:

\code
MyFlags flagsA{};
flagsA.setFlag1(true);
functionCall(flagsA.plusFlag2().minusFlag1().plusFlag3());
\endcode
*/
//===-----------------------------------------------------------------------===

#ifndef HERMES_VM_TYPESAFEFLAGS_H
#define HERMES_VM_TYPESAFEFLAGS_H

#define _HERMES_VM__DECL_FLAG(name) bool f##name##_ : 1;
#define _HERMES_VM__IMPL_FLAG(name) \
  Self plus##name() const {         \
    auto r(*this);                  \
    r.f##name##_ = true;            \
    return r;                       \
  }                                 \
  Self minus##name() const {        \
    auto r(*this);                  \
    r.f##name##_ = false;           \
    return r;                       \
  }                                 \
  Self set##name(bool v) const {    \
    auto r(*this);                  \
    r.f##name##_ = v;               \
    return r;                       \
  }                                 \
  bool get##name() const {          \
    return f##name##_;              \
  }

#define HERMES_VM__DECLARE_FLAGS_CLASS(ClassName, listMacro) \
  union ClassName {                                          \
   private:                                                  \
    struct {                                                 \
      listMacro(_HERMES_VM__DECL_FLAG)                       \
    };                                                       \
    unsigned flags_ = 0;                                     \
                                                             \
   public:                                                   \
    typedef ClassName Self;                                  \
    unsigned getRaw() const {                                \
      return flags_;                                         \
    }                                                        \
    listMacro(_HERMES_VM__IMPL_FLAG)                         \
  }

#endif // HERMES_VM_TYPESAFEFLAGS_H
