/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StaticH-internal.h"

using namespace hermes;
using namespace hermes::vm;

/// The lifetime of a Runtime is managed by a smart pointer, but the C API wants
/// to deal with a regular pointer. Keep all created runtimes here, so they can
/// be destroyed from a pointer.
static llvh::DenseMap<Runtime *, std::shared_ptr<Runtime>> s_runtimes{};

extern "C" SHRuntime *_sh_init(void) {
  auto config = RuntimeConfig::Builder().build();
  std::shared_ptr<Runtime> runtimePtr = Runtime::create(config);
  // Get the pointer first, since order of argument evaluation is not defined.
  Runtime *pRuntime = runtimePtr.get();
  s_runtimes.try_emplace(pRuntime, std::move(runtimePtr));
  return getSHRuntime(*pRuntime);
}

extern "C" void _sh_done(SHRuntime *shr) {
  auto it = s_runtimes.find(&getRuntime(shr));
  if (it == s_runtimes.end()) {
    llvh::errs() << "SHRuntime not found\n";
    abort();
  }
  s_runtimes.erase(it);
}
