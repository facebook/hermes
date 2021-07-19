/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

} // namespace facebook
