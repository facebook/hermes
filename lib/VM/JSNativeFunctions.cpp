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
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/PrimitiveBox.h"

#include <limits>
namespace hermes {
namespace vm {

// A helper function to get the length of a string literal while ensuring it
// fits in a uint8.
// \return the length of the string including the terminating nul.
template <size_t N>
constexpr uint8_t u8sizeof(const char (&str)[N]) {
  static_assert(N <= std::numeric_limits<uint8_t>::max(), "overflowed uint8_t");
  return N;
}

#define NATIVE_FUNCTION_STR(func) #func
#define NATIVE_FUNCTION_TYPED_STR(func, type) #func "<" #type ">"
#define NATIVE_FUNCTION_TYPED_2_STR(func, type, type2) \
#func "<" #type ", " #type2 ">"
#define NATIVE_CONSTRUCTOR_STR(func) #func
#define NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func) \
#classname "<" #type ", " #type2 ">::" #func

static llvm::DenseMap<const void *, const char *> funcNames() {
  static constexpr uint8_t nameLengths[] = {
#define NATIVE_FUNCTION(func) u8sizeof(NATIVE_FUNCTION_STR(func)),
#define NATIVE_FUNCTION_TYPED(func, type) \
  u8sizeof(NATIVE_FUNCTION_TYPED_STR(func, type)),
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  u8sizeof(NATIVE_FUNCTION_TYPED_2_STR(func, type, type2)),
#define NATIVE_CONSTRUCTOR(func) u8sizeof(NATIVE_CONSTRUCTOR_STR(func)),
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  u8sizeof(NATIVE_CONSTRUCTOR_TYPED_STR(classname, type, type2, func)),
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

  static const void *const functionPointers[] = {
#define NATIVE_FUNCTION(func) (void *)func,
#define NATIVE_FUNCTION_TYPED(func, type) (void *)func<type>,
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) (void *)func<type, type2>,

  // Creator functions are overloaded, we have to cast them to CreatorFunction *
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

  size_t numFuncs = sizeof functionPointers / sizeof *functionPointers;
  llvm::DenseMap<const void *, const char *> map(numFuncs);
  const char *curStr = names;
  for (size_t i = 0; i < numFuncs; ++i) {
    map[functionPointers[i]] = curStr;
    curStr += nameLengths[i];
  }
  assert(map.size() == numFuncs && "A function should only be mapped once");
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
