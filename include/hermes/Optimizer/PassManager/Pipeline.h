/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_PASSMANAGER_PIPELINE_H
#define HERMES_OPTIMIZER_PASSMANAGER_PIPELINE_H

#include "hermes/IR/IR.h"

#include <string>
#include <vector>

namespace hermes {
class Module;

/// Run a custom set of optimizations.
bool runCustomOptimizationPasses(
    Module &M,
    const std::vector<std::string> &Opts);

/// Run optimization passes corresponding to -O3
void runFullOptimizationPasses(Module &M);

/// Run optimization passes corresponding to -Og
void runDebugOptimizationPasses(Module &M);

/// Run optimization passes corresponding to -O0
void runNoOptimizationPasses(Module &M);
} // namespace hermes

#endif // HERMES_OPTIMIZER_PASSMANAGER_PIPELINE_H
