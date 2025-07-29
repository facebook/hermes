/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/CommandLine.h"

namespace hermes {

/// Command line options used by the Runtime but also shared with the compiler.
/// This struct can be used as a header-only dependency and instantiated by
/// any client needing to parse these command line options.
struct CompilerRuntimeFlags {
  llvh::cl::opt<bool> EnableEval{
      "enable-eval",
      llvh::cl::init(true),
      llvh::cl::desc("Enable support for eval()")};

  // This is normally a compiler option, but it also applies to strings given
  // to eval or the Function constructor.
  llvh::cl::opt<bool> VerifyIR{
      "verify-ir",
#ifdef HERMES_SLOW_DEBUG
      llvh::cl::init(true),
#else
      llvh::cl::init(false),
      llvh::cl::Hidden,
#endif
      llvh::cl::desc("Verify the IR after creating it")};

  llvh::cl::opt<bool> EmitAsyncBreakCheck{
      "emit-async-break-check",
      llvh::cl::desc("Emit instruction to check async break request"),
      llvh::cl::init(false)};

  llvh::cl::opt<bool> OptimizedEval{
      "optimized-eval",
      llvh::cl::desc("Turn on compiler optimizations in eval."),
      llvh::cl::init(false)};

  llvh::cl::opt<bool> ES6BlockScoping{
      "Xes6-block-scoping",
      llvh::cl::init(false),
      llvh::cl::desc("Enable support for ES6 block scoping"),
      llvh::cl::Hidden};

  llvh::cl::opt<bool> EnableAsyncGenerators{
      "Xasync-generators",
      llvh::cl::init(false),
      llvh::cl::desc("Enable support for async generators"),
      llvh::cl::Hidden};
};

} // namespace hermes
