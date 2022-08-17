/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_PERFSECTION_H
#define HERMES_SUPPORT_PERFSECTION_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/StringRef.h"

#include <string>

#if defined(HERMES_FACEBOOK_BUILD) && !defined(_WINDOWS) &&      \
    !defined(__EMSCRIPTEN__) && !defined(HERMES_FBCODE_BUILD) && \
    !defined(HERMES_HOST_BUILD)
#define HERMES_USE_FBSYSTRACE
#endif

namespace hermes {

/// A class whose constructor/destructor delimit a region of code
/// execution that should be timed, and may have further attributes
/// associated with it.  This may be used to produce systrace output,
/// platform-specific logging such as Android logcat, or simple
/// print statement to standard output.
class PerfSection {
 public:
#if defined(HERMES_USE_FBSYSTRACE) || defined(HERMESVM_PLATFORM_LOGGING)
  /// Start a section with the given name, and optional category, used in logs.
  PerfSection(const char *name, const char *category = nullptr);

  /// Terminate a section.  This may send telemetry, or write log
  /// messages.
  ~PerfSection();

  /// Add the argument with the given \p argName, with \p value as the
  /// value.  If there are multiples call for the same argName, the
  /// last value is recorded.
  void addArg(const char *argName, size_t value);
  /// Like the above, but for a double value.  We change the name, rather than
  /// use overloading, because multiple integer types convert to the size_t add
  /// arg above, and we don't want the overloading to be ambiguous.
  void addArgD(const char *argName, double d);
  void
  addArg(const char *argName, const llvh::StringRef value, bool copy = true);

#ifdef HERMESVM_PLATFORM_LOGGING
 protected:
  const char *name_;
  const char *category_;
#endif
  /// Whether any tracing output is enabled.
  bool enabled_{false};

 private:
  /// We want ArgValue to be able to hold arguments of various types,
  /// in its union.  This enumeration should correspond to those
  /// types.  When we upgrade to a C++ with std::variant, we should
  /// use that.
  enum class ArgType {
    SIZE_T,
    DOUBLE,
    STRINGREF,
  };

  struct ArgValue {
    ArgType type;
    union {
      size_t sz_t;
      double d;
      struct {
        const char *data;
        size_t size;
        bool owned;
      } stringref;
    } value;

    void freeDependencies() {
      switch (type) {
        case ArgType::STRINGREF:
          if (value.stringref.owned) {
            delete[] value.stringref.data;
          }
          break;
        case ArgType::SIZE_T:
        case ArgType::DOUBLE:
          break;
      }
    }
  };

  llvh::SmallDenseMap<llvh::StringRef, ArgValue> argValues_;

  void freeDataIfExists(const char *argName) {
    auto it = argValues_.find(argName);
    if (it != argValues_.end()) {
      it->second.freeDependencies();
    }
  }

#else
  // Dummy no-overhead implementations:
  PerfSection(const char *name, const char *category = nullptr) {}
  void addArg(const char *argName, size_t value) {}
  void addArgD(const char *argName, double d) {}
  void
  addArg(const char *argName, const llvh::StringRef value, bool copy = true) {}
#endif
};

} // namespace hermes

#endif // HERMES_SUPPORT_PERFSECTION_H
