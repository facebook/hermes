/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Logging.h"

#ifdef __ANDROID__
#include <android/log.h>
#elif defined(__APPLE__)
#include <os/log.h>
#else
#include <cstdio>
#endif

#include <cstdarg>
#include <memory>

namespace hermes {

void hermesLog(const char *componentName, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
#ifdef __ANDROID__
  __android_log_vprint(ANDROID_LOG_INFO, componentName, fmt, args);
#elif defined(__APPLE__)
  static os_log_t hermesLogger = os_log_create("dev.hermesengine", "Default");
  // Need to make a copy in order to do the vsprintf trick.
  va_list argsCopy;
  va_copy(argsCopy, args);
  int numChars = vsnprintf(nullptr, 0, fmt, argsCopy);
  va_end(argsCopy);
  std::unique_ptr<char[]> buffer{new char[numChars + 1]};
  int numCharsWritten = vsnprintf(buffer.get(), numChars + 1, fmt, args);
  (void)numCharsWritten;
  // Log all messages as public here. Hermes will never log private information.
  os_log_with_type(
      hermesLogger,
      OS_LOG_TYPE_INFO,
      "%{public}s: %{public}s",
      componentName,
      buffer.get());
#else
  fprintf(stderr, "%s: ", componentName);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
#endif
  va_end(args);
}

} // namespace hermes
