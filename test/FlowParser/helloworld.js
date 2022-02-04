/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xflow-parser %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function hello(thing: string): string {
  return "hello " + thing;
}

// CHECK-LABEL: hello world
print(hello("world"));
