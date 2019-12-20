/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSNativeFunctions.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSDate.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSMapImpl.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/PrimitiveBox.h"

#include <limits>
namespace hermes {
namespace vm {

constexpr static int numFuncNames() {
  return 0
#define NATIVE_FUNCTION(func) +1
#define NATIVE_FUNCTION_TYPED(func, type) +1
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) +1
#define NATIVE_CONSTRUCTOR(func) +1
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) +1
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_FUNCTION_TYPED
#undef NATIVE_FUNCTION_TYPED_2
#undef NATIVE_CONSTRUCTOR
#undef NATIVE_CONSTRUCTOR_TYPED
      ;
}

#define NATIVE_FUNCTION_STR(func) #func
#define NATIVE_FUNCTION_TYPED_STR(func, type) #func "<" #type ">"
#define NATIVE_FUNCTION_TYPED_2_STR(func, type, type2) \
#func "<" #type ", " #type2 ">"
#define NATIVE_CONSTRUCTOR_STR(func) #func
#define NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func) \
#classname "<" #type ", " #type2 ">::" #func

static llvm::DenseMap<void *, const char *> funcNames() {
  constexpr int numNames = numFuncNames();

  // Can we store all the lengths in uint8_t safely?
#define CHECK_OVERFLOW(len) \
  static_assert(            \
      (len) <= std::numeric_limits<uint8_t>::max(), "overflowed uint8_t");
#define NATIVE_FUNCTION(func) CHECK_OVERFLOW(sizeof(NATIVE_FUNCTION_STR(func)))
#define NATIVE_FUNCTION_TYPED(func, type) \
  CHECK_OVERFLOW(sizeof(NATIVE_FUNCTION_TYPED_STR(func, type)))
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  CHECK_OVERFLOW(sizeof(NATIVE_FUNCTION_TYPED_2_STR(func, type, type2)))
#define NATIVE_CONSTRUCTOR(func) \
  CHECK_OVERFLOW(sizeof(NATIVE_CONSTRUCTOR_STR(func)))
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  CHECK_OVERFLOW(                                              \
      sizeof(NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func)))
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_FUNCTION_TYPED
#undef NATIVE_FUNCTION_TYPED_2
#undef NATIVE_CONSTRUCTOR
#undef NATIVE_CONSTRUCTOR_TYPED

  // Great, store them in uint8_t.
  static constexpr uint8_t nameLengths[numNames] = {
#define NATIVE_FUNCTION(func) sizeof(NATIVE_FUNCTION_STR(func)),
#define NATIVE_FUNCTION_TYPED(func, type) \
  sizeof(NATIVE_FUNCTION_TYPED_STR(func, type)),
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  sizeof(NATIVE_FUNCTION_TYPED_2_STR(func, type, type2)),
#define NATIVE_CONSTRUCTOR(func) sizeof(NATIVE_CONSTRUCTOR_STR(func)),
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  sizeof(NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func)),
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_FUNCTION_TYPED
#undef NATIVE_FUNCTION_TYPED_2
#undef NATIVE_CONSTRUCTOR
#undef NATIVE_CONSTRUCTOR_TYPED
  };

  static constexpr char names[] = {
#define NATIVE_FUNCTION(func) NATIVE_FUNCTION_STR(func) "\0"
#define NATIVE_FUNCTION_TYPED(func, type) \
  NATIVE_FUNCTION_TYPED_STR(func, type) "\0"
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  NATIVE_FUNCTION_TYPED_2_STR(func, type, type2) "\0"
#define NATIVE_CONSTRUCTOR(func) NATIVE_CONSTRUCTOR_STR(func) "\0"
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func) "\0"
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_FUNCTION_TYPED
#undef NATIVE_FUNCTION_TYPED_2
#undef NATIVE_CONSTRUCTOR
#undef NATIVE_CONSTRUCTOR_TYPED
  };

  static constexpr void *functionPointers[numNames] = {
#define NATIVE_FUNCTION(func) (void *)func,
#define NATIVE_FUNCTION_TYPED(func, type) (void *)func<type>,
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) (void *)func<type, type2>,

  // Creator functions are overloaded, we have to cast them to CreatorFunciton *
  // first.
#define NATIVE_CONSTRUCTOR(func) (void *)(CreatorFunction *) func,
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  (void *)(CreatorFunction *) classname<type, type2>::func,
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_FUNCTION_TYPED
#undef NATIVE_FUNCTION_TYPED_2
#undef NATIVE_CONSTRUCTOR
#undef NATIVE_CONSTRUCTOR_TYPED
  };
  llvm::DenseMap<void *, const char *> map(numNames);
  const char *curStr = names;
  for (int i = 0; i < numNames; ++i) {
    map[functionPointers[i]] = curStr;
    curStr += nameLengths[i];
  }
  assert(map.size() == numNames && "A function should only be mapped once");
  return map;
}

static const char *getFunctionNameImpl(void *func) {
  static auto map = funcNames();
  auto it = map.find(func);
  if (it == map.end()) {
    // This function's name isn't in the map, which is possible for some
    // functions defined only in ConsoleHost.
    return "";
  }
  return it->second;
}

const char *getFunctionName(NativeFunctionPtr func) {
  return getFunctionNameImpl((void *)func);
}

const char *getFunctionName(CreatorFunction *func) {
  return getFunctionNameImpl((void *)func);
}

} // namespace vm
} // namespace hermes
