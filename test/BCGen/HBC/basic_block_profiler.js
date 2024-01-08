/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode --basic-block-profiling -O %s | %FileCheckOrRegen --match-full-lines %s

var condition = false;
try {
  try {
    print(condition? "yes": "no");
  } finally {
    print("rethrowing");
  }
} catch (e) {
  print(e.stack);
}
