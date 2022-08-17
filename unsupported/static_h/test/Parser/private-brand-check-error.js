/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

class C {
  foo() {
    #foo + 1;
    #bar;
  }
}

// CHECK-LABEL: {{.*}}:12:5: error: Private name can only be used as left-hand side of `in` expression
// CHECK-NEXT:     #foo + 1;
// CHECK-NEXT:     ^~~~

// CHECK-LABEL: {{.*}}:13:5: error: Private name can only be used as left-hand side of `in` expression
// CHECK-NEXT:     #bar;
// CHECK-NEXT:     ^~~~
