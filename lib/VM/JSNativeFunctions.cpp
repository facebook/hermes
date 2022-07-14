/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
#include "hermes/VM/JSWeakRef.h"
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

static llvh::DenseMap<const void *, const char *> funcNames() {
  static constexpr uint8_t nameLengths[] = {
#define NATIVE_FUNCTION(func) u8sizeof(#func),
#define NATIVE_CONSTRUCTOR(func) u8sizeof(#func),
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_CONSTRUCTOR
  };

  static constexpr char names[] = {
#define NATIVE_FUNCTION(func) #func "\0"
#define NATIVE_CONSTRUCTOR(func) #func "\0"
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_CONSTRUCTOR
  };

  static const void *const functionPointers[] = {
#define NATIVE_FUNCTION(func) (void *)func,

  // Creator functions are overloaded, we have to cast them to CreatorFunction *
  // first.
#define NATIVE_CONSTRUCTOR(func) \
  (void *)(NativeConstructor::CreatorFunction *) func,
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_FUNCTION
#undef NATIVE_CONSTRUCTOR
  };

  size_t numFuncs = sizeof functionPointers / sizeof *functionPointers;
  llvh::DenseMap<const void *, const char *> map(numFuncs);
  const char *curStr = names;
  for (size_t i = 0; i < numFuncs; ++i) {
    map[functionPointers[i]] = curStr;
    curStr += nameLengths[i];
  }

#ifndef _MSC_VER
  // TODO(T57439543): Currently, under Windows, this will cause a crash due to
  // how multiple functions will be collapsed into this one.
  assert(map.size() == numFuncs && "A function should only be mapped once");
#endif

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

const char *getFunctionName(NativeConstructor::CreatorFunction *func) {
  return getFunctionNameImpl((void *)func);
}

} // namespace vm
} // namespace hermes
