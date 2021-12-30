/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ITERATIONKIND_H
#define HERMES_VM_ITERATIONKIND_H

namespace hermes {
namespace vm {

/// Different Iteration Kinds. Entry means Key + Value.
enum class IterationKind {
  Key,
  Value,
  Entry,
  NumKinds,
};

} // namespace vm
} // namespace hermes

#endif
