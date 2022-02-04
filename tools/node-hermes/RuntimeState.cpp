/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RuntimeState.h"

namespace facebook {

llvh::SmallString<32> RuntimeState::resolvePath(
    llvh::StringRef target,
    llvh::StringRef rootDir) {
  llvh::SmallString<32> result;
  if (target.startswith("/")) {
    // If the target is absolute (starts with a '/'), resolve from the root.
    result = rootDir;
    target = target.drop_front(1);
  } else {
    result = getDirname();
  }

  llvh::sys::path::append(result, llvh::sys::path::Style::posix, target);
  // Remove all dots. This is done to get rid of ../ or anything like ././.
  llvh::sys::path::remove_dots(result, true, llvh::sys::path::Style::posix);
  return result;
}

void RuntimeState::defineJSFunction(
    InternalFunction *functionPtr,
    llvh::StringRef functionName,
    size_t numArgs,
    jsi::Object &bindingProp) {
  jsi::Runtime &rt = getRuntime();
  jsi::String jsiFunctionName = jsi::String::createFromAscii(rt, functionName);
  jsi::Object jsFunction = jsi::Function::createFromHostFunction(
      rt,
      jsi::PropNameID::forString(rt, jsiFunctionName),
      numArgs,
      [this, functionPtr](
          jsi::Runtime &rt,
          const jsi::Value &thisValue,
          const jsi::Value *args,
          size_t count) -> jsi::Value {
        return functionPtr(*this, thisValue, args, count);
      });
  bindingProp.setProperty(rt, jsiFunctionName, jsFunction);
}

} // namespace facebook
