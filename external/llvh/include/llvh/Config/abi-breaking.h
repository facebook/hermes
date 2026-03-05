/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef LLVH_CONFIG_ABI_BREAKING_H
#define LLVH_CONFIG_ABI_BREAKING_H

// Hermes does not use LLVM's ABI-breaking checks.
#define LLVM_ENABLE_ABI_BREAKING_CHECKS 0

// Hermes does not use LLVM's reverse iteration.
#define LLVM_ENABLE_REVERSE_ITERATION 0

#endif // LLVH_CONFIG_ABI_BREAKING_H
