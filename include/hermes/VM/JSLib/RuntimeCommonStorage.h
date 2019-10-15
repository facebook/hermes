/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_RUNTIMECOMMONSTORAGE_H
#define HERMES_VM_JSLIB_RUNTIMECOMMONSTORAGE_H

#include <random>
#if __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#include "hermes/VM/MockedEnvironment.h"

#include "llvm/ADT/Optional.h"

namespace hermes {
namespace vm {

/// This struct provides a shared location for per-Runtime storage needs of
/// JSLib. Runtime owns and provides access to an instance of this class.
struct RuntimeCommonStorage {
  RuntimeCommonStorage(bool shouldTrace);
  ~RuntimeCommonStorage();

  /// RuntimeCommonStorage is tied to a single Runtime, and should not be copied
  RuntimeCommonStorage(const RuntimeCommonStorage &) = delete;
  void operator=(const RuntimeCommonStorage &) = delete;

  bool shouldTrace = false;

  /// An environment to replay instead of executing environment-dependent
  /// behavior. This should be used for any circumstance where a result can
  /// change from one run of JS to another.
  llvm::Optional<MockedEnvironment> env;

  /// An environment to record environment-dependent behavior (as a sequence of
  /// results of calls to functions).
  MockedEnvironment tracedEnv;

  /// PRNG used by Math.random()
  std::minstd_rand randomEngine_;
  bool randomEngineSeeded_ = false;

#if __APPLE__
  /// \return a reference to the locale to use for collation, date formatting,
  /// etc. The caller must CFRelease this.
  static CFLocaleRef copyLocale();
#endif
};

} // namespace vm
} // namespace hermes

#endif
