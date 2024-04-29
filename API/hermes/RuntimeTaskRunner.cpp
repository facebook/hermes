/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RuntimeTaskRunner.h"

namespace facebook {
namespace hermes {
namespace debugger {

RuntimeTaskRunner::RuntimeTaskRunner(
    debugger::AsyncDebuggerAPI &debugger,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskFunc)
    : debugger_(debugger), enqueueRuntimeTask_(enqueueRuntimeTaskFunc) {}

RuntimeTaskRunner::~RuntimeTaskRunner() {}

void RuntimeTaskRunner::enqueueTask(RuntimeTask task, TaskQueues queues) {
  switch (queues) {
    case TaskQueues::All: {
      // Flag indicating whether the task has already been completed. No thread
      // synchronization is required, as tasks are only executed with exclusive
      // access to the runtime, implying they are never concurrently executed.
      std::shared_ptr<bool> alreadyRan = std::make_shared<bool>(false);

      // Ask the integrator to run the task whenever JavaScript is not running.
      enqueueRuntimeTask_([alreadyRan, task](HermesRuntime &runtime) {
        if (!*alreadyRan) {
          // Make sure to set alreadyRan first before executing the RuntimeTask.
          // The task could potentially trigger interrupts by calling into
          // HermesRuntime, which might cause the callback queued with
          // `triggerInterrupt_TS()` to run.
          *alreadyRan = true;
          task(runtime);
        }
      });

      // Ask the AsyncDebuggerAPI to run the task whenever running JavaScript
      // can be interrupted.
      debugger_.triggerInterrupt_TS([alreadyRan, task](HermesRuntime &runtime) {
        if (!*alreadyRan) {
          *alreadyRan = true;
          task(runtime);
        }
      });

    } break;
    case TaskQueues::Integrator:
      // Ask the integrator to run the task whenever JavaScript is not running.
      enqueueRuntimeTask_([task](HermesRuntime &runtime) { task(runtime); });
      break;
    default:
      throw std::invalid_argument("Invalid TaskQueues value");
  }
}

} // namespace debugger
} // namespace hermes
} // namespace facebook
