/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

({
   foo() :<T = { () }
})

// CHECK: {{.*}}:11:21: error: ':' expected in function type annotation
// CHECK-NEXT:    foo() :<T = { () }
// CHECK-NEXT:                  ~~~^
