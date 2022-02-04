/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_RUNTIMECOMMONSTORAGE_H
#define HERMES_VM_JSLIB_RUNTIMECOMMONSTORAGE_H

#include <random>
#include "hermes/VM/MockedEnvironment.h"

#include "llvh/ADT/Optional.h"

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
  llvh::Optional<MockedEnvironment> env;

  /// An environment to record environment-dependent behavior (as a sequence of
  /// results of calls to functions).
  MockedEnvironment tracedEnv;

  /// PRNG used by Math.random()
  std::minstd_rand randomEngine_;
  bool randomEngineSeeded_ = false;
};

} // namespace vm
} // namespace hermes

#endif
