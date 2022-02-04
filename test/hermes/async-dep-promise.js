/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines --check-prefix=ON %s
// RUN: (! %hermes -Xes6-promise=0 %s 2>&1) | %FileCheck --match-full-lines --check-prefix=OFF %s
// RUN: %hermesc %s -emit-binary -out %t.hbc && (! %hermes -Xes6-promise=0 %t.hbc 2>&1) | %FileCheck --match-full-lines --check-prefix=OFF %s

// Async function depends on Hermes Promise.

async function empty() {};
print(empty())
// ON: [object Object]
// OFF: Uncaught TypeError: Cannot execute a bytecode having async functions when Promise is disabled.
