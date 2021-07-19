/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Logging.h"

#ifdef __ANDROID__
#include <android/log.h>
#elif defined(__APPLE__)
#include <os/log.h>
#elif defined(_MSC_VER)
#include <windows.h>
#include <TraceLoggingProvider.h>
#include <mutex>
#else
#include <cstdio>
#endif

#include <cstdarg>
#include <memory>

namespace hermes {

#if defined(_MSC_VER)
// Define the GUID to use in TraceLoggingProviderRegister
// {0x5bf36bca, 0x6f36, 0x44a7, {0x83, 0x6f, 0x4f, 0x41, 0xad, 0x75, 0x86,
// 0x32}}
TRACELOGGING_DEFINE_PROVIDER(
    g_hTraceLoggingProvider,
    "Facebook.hermes",
    (0x5bf36bca,
     0x6f36,
     0x44a7,
     0x83,
     0x6f,
     0x4f,
     0x41,
     0xad,
     0x75,
     0x86,
     0x32));
#endif

void hermesLog(const char *componentName, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
#ifdef __ANDROID__
  __android_log_vprint(ANDROID_LOG_INFO, componentName, fmt, args);
#elif defined(__APPLE__) || defined(_MSC_VER)
  // Need to make a copy in order to do the vsprintf trick.
  va_list argsCopy;
  va_copy(argsCopy, args);
  int numChars = vsnprintf(nullptr, 0, fmt, argsCopy);
  va_end(argsCopy);
  std::unique_ptr<char[]> buffer{new char[numChars + 1]};
  int numCharsWritten = vsnprintf(buffer.get(), numChars + 1, fmt, args);
  (void)numCharsWritten;
  // Log all messages as public here. Hermes will never log private information.
#if defined(__APPLE__)
  static os_log_t hermesLogger = os_log_create("dev.hermesengine", "Default");
  os_log_with_type(
      hermesLogger,
      OS_LOG_TYPE_INFO,
      "%{public}s: %{public}s",
      componentName,
      buffer.get());
#elif defined(_MSC_VER)
  static std::once_flag g_initEtw_once;
  std::call_once(
      g_initEtw_once, []() { TraceLoggingRegister(g_hTraceLoggingProvider); });

  TraceLoggingWrite(
      g_hTraceLoggingProvider,
      "hermesLog",
      TraceLoggingString(componentName, "componentName"),
      TraceLoggingString(buffer.get(), "args"));
#endif
#else
  fprintf(stderr, "%s: ", componentName);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
#endif
  va_end(args);
}

} // namespace hermes
