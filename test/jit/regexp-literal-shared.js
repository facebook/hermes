/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %S/../hermes/regexp-literal-shared.js | %FileCheck --match-full-lines %S/../hermes/regexp-literal-shared.js
// REQUIRES: jit

// The JIT reaches the same Interpreter::createRegExp; run the shared
// literal test through it.
