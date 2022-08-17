/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type X = {
  get x(number): number;
  set x(): number;
}

// CHECK-LABEL: {{.*}}:11:3: error: Getter must have 0 parameters
// CHECK-NEXT:   get x(number): number;
// CHECK-NEXT:   ^~~~~~~~~~~~~~~~~~~~~

// CHECK-LABEL: {{.*}}:12:3: error: Setter must have 1 parameter
// CHECK-NEXT:   set x(): number;
// CHECK-NEXT:   ^~~~~~~~~~~~~~~
