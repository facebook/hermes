/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef JSI_UNSTABLE
#include "Worker.h"
#include "Intrinsics.h"
#include "llvh/Support/Compiler.h"

namespace facebook {
namespace hermes {
namespace {

/// Worker-specific Native state, used to mark an Object as a Worker instance.
/// These Worker instances are modeled after HTML WebWorkers.
/// Workers will create a new Hermes Runtime and a new thread upon creation to
/// execute the user-provided Javascript code. It also communicates with the
/// parent runtime through message queues, which are guarded by a mutex. Upon
/// Worker termination (via GC or terminate/close calls), the Worker will quit
/// its execution and the thread will be joined.
class WorkerNativeState : public jsi::NativeState {
 public:
  WorkerNativeState() = default;
};

/// Called by the JS constructor in `11-Worker.js` to mark the first argument.
/// The arguments in \p args must be provided in the following order:
/// 1. the object to be marked as Worker
/// 2. the script to be executed by the Worker
jsi::Value initializeWorker(
    jsi::Runtime &rt,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  // This is only called by the Worker extension script in `11-Worker.js`, so we
  // can guarantee that the argument count is 2.
  assert(count == 2);

  // Verify the user-provided script argument
  if (LLVM_UNLIKELY(!args[1].isString())) {
    throwTypeError(
        rt, "Must provide the source code as a String for the Worker to run");
  }

  // Mark the self object as a Worker
  // This is provided by the `11-Worker.js` script, so it should always be an
  // Object.
  auto self = args[0].asObject(rt);
  self.setNativeState(rt, std::make_shared<WorkerNativeState>());

  return jsi::Value::undefined();
}

} // namespace

void installWorker(jsi::Runtime &rt, jsi::Object &extensions) {
  // Set up function specified in `11-Worker.js`.
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "Worker");

  jsi::Function initWorker = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "initWorker"), 2, initializeWorker);

  setup.call(rt, initWorker);
}

} // namespace hermes
} // namespace facebook
#endif
