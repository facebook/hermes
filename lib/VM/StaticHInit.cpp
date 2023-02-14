/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/RuntimeFlags.h"
#include "hermes/VM/StaticHUtils.h"

#include "llvh/ADT/ScopeExit.h"

#include <mutex>

using namespace hermes;
using namespace hermes::vm;

namespace {
struct GlobalState {
  /// Mutex protecting the global state.
  std::mutex mutex{};

  /// The lifetime of a Runtime is managed by a smart pointer, but the C API
  /// wants to deal with a regular pointer. Keep all created runtimes here, so
  /// they can be destroyed from a pointer.
  llvh::DenseMap<Runtime *, std::shared_ptr<Runtime>> runtimes{};
};

GlobalState &getGlobalState() {
  static GlobalState state{};
  return state;
}
} // namespace

SHRuntime *_sh_init(const RuntimeConfig &config) {
  std::shared_ptr<Runtime> runtimePtr = Runtime::create(config);
  Runtime *pRuntime = runtimePtr.get();
  {
    auto &gs = getGlobalState();
    std::lock_guard<std::mutex> lock(gs.mutex);
    gs.runtimes.try_emplace(pRuntime, std::move(runtimePtr));
  }
  return getSHRuntime(*pRuntime);
}

extern "C" void _sh_done(SHRuntime *shr) {
  std::shared_ptr<Runtime> runtimePtr{};
  // Keep the scope of the lock only around finding the runtime and removing
  // it from the map.
  {
    auto &gs = getGlobalState();
    std::lock_guard<std::mutex> lock(gs.mutex);
    // Find the runtime.
    auto it = gs.runtimes.find(&getRuntime(shr));
    if (it == gs.runtimes.end()) {
      llvh::errs() << "SHRuntime not found\n";
      abort();
    }
    // Store the runtime in the shared pointer, so it will be destroyed outside
    // the lock.
    runtimePtr.swap(it->second);
    gs.runtimes.erase(it);
  }
}

#ifndef HERMES_IS_MOBILE_BUILD
static SHRuntime *_sh_init(int argc, char **argv, llvh::raw_ostream &errs) {
  vm::RuntimeConfig runtimeConfig{};

  // Serialize command line parsing, since there is a single global parser.
  {
    std::lock_guard<std::mutex> lock(getGlobalState().mutex);

    // Reset the command line parser, in case it was "dirty", and register all
    // options.
    llvh::cl::ResetCommandLineParser();
    hermes::cli::RuntimeFlags runtimeFlags{};

    // Make sure the parser is reset on exit, so it doesn't keep references to
    // stack locations.
    const auto resetParser =
        llvh::make_scope_exit(llvh::cl::ResetCommandLineParser);

    // Parse, if there are any args supplied.
    if (argc && !llvh::cl::ParseCommandLineOptions(argc, argv, "", &errs))
      return nullptr;

    runtimeConfig = cli::buildRuntimeConfig(runtimeFlags);
  }

  return _sh_init(runtimeConfig);
}

extern "C" SHRuntime *_sh_init(int argc, char **argv) {
  if (SHRuntime *shr = _sh_init(argc, argv, llvh::errs()))
    return shr;

  ::exit(1);
}

extern "C" SHRuntime *
_sh_init_with_error(int argc, char **argv, char **errorMessage) {
  std::string err{};
  llvh::raw_string_ostream OS{err};
  if (SHRuntime *shr = _sh_init(argc, argv, OS)) {
    if (errorMessage)
      *errorMessage = nullptr;
    return shr;
  }

  if (errorMessage)
    *errorMessage = ::strdup(OS.str().c_str());
  return nullptr;
}

#endif // HERMES_IS_MOBILE_BUILD
