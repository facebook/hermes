/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

namespace hermes {
namespace vm {

static llvm::DenseMap<void *, std::string> funcNames() {
  llvm::DenseMap<void *, std::string> map;
#define NATIVE_FUNCTION(func)                   \
  assert(                                       \
      map.count((void *)func) == 0 &&           \
      "A function should only be mapped once"); \
  map[(void *)func] = #func;

#define NATIVE_FUNCTION_TYPED(func, type)       \
  assert(                                       \
      map.count((void *)func<type>) == 0 &&     \
      "A function should only be mapped once"); \
  map[(void *)func<type>] = std::string(#func) + "<" + #type + ">";

#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  assert(                                          \
      map.count((void *)func<type, type2>) == 0 && \
      "A function should only be mapped once");    \
  map[(void *)func<type, type2>] =                 \
      std::string(#func) + "<" + #type + ", " + #type2 + ">";

  // Creator functions are overloaded, we have to cast them to CreatorFunciton *
  // first.
  CreatorFunction *funcPtr;
#define NATIVE_CONSTRUCTOR(func)                \
  funcPtr = func;                               \
  assert(                                       \
      map.count((void *)funcPtr) == 0 &&        \
      "A function should only be mapped once"); \
  map[(void *)funcPtr] = #func;

#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func) \
  funcPtr = classname<type, type2>::func;                      \
  assert(                                                      \
      map.count((void *)funcPtr) == 0 &&                       \
      "A function should only be mapped once");                \
  map[(void *)funcPtr] =                                       \
      std::string(#classname) + "<" + #type + ", " + #type2 + ">::" + #func;
#include "hermes/VM/NativeFunctions.def"

  return map;
}

static std::string getFunctionNameImpl(void *func) {
  static auto map = funcNames();
  auto it = map.find(func);
  if (it == map.end()) {
    // This function's name isn't in the map, which is possible for some
    // functions defined only in ConsoleHost.
    return "";
  }
  return it->second;
}

std::string getFunctionName(NativeFunctionPtr func) {
  return getFunctionNameImpl((void *)func);
}

std::string getFunctionName(CreatorFunction *func) {
  return getFunctionNameImpl((void *)func);
}

} // namespace vm
} // namespace hermes
